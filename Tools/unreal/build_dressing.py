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
# Utilería de set (T-0036), en la franja de primer plano, lejos de los beats (L1 1600, L2 3050, puente 4800-6000,
# L3 6750) y de los props de arriba: (carpeta en Art/Export/Dressing/SetProps, archivo, x, y, yaw, alto cm, tinte).
SET_PROPS = [
    ("SM_DirectorChair", "SM_DirectorChair.fbx", 300.0, 650.0, 25.0, 90.0, 0.85),
    ("SM_FilmCanStack", "SM_FilmCanStack.glb", 2350.0, 620.0, -10.0, 45.0, 0.9),
    ("SM_BoomMic", "SM_BoomMic.fbx", 3950.0, 800.0, 160.0, 220.0, 0.45),  # madera clara del trípode: oscurecer
    ("SM_FilmCanStack", "SM_FilmCanStack.glb", 6350.0, 600.0, 15.0, 40.0, 0.9),
    ("SM_DirectorChair", "SM_DirectorChair.fbx", 7650.0, 700.0, -30.0, 90.0, 0.85),
]
HANGING_LIGHT_DIR = os.path.join(SRC, "StageLightHanging")
HANGING_LIGHT_DEST = "/Game/Dressing/Props"
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
    # T-0039 (auditoría visual): el fondo era lo más brillante de la pantalla y el plano de juego quedaba oscuro.
    # Fondo a 0.30–0.45 y teñido violeta apagado (PARALLAX_TINT): el ojo va al plano de juego, como en el key art.
    ("L0_cielo.png", "Parallax_L0", -4500.0, -2000.0, 5000.0, 17000.0, False, 0.38),
    ("L1_skyline_lejano.png", "Parallax_L1", -3000.0, -1500.0, 3600.0, 15000.0, True, 0.32),
    ("L2_edificios_cercanos.png", "Parallax_L2", -1600.0, -1100.0, 2600.0, 13000.0, True, 0.45),
]
PARALLAX_DIR = os.path.join(SRC, "Parallax")
PARALLAX_TINT = (0.82, 0.76, 1.0)  # violeta apagado (no el violeta saturado de la mecánica)


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
    tint = mel.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -350, 150)
    tint.set_editor_property("constant", unreal.LinearColor(PARALLAX_TINT[0], PARALLAX_TINT[1], PARALLAX_TINT[2], 1.0))
    tinted = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -80, 0)
    mel.connect_material_expressions(mul, "", tinted, "A")
    mel.connect_material_expressions(tint, "", tinted, "B")
    mel.connect_material_property(tinted, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
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


CARD_LIGHTS = ("fg_farola.png", "mid_marquesina.png")  # cards que son fuente de luz (auditoría #6: luces prácticas)


def card_practical_lights():
    """Luz ámbar desaturada (~2700 K) en cada farola y en la marquesina: charcos de luz cálida en la calle azul."""
    spec = os.path.join(CARDS_DIR, "cards.json")
    if not os.path.isfile(spec):
        return
    with open(spec, encoding="utf-8") as f:
        cards = json.load(f)
    if isinstance(cards, dict):
        cards = cards.get("cards", [])
    n = 0
    for c in cards:
        if c.get("png") not in CARD_LIGHTS:
            continue
        z = float(c.get("z", 0.0)) + float(c.get("height", 300.0)) * 0.85
        actor = eas.spawn_actor_from_class(unreal.PointLight, vec(float(c["x"]), float(c["y"]) - 80.0, z), rot())
        actor.set_actor_label(f"DR_CardLight_{n}")
        actor.set_folder_path("Dressing/Lights")
        comp = actor.get_component_by_class(unreal.PointLightComponent)
        comp.set_mobility(unreal.ComponentMobility.MOVABLE)
        comp.set_intensity(35.0)
        comp.set_attenuation_radius(900.0)
        comp.set_light_color(unreal.LinearColor(1.0, 0.7, 0.42, 1.0))
        comp.set_cast_shadows(False)
        n += 1
    log(f"luces prácticas en cards: {n}")


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
                    # lluvia y vapor ahora son pintados (T-0048): bilinear, sin escalones al agrandarse
                    tex.set_editor_property("filter", unreal.TextureFilter.TF_BILINEAR)
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


def static_fbx_options():
    ui = unreal.FbxImportUI()
    ui.import_mesh = True
    ui.import_as_skeletal = False
    ui.mesh_type_to_import = unreal.FBXImportType.FBXIT_STATIC_MESH
    ui.import_animations = False
    ui.import_materials = False  # el material lo arma prop_material (tinte mate, sin metallic de Rodin)
    ui.import_textures = False
    ui.automated_import_should_detect_type = False
    ui.static_mesh_import_data.combine_meshes = True
    ui.static_mesh_import_data.set_editor_property("auto_generate_collision", False)
    return ui


def first_static_mesh(folder):
    for path in eal.list_assets(folder, recursive=True, include_folder=False):
        asset = eal.load_asset(path)
        if isinstance(asset, unreal.StaticMesh):
            return asset
    return None


def prop_material(folder, name, png, tint):
    """Material mate con la difusa de Rodin multiplicada por un tinte (oscurece lo que salió claro). None si no hay png."""
    if not os.path.isfile(png):
        return None
    tex = import_file(png, f"T_{name}_D", folder=folder)
    if not tex:
        return None
    path = f"{folder}/M_{name}"
    require_writable(path, ".uasset")
    if eal.does_asset_exist(path):
        mat = eal.load_asset(path)
        mel.delete_all_material_expressions(mat)
    else:
        mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(f"M_{name}", folder, unreal.Material,
                                                                       unreal.MaterialFactoryNew())
    sample = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -500, 0)
    sample.set_editor_property("texture", tex)
    mul = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -250, 0)
    mul.set_editor_property("const_b", tint)
    mel.connect_material_expressions(sample, "RGB", mul, "A")
    mel.connect_material_property(mul, "", unreal.MaterialProperty.MP_BASE_COLOR)
    for prop, value, y in ((unreal.MaterialProperty.MP_ROUGHNESS, 0.8, 200), (unreal.MaterialProperty.MP_METALLIC, 0.0, 300)):
        c = mel.create_material_expression(mat, unreal.MaterialExpressionConstant, -250, y)
        c.set_editor_property("r", value)
        mel.connect_material_property(c, "", prop)
    mel.recompile_material(mat)
    eal.save_asset(path, only_if_is_dirty=False)
    return mat


