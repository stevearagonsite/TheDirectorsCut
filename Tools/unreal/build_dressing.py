"""
Ambientación de cine negro sobre L_Slice (T-0012). Correr DESPUÉS de build_slice.py, en el editor:
  .\\Tools\\scripts\\tdc.ps1 python -Script Tools\\unreal\\build_dressing.py
Antes: git lfs lock Game/Content/Maps/L_Slice.umap

Qué hace (idempotente: borra y rehace todo lo que tenga label "DR_"):
- Importa Art/Export/Dressing/T_Backdrop_Noir.png y crea M_Backdrop (unlit, la textura va a Emissive).
- Pone el telón pintado (skyline noir) detrás del nivel, del lado -Y. La cámara está en +Y mirando a -Y.
- Importa los props Art/Export/Dressing/SM_*.fbx que existan y los ubica en primer plano (entre la cámara y el
  plano de juego), sobre una franja de escenario oscura: son siluetas de utilería (D-012), no tocan el gameplay.
- Pasa la luz a noche: sol bajo y azulado, SkyLight tenue, niebla más densa y azul.

Plano de juego: X = horizontal, Z = vertical, Y = profundidad (el personaje vive en Y=0).
"""

import json
import os

import unreal

MAP_PATH = "/Game/Maps/L_Slice"
DEST = "/Game/Dressing"
REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
SRC = os.path.join(REPO, "Art", "Export", "Dressing")

eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
eal = unreal.EditorAssetLibrary
mel = unreal.MaterialEditingLibrary

SET_PIECE = unreal.load_class(None, "/Script/TheDirectorsCut.TDCSetPiece")
CUBE = "/Engine/BasicShapes/Cube.Cube"

# Telón: cubre el nivel (X -1500..9000) con la proporción de la imagen (3168x1344 = 2.357).
# La cámara (a ~4000 cm del telón, FOV 60) ve una franja de ~2600 cm de alto: el telón tiene que caber en esa franja
# para que se vea el skyline y no solo la bruma de abajo. Para cubrir el ancho del nivel, la imagen se repite 2 veces.
BACKDROP_TILES_U = 2.0
BACKDROP_HEIGHT = 3400.0
BACKDROP_WIDTH = BACKDROP_HEIGHT * 2.357 * BACKDROP_TILES_U
BACKDROP_CENTER = (3750.0, -1800.0, -700.0 + BACKDROP_HEIGHT / 2.0)

# Props en primer plano: (archivo sin extensión, X, Y, yaw, alto deseado en cm).
# Repartidos para que siempre haya uno o dos en cuadro (la cámara mira ~2500 cm centrados 700 por delante del director).
PROPS = [
    ("SM_Spotlight", 700.0, 650.0, 30.0, 220.0),
    ("SM_Crates", 1900.0, 700.0, 10.0, 150.0),
    ("SM_FilmCamera", 3300.0, 700.0, -25.0, 190.0),
    ("SM_Spotlight", 4600.0, 650.0, -30.0, 220.0),
    ("SM_Crates", 5700.0, 700.0, -5.0, 150.0),
    ("SM_FilmCamera", 7000.0, 700.0, 20.0, 190.0),
]
FOREGROUND_Y = (450.0, 950.0)   # franja de escenario en primer plano
FOREGROUND_TOP_Z = -40.0


def log(msg):
    unreal.log("[TDC] " + msg)


def fail(message):
    unreal.log_error(f"[TDC] ERROR: {message}")
    raise RuntimeError(message)


def require_writable(asset_path, ext):
    rel = asset_path.replace("/Game/", "", 1) + ext
    disk = os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_content_dir()), rel)
    if os.path.exists(disk) and not os.access(disk, os.W_OK):
        fail(f"Game/Content/{rel} es read-only (lockable de LFS sin lock). Antes: git lfs lock Game/Content/{rel}")


def vec(x, y, z):
    return unreal.Vector(x, y, z)


def rot(pitch=0.0, yaw=0.0, roll=0.0):
    r = unreal.Rotator()
    r.pitch, r.yaw, r.roll = pitch, yaw, roll
    return r


