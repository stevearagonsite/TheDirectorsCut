"""
Look y locomoción del director (T-0012). Correr DESPUÉS de import_director.py:
  .\\Tools\\scripts\\tdc.ps1 python -Script Tools\\unreal\\fix_director_look.py

1) M_Director: material NO metálico como el concepto (estilizado, colores planos). Usa la textura difusa y la
   normal que importó Rodin, con Metallic 0, Roughness 0.85 y Specular 0.25. Lo asigna a todos los slots de
   SK_Director. Rodin trae un mapa metallic que en UE hacía ver al personaje metalizado.
2) BS_Director_Locomotion: BlendSpace1D por velocidad (cm/s): Idle 0, Walk 160, Run 380, Sprint 600.
   ATDCCharacter lo usa si existe (transiciones suaves). Si algo falla, se borra y el C++ cae al modo por clips.
Idempotente.
"""

import unreal

DEST = "/Game/Characters/Director"
eal = unreal.EditorAssetLibrary
mel = unreal.MaterialEditingLibrary
tools = unreal.AssetToolsHelpers.get_asset_tools()


def log(msg):
    unreal.log("[TDC] " + msg)


def fail(message):
    unreal.log_error(f"[TDC] ERROR: {message}")
    raise RuntimeError(message)


def find_texture(keyword):
    for path in eal.list_assets(DEST, recursive=True, include_folder=False):
        asset = eal.load_asset(path)
        if isinstance(asset, unreal.Texture2D) and keyword in asset.get_name().lower():
            return asset
    return None


def director_material(sk):
    diffuse = find_texture("diffuse") or find_texture("basecolor") or find_texture("albedo")
    normal = find_texture("normal")
    if not diffuse:
        fail("no encontré la textura difusa del director en " + DEST)
    path = f"{DEST}/M_Director"
    mat = eal.load_asset(path) if eal.does_asset_exist(path) else None
    if not mat:
        mat = tools.create_asset("M_Director", DEST, unreal.Material, unreal.MaterialFactoryNew())
    mel.delete_all_material_expressions(mat)
    base = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -500, -100)
    base.set_editor_property("texture", diffuse)
    mel.connect_material_property(base, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
    if normal:
        normal.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
        normal.set_editor_property("srgb", False)
        nrm = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -500, 200)
        nrm.set_editor_property("texture", normal)
        nrm.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL)
        mel.connect_material_property(nrm, "RGB", unreal.MaterialProperty.MP_NORMAL)
    for prop, value, y in ((unreal.MaterialProperty.MP_METALLIC, 0.0, 400),
                           (unreal.MaterialProperty.MP_ROUGHNESS, 0.85, 480),
                           (unreal.MaterialProperty.MP_SPECULAR, 0.25, 560)):
        const = mel.create_material_expression(mat, unreal.MaterialExpressionConstant, -250, y)
        const.set_editor_property("r", value)
        mel.connect_material_property(const, "", prop)
    mel.recompile_material(mat)
    eal.save_asset(path)

    sk.modify()
    materials = sk.get_editor_property("materials")
    for slot in materials:
        slot.set_editor_property("material_interface", mat)
    sk.set_editor_property("materials", materials)
    # en commandlet set_editor_property no marca el paquete dirty: forzar el guardado
    eal.save_asset(sk.get_path_name(), only_if_is_dirty=False)
    log(f"M_Director asignado a {len(materials)} slot(s) (sin metallic)")


def locomotion_blendspace(sk):
    anims = {name: eal.load_asset(f"{DEST}/{name}") for name in
             ("AS_Director_Idle", "AS_Director_Walk", "AS_Director_Run", "AS_Director_Sprint")}
    samples_def = [("AS_Director_Idle", 0.0), ("AS_Director_Walk", 160.0),
                   ("AS_Director_Run", 380.0), ("AS_Director_Sprint", 600.0)]
    samples_def = [(n, v) for n, v in samples_def if anims.get(n)]
    if len(samples_def) < 2:
        log("faltan animaciones de locomoción: sin blendspace")
        return
    path = f"{DEST}/BS_Director_Locomotion"
    if eal.does_asset_exist(path):
        eal.delete_asset(path)
    try:
        factory = unreal.BlendSpaceFactory1D()
        factory.set_editor_property("target_skeleton", sk.skeleton)
        bs = tools.create_asset("BS_Director_Locomotion", DEST, unreal.BlendSpace1D, factory)
        params = bs.get_editor_property("blend_parameters")
        p0 = params[0]
        p0.set_editor_property("display_name", "Speed")
        p0.set_editor_property("min", 0.0)
        p0.set_editor_property("max", 600.0)
        p0.set_editor_property("grid_num", 4)
        params[0] = p0
        bs.set_editor_property("blend_parameters", params)
        samples = []
        for name, value in samples_def:
            s = unreal.BlendSample()
            s.set_editor_property("animation", anims[name])
            s.set_editor_property("sample_value", unreal.Vector(value, 0.0, 0.0))
            samples.append(s)
        bs.set_editor_property("sample_data", samples)
        eal.save_asset(path)
        log(f"BS_Director_Locomotion con {len(samples)} muestras")
    except Exception as e:  # el C++ cae a clips sueltos si no hay blendspace
        unreal.log_warning(f"[TDC] no se pudo armar el blendspace ({e}); queda el modo por clips")
        if eal.does_asset_exist(path):
            eal.delete_asset(path)


def main():
    sk = eal.load_asset(f"{DEST}/SK_Director")
    if not sk:
        fail("no existe SK_Director: correr import_director.py antes")
    director_material(sk)
    locomotion_blendspace(sk)


main()
