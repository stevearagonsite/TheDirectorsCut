"""
Crea (o rehace) /Game/Materials/M_TDCColor: material base (DefaultLit) de las piezas greybox con color de lectura.

Parámetros:
  - Color    (Vector) -> Base Color. Lo setea el C++ (TDC::ApplyReadColor) con SetVectorParameterValue("Color", ...).
  - Emissive (Vector) -> Emissive Color. Negro por defecto; sirve para que el bulbo del reflector brille encendido.
Roughness 0.7, Metallic 0.

Por qué existe: en T-0008 el personaje sale gris; el parámetro "Color" de BasicShapeMaterial no se aplica en 5.8,
y sin color no se leen las mecánicas (amarillo = solo con luz, violeta = solo a oscuras, celeste = en toma).

Correr en Windows (idempotente). Si M_TDCColor ya está en el repo: antes `git lfs lock Game/Content/Materials/M_TDCColor.uasset`.
  .\\Tools\\scripts\\tdc.ps1 python -Script Tools\\unreal\\make_materials.py
  (si el commandlet no puede compilar el material, agregar -InEditor)
"""

import os

import unreal

FOLDER = "/Game/Materials"
NAME = "M_TDCColor"
PATH = f"{FOLDER}/{NAME}"

mel = unreal.MaterialEditingLibrary
eal = unreal.EditorAssetLibrary


def fail(message):
    """Error claro en el log; tdc.ps1 python busca "[TDC] ERROR" y corta con código distinto de 0."""
    unreal.log_error(f"[TDC] ERROR: {message}")
    raise RuntimeError(message)


def require_writable(asset_path, ext):
    """Los .umap/.uasset son lockables de LFS: sin `git lfs lock` quedan read-only y el guardado falla en silencio."""
    rel = asset_path.replace("/Game/", "", 1) + ext
    disk = os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_content_dir()), rel)
    if os.path.exists(disk) and not os.access(disk, os.W_OK):
        fail(f"Game/Content/{rel} es read-only (lockable de LFS sin lock). Antes: git lfs lock Game/Content/{rel}")


def get_or_create_material():
    if eal.does_asset_exist(PATH):
        mat = eal.load_asset(PATH)
        mel.delete_all_material_expressions(mat)
        return mat
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    return tools.create_asset(NAME, FOLDER, unreal.Material, unreal.MaterialFactoryNew())


def vector_param(mat, name, default, y):
    node = mel.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -500, y)
    node.set_editor_property("parameter_name", name)
    node.set_editor_property("default_value", unreal.LinearColor(*default))
    return node


def constant(mat, value, y):
    node = mel.create_material_expression(mat, unreal.MaterialExpressionConstant, -500, y)
    node.set_editor_property("r", value)
    return node


def build():
    require_writable(PATH, ".uasset")
    mat = get_or_create_material()
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)

    color = vector_param(mat, "Color", (0.35, 0.35, 0.38, 1.0), 0)
    mel.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)

    emissive = vector_param(mat, "Emissive", (0.0, 0.0, 0.0, 1.0), 250)
    mel.connect_material_property(emissive, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

    mel.connect_material_property(constant(mat, 0.7, 500), "", unreal.MaterialProperty.MP_ROUGHNESS)
    mel.connect_material_property(constant(mat, 0.0, 600), "", unreal.MaterialProperty.MP_METALLIC)

    mel.recompile_material(mat)
    if not eal.save_asset(PATH, only_if_is_dirty=False):
        fail(f"No se pudo guardar {PATH} (¿read-only por LFS? git lfs lock Game/Content/Materials/{NAME}.uasset)")
    unreal.log(f"[TDC] {PATH} creado y guardado.")


try:
    build()
finally:
    # Lanzado con -ExecutePythonScript (tdc.ps1 python -InEditor): cerrar el editor al terminar, también si falló.
    if "-executepythonscript" in unreal.SystemLibrary.get_command_line().lower():
        unreal.SystemLibrary.quit_editor()