def assign_material(sm, mat):
    if not sm or not mat:
        return
    for i in range(max(1, len(sm.get_editor_property("static_materials")))):
        sm.set_material(i, mat)
    eal.save_asset(sm.get_path_name().split(".")[0], only_if_is_dirty=False)


def import_set_prop(folder_name, file_name, tint):
    """Importa Art/Export/Dressing/SetProps/<folder>/<file> (FBX de Rodin o glTF) a /Game/Dressing/SetProps/<folder>."""
    src_dir = os.path.join(SRC, "SetProps", folder_name)
    src = os.path.join(src_dir, file_name)
    if not os.path.isfile(src):
        log(f"sin {file_name}: se omite")
        return None
    folder = f"{DEST}/SetProps/{folder_name}"
    is_fbx = file_name.lower().endswith(".fbx")
    sm = import_file(src, folder_name, static_fbx_options() if is_fbx else None, folder=folder)
    if not isinstance(sm, unreal.StaticMesh):
        sm = first_static_mesh(folder)  # glTF (Interchange) nombra la malla por el nodo
    if not sm:
        unreal.log_warning(f"[TDC] {file_name} no produjo StaticMesh")
        return None
    if is_fbx:
        assign_material(sm, prop_material(folder, folder_name, os.path.join(src_dir, "texture_diffuse.png"), tint))
    return sm


def import_hanging_light():
    """Reflector colgante (T-0036) -> /Game/Dressing/Props/SM_StageLight_Hanging. ATDCStageLight lo carga si existe."""
    fbx = os.path.join(HANGING_LIGHT_DIR, "SM_StageLight_Hanging.fbx")
    if not os.path.isfile(fbx):
        log("sin SM_StageLight_Hanging.fbx: los reflectores siguen con la esfera")
        return None
    require_writable(f"{HANGING_LIGHT_DEST}/SM_StageLight_Hanging", ".uasset")
    sm = import_file(fbx, "SM_StageLight_Hanging", static_fbx_options(), folder=HANGING_LIGHT_DEST)
    if not sm:
        unreal.log_warning("[TDC] no se importó SM_StageLight_Hanging")
        return None
    # Salió gris claro: 0.35 lo lleva al metal oscuro de los reflectores de los 40.
    assign_material(sm, prop_material(HANGING_LIGHT_DEST, "StageLight_Hanging",
                                      os.path.join(HANGING_LIGHT_DIR, "texture_diffuse.png"), 0.35))
    log("reflector colgante importado (/Game/Dressing/Props/SM_StageLight_Hanging)")
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
            actor.get_component_by_class(unreal.SkyLightComponent).set_intensity(0.55)
        elif label == "Pit_Floor":
            # T-0041 (auditoría): era una franja negra pura abajo. Ahora es la calle: adoquín mojado azul grisáceo, oscuro
            # pero con forma (el color lo trae la textura; el override solo la apaga un poco).
            actor.set_editor_property("color_override", unreal.LinearColor(0.55, 0.57, 0.65, 1.0))
            try:
                actor.set_editor_property("pattern_override", "T_Street_Cobble")
                actor.set_editor_property("pattern_override_strength", 1.0)
            except Exception as e:
                unreal.log_warning(f"[TDC] Pit_Floor sin patrón (¿C++ viejo?): {e}")
                actor.set_editor_property("color_override", unreal.LinearColor(0.06, 0.065, 0.08, 1.0))
        elif label == "Height_Fog":
            # Poca niebla y solo lejos: el telón ya trae su bruma pintada; la niebla densa lo lavaba.
            fog = actor.get_component_by_class(unreal.ExponentialHeightFogComponent)
            fog.set_editor_property("fog_density", 0.008)
            fog.set_editor_property("start_distance", 4000.0)
            fog.set_editor_property("fog_inscattering_luminance", unreal.LinearColor(0.03, 0.04, 0.08, 1.0))
            # Volumétrica: el reflector encendido dibuja su halo (= su alcance) en el aire.
            try:
                # UE 5.8: la propiedad es bEnableVolumetricFog -> "enable_volumetric_fog" en Python.
                try:
                    fog.set_editor_property("enable_volumetric_fog", True)
                except Exception:
                    fog.set_editor_property("volumetric_fog", True)
                fog.set_editor_property("volumetric_fog_start_distance", 0.0)
                fog.set_editor_property("volumetric_fog_extinction_scale", 0.6)
            except Exception as e:
                unreal.log_warning(f"[TDC] niebla volumétrica no disponible ({e})")


