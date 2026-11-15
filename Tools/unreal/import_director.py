"""
Importa el protagonista (Rodin + auto-rig con nombres Mixamo) a /Game/Characters/Director/.

Correr en Windows:
  .\\Tools\\scripts\\tdc.ps1 python -Script Tools\\unreal\\import_director.py
  (o en el editor: Output Log > Python:  py "<repo>/Tools/unreal/import_director.py")

Entrada:  Art/Export/Director/Director_rigged_anims.fbx (mesh + skeleton + animaciones Idle, Walking, Jogging, Sprint, Jumping).
Opcional: Art/Export/Director/SM_Clapperboard.fbx -> SM_Clapperboard (prop del claquetazo, T-0009).
Salida:   SK_Director (skeletal mesh, ~180 cm), SKEL_Director, y las animaciones renombradas:
          AS_Director_Idle, AS_Director_Walk, AS_Director_Run (Jogging), AS_Director_Sprint, AS_Director_Jump.
ATDCCharacter carga estos nombres por ruta (T-0007): no cambiarlos sin cambiar el C++.

Usa el importador FBX clásico (desactiva Interchange para FBX) porque sus opciones son estables desde Python.
Es idempotente: reimporta encima.
"""

import os
import unreal

DEST = "/Game/Characters/Director"
REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
FBX = os.path.join(REPO, "Art", "Export", "Director", "Director_rigged_anims.fbx")
TARGET_HEIGHT_CM = 180.0
# Prop opcional (T-0009): la claqueta, pivote en el agarre. Se engancha al hueso RightHand desde C++.
THROW_FBX = os.path.join(REPO, "Art", "Export", "Director", "Director_anim_throw.fbx")  # opcional: AS_Director_Clap
CLAPPER_FBX = os.path.join(REPO, "Art", "Export", "Director", "SM_Clapperboard.fbx")

# Palabra clave en el nombre de la animación importada -> nombre final. El orden importa ("sprint" antes que "run").
ANIM_NAMES = [
    ("idle", "AS_Director_Idle"),
    ("walk", "AS_Director_Walk"),
    ("sprint", "AS_Director_Sprint"),
    ("jog", "AS_Director_Run"),
    ("run", "AS_Director_Run"),
    ("jump", "AS_Director_Jump"),
    ("throw", "AS_Director_Clap"),
    ("clap", "AS_Director_Clap"),
]

eal = unreal.EditorAssetLibrary


def log(msg):
    unreal.log("[TDC] " + msg)


def import_fbx(scale):
    ui = unreal.FbxImportUI()
    ui.import_mesh = True
    ui.import_as_skeletal = True
    ui.mesh_type_to_import = unreal.FBXImportType.FBXIT_SKELETAL_MESH
    ui.import_animations = True
    ui.import_materials = True
    ui.import_textures = True
    ui.create_physics_asset = True
    ui.automated_import_should_detect_type = False
    ui.skeletal_mesh_import_data.import_uniform_scale = scale
    ui.skeletal_mesh_import_data.set_editor_property("import_morph_targets", False)
    ui.anim_sequence_import_data.import_uniform_scale = scale
    ui.anim_sequence_import_data.set_editor_property(
        "animation_length", unreal.FBXAnimationLengthImportType.FBXALIT_EXPORTED_TIME)

    task = unreal.AssetImportTask()
    task.filename = FBX
    task.destination_path = DEST
    task.destination_name = "SK_Director"
    task.replace_existing = True
    task.automated = True
    task.save = True
    task.options = ui
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return list(task.imported_object_paths or [])


def import_clapper():
    if not os.path.isfile(CLAPPER_FBX):
        log("sin SM_Clapperboard.fbx: el claquetazo va sin prop")
        return
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
    task = unreal.AssetImportTask()
    task.filename = CLAPPER_FBX
    task.destination_path = DEST
    task.destination_name = "SM_Clapperboard"
    task.replace_existing = True
    task.automated = True
    task.save = True
    task.options = ui
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    sm = eal.load_asset(DEST + "/SM_Clapperboard")
    if sm:
        log("SM_Clapperboard importado (alto %.1f cm)" % (sm.get_bounds().box_extent.z * 2.0))
    else:
        log("SM_Clapperboard NO se importó")


