"""
Extras animados (Rodin) para The Director's Cut (T-0019). Correr en el editor:
  .\\Tools\\scripts\\tdc.ps1 python -Script Tools\\unreal\\import_extras.py
Antes: git lfs lock de Game/Content/Characters/Extras/** si ya existen.

Por cada Art/Export/Extras/<Comun|Resistente|Sombra>.fbx (mesh + skin + animaciones Idle/Walking de Rodin):
  /Game/Characters/Extras/<Tipo>/SK_Extra_<Tipo>, SKEL_Extra_<Tipo>, AS_Extra_<Tipo>_Idle y AS_Extra_<Tipo>_Walk.
ATDCExtra los carga por ruta según su vulnerabilidad; si no existen, queda el greybox.
Importador FBX clásico (Interchange off), como import_director.py. Import limpio por tipo.
"""

import os

import unreal

ROOT = "/Game/Characters/Extras"
REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
SRC = os.path.join(REPO, "Art", "Export", "Extras")
TYPES = {"Comun": 175.0, "Resistente": 190.0, "Sombra": 180.0}
eal = unreal.EditorAssetLibrary


def log(msg):
    unreal.log("[TDC] " + msg)


def fail(message):
    unreal.log_error(f"[TDC] ERROR: {message}")
    raise RuntimeError(message)


def import_fbx(fbx, dest, name, scale):
    ui = unreal.FbxImportUI()
    ui.import_mesh = True
    ui.import_as_skeletal = True
    ui.mesh_type_to_import = unreal.FBXImportType.FBXIT_SKELETAL_MESH
    ui.import_animations = True
    ui.import_materials = True
    ui.import_textures = True
    ui.create_physics_asset = False
    ui.automated_import_should_detect_type = False
    ui.skeletal_mesh_import_data.import_uniform_scale = scale
    ui.skeletal_mesh_import_data.set_editor_property("import_morph_targets", False)
    ui.anim_sequence_import_data.import_uniform_scale = scale
    ui.anim_sequence_import_data.set_editor_property(
        "animation_length", unreal.FBXAnimationLengthImportType.FBXALIT_EXPORTED_TIME)
    task = unreal.AssetImportTask()
    task.filename = fbx
    task.destination_path = dest
    task.destination_name = name
    task.replace_existing = True
    task.automated = True
    task.save = True
    task.options = ui
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])


def find(dest, cls):
    out = []
    for path in eal.list_assets(dest, recursive=False, include_folder=False):
        obj = eal.load_asset(path)
        if isinstance(obj, cls):
            out.append(obj)
    return out


def clean(dest):
    if eal.does_directory_exist(dest):
        for path in eal.list_assets(dest, recursive=True, include_folder=False):
            if not eal.delete_asset(path.split(".")[0]):
                fail(f"no pude borrar {path} (¿falta git lfs lock?)")


def rename(obj, dest, final):
    if obj.get_name() != final:
        eal.rename_asset(obj.get_path_name().split(".")[0], f"{dest}/{final}")


MEL = unreal.MaterialEditingLibrary
# Tono: el rojo y el naranja de Rodin salieron algo vivos para el noir; se apagan un poco.
TINT = {"Comun": 0.8, "Resistente": 0.8, "Sombra": 1.0}