PLAY_KEYS = [300.0, 1500.0, 2700.0, 3900.0, 5100.0, 6300.0, 7500.0]  # X de las luces de relleno del plano de juego


def playfield_key_lights():
    """Luz de relleno ámbar apagado desde el lado de la cámara: el plano de juego (piso, plataformas, personajes) tiene que
    ser lo más legible de la pantalla. Ámbar desaturado, no el amarillo de la mecánica de luz."""
    for i, x in enumerate(PLAY_KEYS):
        actor = eas.spawn_actor_from_class(unreal.PointLight, vec(x, 700.0, 450.0), rot())
        actor.set_actor_label(f"DR_PlayKey_{i}")
        actor.set_folder_path("Dressing/Lights")
        comp = actor.get_component_by_class(unreal.PointLightComponent)
        comp.set_mobility(unreal.ComponentMobility.MOVABLE)
        comp.set_intensity(18.0)
        comp.set_attenuation_radius(1400.0)
        comp.set_light_color(unreal.LinearColor(1.0, 0.82, 0.62, 1.0))
        comp.set_editor_property("volumetric_scattering_intensity", 0.0)  # sin halo: el halo es del reflector
        comp.set_cast_shadows(False)
    log(f"luces de relleno del plano de juego: {len(PLAY_KEYS)}")


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
    # Adaptación casi instantánea: el build arrancaba oscuro (frame 0) mientras el auto-exposure se adaptaba.
    for name, value in (("auto_exposure_speed_up", 20.0),
                        ("auto_exposure_speed_down", 20.0),
                        ("auto_exposure_bias", 0.0),
                        ("vignette_intensity", 0.45),
                        ("motion_blur_amount", 0.0),  # las capturas salían borrosas y en 2.5D no suma
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
    card_practical_lights()
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
    # T-0041 (auditoría #3): era la franja negra pura del 22 % inferior. Ahora es vereda de adoquín mojado.
    stage.set_editor_property("color_override", unreal.LinearColor(0.5, 0.52, 0.6, 1.0))
    try:
        stage.set_editor_property("pattern_override", "T_Street_Cobble")
        stage.set_editor_property("pattern_override_strength", 1.0)
    except Exception as e:
        unreal.log_warning(f"[TDC] DR_ForegroundStage sin patrón (¿C++ viejo?): {e}")
        stage.set_editor_property("color_override", unreal.LinearColor(0.06, 0.065, 0.08, 1.0))

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

    import_hanging_light()
    set_cache = {}
    placed = 0
    for i, (folder_name, file_name, x, y, yaw, height, tint) in enumerate(SET_PROPS):
        if folder_name not in set_cache:
            set_cache[folder_name] = import_set_prop(folder_name, file_name, tint)
        sm = set_cache[folder_name]
        if not sm:
            continue
        extent = sm.get_bounds().box_extent
        s = height / max(1.0, extent.z * 2.0)
        spawn_mesh(f"DR_SetProp_{i}_{folder_name}", sm, (x, y, FOREGROUND_TOP_Z), rot(yaw=yaw), vec(s, s, s))
        placed += 1
    log(f"utilería de set colocada: {placed}")

    night_lighting()
    playfield_key_lights()
    post_process()
    dynamic_lighting_only()

    if not les.save_current_level():
        fail(f"No se pudo guardar {MAP_PATH} (¿read-only por LFS? git lfs lock Game/Content/Maps/L_Slice.umap)")
    log("ambientación noir aplicada y L_Slice guardado.")


build()