def import_file(path, name, options=None, folder=None):
    task = unreal.AssetImportTask()
    task.filename = path
    task.destination_path = folder or DEST
    task.destination_name = name
    task.replace_existing = True
    task.automated = True
    task.save = True
    if options:
        task.options = options
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    target = f"{folder or DEST}/{name}"
    return eal.load_asset(target) if eal.does_asset_exist(target) else None


def backdrop_material():
    png = os.path.join(SRC, "T_Backdrop_Noir.png")
    if not os.path.isfile(png):
        log("sin T_Backdrop_Noir.png: no hay telón")
        return None
    tex = import_file(png, "T_Backdrop_Noir")
    if not tex:
        fail("no se importó T_Backdrop_Noir")
    path = f"{DEST}/M_Backdrop"
    # does_asset_exist antes de load_asset: si no, el primer run loguea "LoadAsset failed" y el commandlet sale con 1.
    mat = eal.load_asset(path) if eal.does_asset_exist(path) else None
    if not mat:
        mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset("M_Backdrop", DEST, unreal.Material, unreal.MaterialFactoryNew())
    mel.delete_all_material_expressions(mat)
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    mat.set_editor_property("two_sided", True)
    sample = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -400, 0)
    sample.set_editor_property("texture", tex)
    tiled = mel.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -600, 0)
    tiled.set_editor_property("u_tiling", BACKDROP_TILES_U)
    mel.connect_material_expressions(tiled, "", sample, "UVs")
    mul = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -150, 0)
    mul.set_editor_property("const_b", 0.85)  # un poco más oscuro que la imagen para que el nivel destaque
    mel.connect_material_expressions(sample, "RGB", mul, "A")
    # Gradiente vertical: el tercio inferior (bruma clara) se oscurece hacia abajo para que no haga una línea celeste
    # a la altura del piso. V=0 arriba, V=1 abajo: factor = 1 - 0.8 * V^3.
    uv = mel.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -700, 250)
    mask = mel.create_material_expression(mat, unreal.MaterialExpressionComponentMask, -550, 250)
    mask.set_editor_property("r", False)
    mask.set_editor_property("g", True)
    mel.connect_material_expressions(uv, "", mask, "")
    pw = mel.create_material_expression(mat, unreal.MaterialExpressionPower, -400, 250)
    pw.set_editor_property("const_exponent", 3.0)
    mel.connect_material_expressions(mask, "", pw, "Base")
    scl = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -250, 250)
    scl.set_editor_property("const_b", 0.8)
    mel.connect_material_expressions(pw, "", scl, "A")
    inv = mel.create_material_expression(mat, unreal.MaterialExpressionOneMinus, -100, 250)
    mel.connect_material_expressions(scl, "", inv, "")
    dark = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, 50, 0)
    mel.connect_material_expressions(mul, "", dark, "A")
    mel.connect_material_expressions(inv, "", dark, "B")
    mel.connect_material_property(dark, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.recompile_material(mat)
    eal.save_asset(path)
    return mat


# Parallax (3 capas a distinta profundidad; la cámara lateral en 3D da el parallax sola).
# (archivo, nombre, Y, z_inferior, alto, ancho, masked, oscurecer). Cámara en Y≈+2200 mirando a -Y.
PARALLAX = [
    ("L0_cielo.png", "Parallax_L0", -4500.0, -2000.0, 5000.0, 17000.0, False, 0.8),
    ("L1_skyline_lejano.png", "Parallax_L1", -3000.0, -1500.0, 3600.0, 15000.0, True, 0.75),
    ("L2_edificios_cercanos.png", "Parallax_L2", -1600.0, -1100.0, 2600.0, 13000.0, True, 0.9),
]
PARALLAX_DIR = os.path.join(SRC, "Parallax")


def parallax_material(png, name, masked, darken, tiles_u):
    tex = import_file(png, f"T_{name}", folder=f"{DEST}/Parallax")
    if not tex:
        fail(f"no se importó T_{name}")
    path = f"{DEST}/Parallax/M_{name}"
    mat = eal.load_asset(path) if eal.does_asset_exist(path) else None
    if not mat:
        mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(f"M_{name}", f"{DEST}/Parallax", unreal.Material, unreal.MaterialFactoryNew())
    mel.delete_all_material_expressions(mat)
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_MASKED if masked else unreal.BlendMode.BLEND_OPAQUE)
    mat.set_editor_property("two_sided", True)
    uv = mel.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -700, 0)
    uv.set_editor_property("u_tiling", tiles_u)
    sample = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -450, 0)
    sample.set_editor_property("texture", tex)
    mel.connect_material_expressions(uv, "", sample, "UVs")
    mul = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -200, 0)
    mul.set_editor_property("const_b", darken)
    mel.connect_material_expressions(sample, "RGB", mul, "A")
    mel.connect_material_property(mul, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    if masked:
        mel.connect_material_property(sample, "A", unreal.MaterialProperty.MP_OPACITY_MASK)
    mel.recompile_material(mat)
    eal.save_asset(path)
    return mat


def build_parallax(cube):
    """Si están las 3 capas en Art/Export/Dressing/Parallax/, arma el parallax y devuelve True."""
    if not all(os.path.isfile(os.path.join(PARALLAX_DIR, f)) for f, *_ in PARALLAX):
        log("sin capas de parallax completas: telón único")
        return False
    for f, name, y, z0, h, w, masked, darken in PARALLAX:
        png = os.path.join(PARALLAX_DIR, f)
        tiles = max(1.0, w / (h * 2.357))  # mantiene la proporción 21:9 de la imagen
        mat = parallax_material(png, name, masked, darken, tiles)
        actor, comp = spawn_mesh(f"DR_{name}", cube, (4000.0, y, z0 + h / 2.0), rot(), vec(w / 100.0, 0.02, h / 100.0))
        comp.set_material(0, mat)
        comp.set_cast_shadow(False)
    log("parallax de 3 capas armado")
    return True


# Cards 2D (recortes PNG con alpha) a distintas profundidades: Art/Export/Dressing/Cards/cards.json.
CARDS_DIR = os.path.join(SRC, "Cards")


def card_flipbook_material(tex, name, fb, darken):
    """MIC de M_TDCFlipbook (import_vfx.py) con la sheet en loop."""
    parent_path = "/Game/VFX/M_TDCFlipbook"
    if not eal.does_asset_exist(parent_path):
        log(f"sin M_TDCFlipbook (correr import_vfx.py antes): {name} queda estática")
        return None
    path = f"{DEST}/Cards/MI_{name}"
    mic = eal.load_asset(path) if eal.does_asset_exist(path) else None
    if not mic:
        mic = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            f"MI_{name}", f"{DEST}/Cards", unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    mel.set_material_instance_parent(mic, eal.load_asset(parent_path))
    mel.set_material_instance_texture_parameter_value(mic, "Sheet", tex)
    for k, v in (("Cols", fb["cols"]), ("Rows", fb["rows"]), ("Frames", fb["frames"]), ("FPS", fb.get("fps", 30)),
                 ("Loop", 1.0), ("StartTime", 0.0), ("Intensity", 1.5 * darken)):
        mel.set_material_instance_scalar_parameter_value(mic, k, float(v))
    eal.save_asset(path)
    return mic


def build_cards(cube):
    spec = os.path.join(CARDS_DIR, "cards.json")
    if not os.path.isfile(spec):
        log("sin cards.json: no hay cards 2D")
        return
    with open(spec, encoding="utf-8") as f:
        cards = json.load(f)
    if isinstance(cards, dict):
        cards = cards.get("cards", [])
    placed = 0
    mats = {}
    for i, c in enumerate(cards):
        png = os.path.join(CARDS_DIR, c["png"])
        if not os.path.isfile(png):
            log(f"card sin archivo: {c['png']}")
            continue
        base = "Card_" + os.path.splitext(os.path.basename(c["png"]))[0]
        darken = float(c.get("darken", 1.0))
        fb = c.get("flipbook")
        key = (base, darken, bool(fb))
        if key not in mats:
            if fb:
                tex = import_file(png, f"T_{base}", folder=f"{DEST}/Cards")
                if tex:
                    tex.set_editor_property("filter", unreal.TextureFilter.TF_NEAREST)
                    tex.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
                mats[key] = card_flipbook_material(tex, f"{base}_{int(darken*100)}", fb, darken) if tex else None
            else:
                mats[key] = parallax_material(png, f"{base}_{int(darken*100)}", bool(c.get("masked", True)), darken, 1.0)
        mat = mats[key]
        if not mat:
            continue
        # Proporción del frame (sheet / grilla para flipbooks).
        tex_path = f"{DEST}/Cards/T_{base}" if fb else f"{DEST}/Parallax/T_{base}_{int(darken*100)}"
        tex = eal.load_asset(tex_path) if eal.does_asset_exist(tex_path) else None
        aspect = 1.0
        if tex:
            w, h = tex.blueprint_get_size_x(), tex.blueprint_get_size_y()
            if fb:
                w, h = w / fb["cols"], h / fb["rows"]
            aspect = w / max(1.0, h)
        height = float(c.get("height", 300.0))
        width = height * aspect * (-1.0 if c.get("flip") else 1.0)
        z = float(c.get("z", 0.0))
        actor, comp = spawn_mesh(f"DR_Card_{i:02d}_{base}", cube, (float(c["x"]), float(c["y"]), z + height / 2.0), rot(),
                                 vec(width / 100.0, 0.02, height / 100.0))
        comp.set_material(0, mat)
        comp.set_cast_shadow(False)
        placed += 1
    log(f"cards 2D colocadas: {placed}")


def import_prop(name):
    fbx = os.path.join(SRC, name + ".fbx")
    if not os.path.isfile(fbx):
        log(f"sin {name}.fbx: se omite")
        return None
    ui = unreal.FbxImportUI()
    ui.import_mesh = True
    ui.import_as_skeletal = False
    ui.mesh_type_to_import = unreal.FBXImportType.FBXIT_STATIC_MESH
    ui.import_animations = False
    ui.import_materials = True
    ui.import_textures = True
    ui.automated_import_should_detect_type = False
    ui.static_mesh_import_data.combine_meshes = True
    ui.static_mesh_import_data.set_editor_property("auto_generate_collision", False)
    # Una subcarpeta por prop: Rodin nombra texturas y materiales igual en todos (texture_diffuse, model...).
    sm = import_file(fbx, name, ui, folder=f"{DEST}/{name}")
    if not sm:
        fail(f"no se importó {name}")
    return sm


def clear_dressing():
    old = [a for a in eas.get_all_level_actors() if a.get_actor_label().startswith("DR_")]
    if old:
        eas.destroy_actors(old)


def spawn_mesh(label, mesh, loc, rotation, scale):
    actor = eas.spawn_actor_from_class(unreal.StaticMeshActor, vec(*loc), rotation)
    actor.set_actor_label(label)
    actor.set_folder_path("Dressing")
    comp = actor.static_mesh_component
    comp.set_static_mesh(mesh)
    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    actor.set_actor_scale3d(scale)
    return actor, comp


def night_lighting():
    for actor in eas.get_all_level_actors():
        label = actor.get_actor_label()
        if label == "Ambient_Sun":
            actor.set_actor_rotation(rot(pitch=-12.0, yaw=-60.0), False)
            comp = actor.get_component_by_class(unreal.DirectionalLightComponent)
            comp.set_intensity(0.8)
            comp.set_light_color(unreal.LinearColor(0.55, 0.65, 1.0, 1.0))
        elif label == "Sky_Light":
            actor.get_component_by_class(unreal.SkyLightComponent).set_intensity(0.3)
        elif label == "Pit_Floor":
            # El fondo del pozo se veía lila al bajar la cámara: oscuro, como el escenario.
            actor.set_editor_property("color_override", unreal.LinearColor(0.03, 0.03, 0.04, 1.0))
        elif label == "Height_Fog":
            # Poca niebla y solo lejos: el telón ya trae su bruma pintada; la niebla densa lo lavaba.
            fog = actor.get_component_by_class(unreal.ExponentialHeightFogComponent)
            fog.set_editor_property("fog_density", 0.008)
            fog.set_editor_property("start_distance", 4000.0)
            fog.set_editor_property("fog_inscattering_luminance", unreal.LinearColor(0.03, 0.04, 0.08, 1.0))


def dynamic_lighting_only():
    """Sin iluminación precalculada: el build no tiene lightmaps y mostraba "LIGHTING NEEDS TO BE REBUILT" en pantalla.
    Todas las luces pasan a Movable y el mundo fuerza no usar luz precalculada."""
    for actor in eas.get_all_level_actors():
        for comp in actor.get_components_by_class(unreal.LightComponentBase):
            comp.set_mobility(unreal.ComponentMobility.MOVABLE)
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    world.get_world_settings().set_editor_property("force_no_precomputed_lighting", True)
    log("luz 100% dinámica (force_no_precomputed_lighting, luces Movable).")


def post_process():
    """Look de cine negro: exposición más baja (el auto-exposure aclaraba todo), viñeta, grano y menos saturación."""
    ppv = eas.spawn_actor_from_class(unreal.PostProcessVolume, vec(0.0, 0.0, 0.0), rot())
    ppv.set_actor_label("DR_PostProcess")
    ppv.set_folder_path("Dressing")
    ppv.set_editor_property("unbound", True)
    st = ppv.get_editor_property("settings")
    for name, value in (("auto_exposure_bias", -0.3),
                        ("vignette_intensity", 0.45),
                        ("film_grain_intensity", 0.25),
                        ("color_saturation", unreal.Vector4(0.8, 0.8, 0.85, 1.0)),
                        ("color_contrast", unreal.Vector4(1.15, 1.15, 1.15, 1.0)),
                        ("scene_color_tint", unreal.LinearColor(0.85, 0.92, 1.0, 1.0))):
        try:
            st.set_editor_property("override_" + name, True)
            st.set_editor_property(name, value)
        except Exception as e:
            unreal.log_warning(f"[TDC] post-proceso: no se pudo setear {name} ({e})")
    ppv.set_editor_property("settings", st)


def build():
    require_writable(MAP_PATH, ".umap")
    require_writable(f"{DEST}/T_Backdrop_Noir", ".uasset")
    require_writable(f"{DEST}/M_Backdrop", ".uasset")
    les.load_level(MAP_PATH)
    clear_dressing()

    cube = unreal.load_asset(CUBE)
    mat = None if build_parallax(cube) else backdrop_material()
    build_cards(cube)
    if mat:
        actor, comp = spawn_mesh("DR_Backdrop", cube, BACKDROP_CENTER, rot(),
                                 vec(BACKDROP_WIDTH / 100.0, 0.02, BACKDROP_HEIGHT / 100.0))
        comp.set_material(0, mat)
        comp.set_cast_shadow(False)

    # Franja de escenario en primer plano (oscura, sin colisión con el plano de juego: está en Y>0).
    y0, y1 = FOREGROUND_Y
    stage = eas.spawn_actor_from_class(SET_PIECE, vec(3750.0, (y0 + y1) / 2.0, FOREGROUND_TOP_Z - 50.0), rot())
    stage.set_actor_label("DR_ForegroundStage")
    stage.set_folder_path("Dressing")
    stage.set_actor_scale3d(vec(110.0, (y1 - y0) / 100.0, 1.0))
    stage.set_editor_property("color_override", unreal.LinearColor(0.04, 0.04, 0.05, 1.0))

    cache = {}
    for i, (name, x, y, yaw, height) in enumerate(PROPS):
        if name not in cache:
            cache[name] = import_prop(name)
        sm = cache[name]
        if not sm:
            continue
        extent = sm.get_bounds().box_extent
        s = height / max(1.0, extent.z * 2.0)
        spawn_mesh(f"DR_{name}_{i}", sm, (x, y, FOREGROUND_TOP_Z), rot(yaw=yaw), vec(s, s, s))

    night_lighting()
    post_process()
    dynamic_lighting_only()

    if not les.save_current_level():
        fail(f"No se pudo guardar {MAP_PATH} (¿read-only por LFS? git lfs lock Game/Content/Maps/L_Slice.umap)")
    log("ambientación noir aplicada y L_Slice guardado.")


build()
