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
BACKDROP_WIDTH = 15000.0
BACKDROP_HEIGHT = BACKDROP_WIDTH / 2.357
# Más ancho que el nivel y con el borde inferior bajo el fondo del pozo (z=-900): no se ven bordes.
BACKDROP_CENTER = (3750.0, -3000.0, -900.0 + BACKDROP_HEIGHT / 2.0)

# Props en primer plano: (archivo sin extensión, X, Y, yaw, alto deseado en cm).
PROPS = [
    ("SM_Spotlight", -150.0, 650.0, 30.0, 220.0),
    ("SM_FilmCamera", 2600.0, 750.0, -20.0, 190.0),
    ("SM_Crates", 4400.0, 650.0, 10.0, 160.0),
    ("SM_Spotlight", 6900.0, 700.0, -35.0, 220.0),
    ("SM_Crates", 7900.0, 600.0, 0.0, 160.0),
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


def import_file(path, name, options=None):
    task = unreal.AssetImportTask()
    task.filename = path
    task.destination_path = DEST
    task.destination_name = name
    task.replace_existing = True
    task.automated = True
    task.save = True
    if options:
        task.options = options
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return eal.load_asset(f"{DEST}/{name}")


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
    mul = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -150, 0)
    mul.set_editor_property("const_b", 0.85)  # un poco más oscuro que la imagen para que el nivel destaque
    mel.connect_material_expressions(sample, "RGB", mul, "A")
    mel.connect_material_property(mul, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.recompile_material(mat)
    eal.save_asset(path)
    return mat


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
    sm = import_file(fbx, name, ui)
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
        elif label == "Height_Fog":
            # Poca niebla y solo lejos: el telón ya trae su bruma pintada; la niebla densa lo lavaba.
            fog = actor.get_component_by_class(unreal.ExponentialHeightFogComponent)
            fog.set_editor_property("fog_density", 0.008)
            fog.set_editor_property("start_distance", 4000.0)
            fog.set_editor_property("fog_inscattering_luminance", unreal.LinearColor(0.03, 0.04, 0.08, 1.0))


def post_process():
    """Look de cine negro: exposición más baja (el auto-exposure aclaraba todo), viñeta, grano y menos saturación."""
    ppv = eas.spawn_actor_from_class(unreal.PostProcessVolume, vec(0.0, 0.0, 0.0), rot())
    ppv.set_actor_label("DR_PostProcess")
    ppv.set_folder_path("Dressing")
    ppv.set_editor_property("unbound", True)
    st = ppv.get_editor_property("settings")
    for name, value in (("auto_exposure_bias", -1.2),
                        ("vignette_intensity", 0.55),
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
    les.load_level(MAP_PATH)
    clear_dressing()

    mat = backdrop_material()
    cube = unreal.load_asset(CUBE)
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

    if not les.save_current_level():
        fail(f"No se pudo guardar {MAP_PATH} (¿read-only por LFS? git lfs lock Game/Content/Maps/L_Slice.umap)")
    log("ambientación noir aplicada y L_Slice guardado.")


build()