def import_throw(scale):
    """Animación del claquetazo: FBX aparte con el mismo rig; se importa solo la animación sobre SKEL_Director."""
    if not os.path.isfile(THROW_FBX):
        log("sin Director_anim_throw.fbx: el claquetazo va sin animación")
        return
    skel = eal.load_asset(DEST + "/SKEL_Director")
    if not skel:
        log("sin SKEL_Director: no puedo importar el throw")
        return
    ui = unreal.FbxImportUI()
    ui.import_mesh = False
    ui.import_as_skeletal = True
    ui.mesh_type_to_import = unreal.FBXImportType.FBXIT_ANIMATION
    ui.import_animations = True
    ui.import_materials = False
    ui.import_textures = False
    ui.skeleton = skel
    ui.automated_import_should_detect_type = False
    ui.anim_sequence_import_data.import_uniform_scale = scale
    ui.anim_sequence_import_data.set_editor_property(
        "animation_length", unreal.FBXAnimationLengthImportType.FBXALIT_EXPORTED_TIME)
    task = unreal.AssetImportTask()
    task.filename = THROW_FBX
    task.destination_path = DEST + "/Throw"
    task.destination_name = "AS_Director_Clap"
    task.replace_existing = True
    task.automated = True
    task.save = True
    task.options = ui
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    anims = [eal.load_asset(p) for p in eal.list_assets(DEST + "/Throw", recursive=True, include_folder=False)]
    anims = [a for a in anims if isinstance(a, unreal.AnimSequence)]
    if not anims:
        log("el throw NO produjo animación")
        return
    if eal.does_asset_exist(DEST + "/AS_Director_Clap"):
        eal.delete_asset(DEST + "/AS_Director_Clap")
    eal.rename_asset(anims[0].get_path_name().split(".")[0], DEST + "/AS_Director_Clap")
    log("anim %s -> AS_Director_Clap (%.2f s)" % (anims[0].get_name(), anims[0].get_play_length()))


def find(cls):
    out = []
    for path in eal.list_assets(DEST, recursive=False, include_folder=False):
        obj = eal.load_asset(path)
        if isinstance(obj, cls):
            out.append(obj)
    return out


def mesh_height(sk):
    box = sk.get_bounds().box_extent  # media altura de la caja (cm)
    return box.z * 2.0


def main():
    if not os.path.isfile(FBX):
        raise RuntimeError("No existe " + FBX)
    unreal.SystemLibrary.execute_console_command(None, "Interchange.FeatureFlags.Import.FBX False")

    scale = 1.0
    paths = import_fbx(scale)
    log("importado: %d objetos" % len(paths))
    meshes = find(unreal.SkeletalMesh)
    if not meshes:
        raise RuntimeError("El FBX no produjo un SkeletalMesh")
    sk = meshes[0]
    h = mesh_height(sk)
    log("altura importada: %.1f cm" % h)
    if h > 1 and abs(h - TARGET_HEIGHT_CM) > 10:
        scale = TARGET_HEIGHT_CM / h
        log("reimportando con escala %.4f" % scale)
        import_fbx(scale)
        sk = find(unreal.SkeletalMesh)[0]
        log("altura final: %.1f cm" % mesh_height(sk))

    # Nombres estables para el C++.
    if sk.get_name() != "SK_Director":
        eal.rename_asset(sk.get_path_name().split(".")[0], DEST + "/SK_Director")
    for skel in find(unreal.Skeleton):
        if skel.get_name() != "SKEL_Director":
            eal.rename_asset(skel.get_path_name().split(".")[0], DEST + "/SKEL_Director")
    taken = set()
    for anim in find(unreal.AnimSequence):
        name = anim.get_name()
        low = name.lower()
        for key, final in ANIM_NAMES:
            if key in low and final not in taken:
                if name != final:
                    eal.rename_asset(anim.get_path_name().split(".")[0], DEST + "/" + final)
                taken.add(final)
                log("anim %s -> %s (%.2f s)" % (name, final, anim.get_play_length()))
                break
        else:
            log("anim sin mapear: " + name)

    import_throw(scale)
    import_clapper()
    eal.save_directory(DEST, only_if_is_dirty=False, recursive=True)
    missing = sorted(n for n in ("AS_Director_Idle", "AS_Director_Run", "AS_Director_Jump") if n not in taken)
    if missing:
        log("FALTAN animaciones: " + ", ".join(missing))
    log("Director importado en " + DEST)


main()