def matte_material(kind, dest, sk):
    """Material mate como el concepto (sin el metallic de Rodin), con la difusa y la normal importadas."""
    textures = [eal.load_asset(p) for p in eal.list_assets(dest, recursive=True, include_folder=False)]
    textures = [t for t in textures if isinstance(t, unreal.Texture2D)]
    diffuse = next((t for t in textures if "diffuse" in t.get_name().lower() or "basecolor" in t.get_name().lower()), None)
    normal = next((t for t in textures if "normal" in t.get_name().lower()), None)
    if not diffuse:
        log(f"{kind}: sin textura difusa, queda el material de Rodin")
        return
    path = f"{dest}/M_Extra_{kind}"
    mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(f"M_Extra_{kind}", dest, unreal.Material,
                                                                  unreal.MaterialFactoryNew())
    base = MEL.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -500, -100)
    base.set_editor_property("texture", diffuse)
    tint = MEL.create_material_expression(mat, unreal.MaterialExpressionMultiply, -250, -100)
    tint.set_editor_property("const_b", TINT.get(kind, 1.0))
    MEL.connect_material_expressions(base, "RGB", tint, "A")
    MEL.connect_material_property(tint, "", unreal.MaterialProperty.MP_BASE_COLOR)
    if normal:
        normal.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
        normal.set_editor_property("srgb", False)
        nrm = MEL.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -500, 200)
        nrm.set_editor_property("texture", normal)
        nrm.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL)
        MEL.connect_material_property(nrm, "RGB", unreal.MaterialProperty.MP_NORMAL)
    for prop, value, y in ((unreal.MaterialProperty.MP_METALLIC, 0.0, 400),
                           (unreal.MaterialProperty.MP_ROUGHNESS, 0.85, 480),
                           (unreal.MaterialProperty.MP_SPECULAR, 0.25, 560)):
        const = MEL.create_material_expression(mat, unreal.MaterialExpressionConstant, -250, y)
        const.set_editor_property("r", value)
        MEL.connect_material_property(const, "", prop)
    MEL.recompile_material(mat)
    eal.save_asset(path)
    sk.modify()
    slots = sk.get_editor_property("materials")
    for i in range(len(slots)):  # en 5.8 el for devuelve copias: reasignar por índice
        slot = slots[i]
        slot.set_editor_property("material_interface", mat)
        slots[i] = slot
    sk.set_editor_property("materials", slots)
    eal.save_asset(sk.get_path_name(), only_if_is_dirty=False)


def import_type(kind, target_cm):
    fbx = os.path.join(SRC, kind + ".fbx")
    if not os.path.isfile(fbx):
        log(f"sin {kind}.fbx: ese extra queda en greybox")
        return
    dest = f"{ROOT}/{kind}"
    clean(dest)
    name = f"SK_Extra_{kind}"
    import_fbx(fbx, dest, name, 1.0)
    meshes = find(dest, unreal.SkeletalMesh)
    if not meshes:
        fail(f"{kind}.fbx no produjo SkeletalMesh")
    h = meshes[0].get_bounds().box_extent.z * 2.0
    if h > 1 and abs(h - target_cm) > 10:
        clean(dest)
        import_fbx(fbx, dest, name, target_cm / h)
        meshes = find(dest, unreal.SkeletalMesh)
        h = meshes[0].get_bounds().box_extent.z * 2.0
    rename(meshes[0], dest, name)
    # NO renombrar el Skeleton: rename_asset actualiza las referencias de los AS solo en memoria y no los guarda;
    # en disco los clips quedaban apuntando a un skeleton que ya no existe ("mismo esqueleto=False") y no animaban.
    taken = set()
    anims = find(dest, unreal.AnimSequence)
    rest = []
    for anim in anims:
        low = anim.get_name().lower()
        for key, suffix in (("idle", "Idle"), ("walk", "Walk")):
            final = f"AS_Extra_{kind}_{suffix}"
            if key in low and final not in taken:
                rename(anim, dest, final)
                taken.add(final)
                break
        else:
            rest.append(anim)
    # Rodin las nombra Armature|human_0000xx: el ciclo de caminar es el más corto, la idle el más largo.
    rest.sort(key=lambda a: a.get_play_length())
    for anim, suffix in zip(rest, ("Walk", "Idle")):
        final = f"AS_Extra_{kind}_{suffix}"
        if final not in taken:
            rename(anim, dest, final)
            taken.add(final)
    matte_material(kind, dest, eal.load_asset(f"{dest}/{name}"))
    # Guardar todo lo que los renames dejaron sucio (clips, mesh) y verificar que los clips usan el skeleton del mesh.
    eal.save_directory(dest, only_if_is_dirty=False, recursive=True)
    sk_skel = eal.load_asset(f"{dest}/{name}").get_editor_property("skeleton")
    for suffix in ("Idle", "Walk"):
        path = f"{dest}/AS_Extra_{kind}_{suffix}"
        anim = eal.load_asset(path) if eal.does_asset_exist(path) else None
        if anim and anim.get_editor_property("skeleton") != sk_skel:
            fail(f"{path} no usa el skeleton de SK_Extra_{kind}")
    log(f"{kind}: {h:.0f} cm, animaciones {sorted(taken)}")


def main():
    unreal.SystemLibrary.execute_console_command(None, "Interchange.FeatureFlags.Import.FBX False")
    for kind, target in TYPES.items():
        import_type(kind, target)


main()
