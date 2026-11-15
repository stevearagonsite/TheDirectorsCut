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


TEX_SRC = os.path.join(os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..")), "Art", "Export", "Textures")
TEX_DEST = f"{FOLDER}/Textures"
PATTERNS = ("T_Stage_Floor", "T_Stage_Edge", "T_Gel_Panel", "T_Take_Holo")


def import_patterns():
    """Texturas tileables del set (T-0021) -> /Game/Materials/Textures/T_*. Devuelve la de tablones (default)."""
    first = None
    for name in PATTERNS:
        png = os.path.join(TEX_SRC, name + ".png")
        if not os.path.isfile(png):
            continue
        require_writable(f"{TEX_DEST}/{name}", ".uasset")
        task = unreal.AssetImportTask()
        task.filename = png
        task.destination_path = TEX_DEST
        task.destination_name = name
        task.replace_existing = True
        task.automated = True
        task.save = True
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        path = f"{TEX_DEST}/{name}"
        tex = eal.load_asset(path) if eal.does_asset_exist(path) else None
        if tex and first is None:
            first = tex
    return first


def world_aligned_uv(mat, tile):
    """UV por posición de mundo (no se estira con cubos escalados): caras frontales usan XZ, techos XY."""
    wp = mel.create_material_expression(mat, unreal.MaterialExpressionWorldPosition, -1500, -300)
    xz = mel.create_material_expression(mat, unreal.MaterialExpressionComponentMask, -1300, -350)
    for ch, on in (("r", True), ("g", False), ("b", True), ("a", False)):
        xz.set_editor_property(ch, on)
    mel.connect_material_expressions(wp, "", xz, "")
    xy = mel.create_material_expression(mat, unreal.MaterialExpressionComponentMask, -1300, -250)
    for ch, on in (("r", True), ("g", True), ("b", False), ("a", False)):
        xy.set_editor_property(ch, on)
    mel.connect_material_expressions(wp, "", xy, "")
    nrm = mel.create_material_expression(mat, unreal.MaterialExpressionVertexNormalWS, -1500, -150)
    nz = mel.create_material_expression(mat, unreal.MaterialExpressionComponentMask, -1300, -150)
    for ch, on in (("r", False), ("g", False), ("b", True), ("a", False)):
        nz.set_editor_property(ch, on)
    mel.connect_material_expressions(nrm, "", nz, "")
    absz = mel.create_material_expression(mat, unreal.MaterialExpressionAbs, -1150, -150)
    mel.connect_material_expressions(nz, "", absz, "")
    up = mel.create_material_expression(mat, unreal.MaterialExpressionRound, -1050, -150)
    mel.connect_material_expressions(absz, "", up, "")
    pick = mel.create_material_expression(mat, unreal.MaterialExpressionLinearInterpolate, -950, -300)
    mel.connect_material_expressions(xz, "", pick, "A")
    mel.connect_material_expressions(xy, "", pick, "B")
    mel.connect_material_expressions(up, "", pick, "Alpha")
    uv = mel.create_material_expression(mat, unreal.MaterialExpressionDivide, -800, -300)
    mel.connect_material_expressions(pick, "", uv, "A")
    mel.connect_material_expressions(tile, "", uv, "B")
    return uv


def build():
    require_writable(PATH, ".uasset")
    default_pattern = import_patterns()
    mat = get_or_create_material()
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)

    color = vector_param(mat, "Color", (0.35, 0.35, 0.38, 1.0), 0)
    if default_pattern:
        # BaseColor = Color * lerp(1, Pattern, PatternStrength). PatternStrength 0 = color liso (personaje, extras, bulbo).
        tile = mel.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -1100, -450)
        tile.set_editor_property("parameter_name", "TileSize")
        tile.set_editor_property("default_value", 200.0)
        uv = world_aligned_uv(mat, tile)
        pattern = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -600, -300)
        pattern.set_editor_property("parameter_name", "Pattern")
        pattern.set_editor_property("texture", default_pattern)
        mel.connect_material_expressions(uv, "", pattern, "UVs")
        strength = mel.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -600, -100)
        strength.set_editor_property("parameter_name", "PatternStrength")
        strength.set_editor_property("default_value", 0.0)
        white = mel.create_material_expression(mat, unreal.MaterialExpressionConstant, -600, -180)
        white.set_editor_property("r", 1.0)
        mix = mel.create_material_expression(mat, unreal.MaterialExpressionLinearInterpolate, -400, -200)
        mel.connect_material_expressions(white, "", mix, "A")
        mel.connect_material_expressions(pattern, "RGB", mix, "B")
        mel.connect_material_expressions(strength, "", mix, "Alpha")
        tinted = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -250, 0)
        mel.connect_material_expressions(color, "", tinted, "A")
        mel.connect_material_expressions(mix, "", tinted, "B")
        mel.connect_material_property(tinted, "", unreal.MaterialProperty.MP_BASE_COLOR)
    else:
        mel.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)

    emissive = vector_param(mat, "Emissive", (0.0, 0.0, 0.0, 1.0), 250)
    mel.connect_material_property(emissive, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

    mel.connect_material_property(constant(mat, 0.7, 500), "", unreal.MaterialProperty.MP_ROUGHNESS)
    mel.connect_material_property(constant(mat, 0.0, 600), "", unreal.MaterialProperty.MP_METALLIC)

    mel.recompile_material(mat)
    if not eal.save_asset(PATH, only_if_is_dirty=False):
        fail(f"No se pudo guardar {PATH} (¿read-only por LFS? git lfs lock Game/Content/Materials/{NAME}.uasset)")
    unreal.log(f"[TDC] {PATH} creado y guardado.")


LIGHT_PATH = f"{FOLDER}/M_TDCLightPiece"


def scalar_param(mat, name, value, x, y):
    node = mel.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, x, y)
    node.set_editor_property("parameter_name", name)
    node.set_editor_property("default_value", value)
    return node


def build_light_piece():
    """M_TDCLightPiece (T-0026): piezas que dependen de la luz, hechas "de luz". Translúcido unlit:
    Emissive = Color * gelatina * (Glow + borde Fresnel * EdgeGlow) * titileo; Opacity = Opacity * (1 + borde).
    El C++ baja Opacity/Glow cuando la pieza está ausente: queda un fantasma tenue sin colisión."""
    require_writable(LIGHT_PATH, ".uasset")
    if eal.does_asset_exist(LIGHT_PATH):
        mat = eal.load_asset(LIGHT_PATH)
        mel.delete_all_material_expressions(mat)
    else:
        mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset("M_TDCLightPiece", FOLDER, unreal.Material,
                                                                       unreal.MaterialFactoryNew())
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)

    color = vector_param(mat, "Color", (1.0, 0.8, 0.15, 1.0), 0)
    glow = scalar_param(mat, "Glow", 1.4, -500, 100)
    edge_glow = scalar_param(mat, "EdgeGlow", 2.5, -700, 350)
    opacity = scalar_param(mat, "Opacity", 0.45, -500, 600)
    pulse = scalar_param(mat, "Pulse", 0.08, -900, 800)

    fres = mel.create_material_expression(mat, unreal.MaterialExpressionFresnel, -900, 300)
    fres.set_editor_property("exponent", 2.0)
    fres.set_editor_property("base_reflect_fraction", 0.0)

    # gelatina world-aligned (si existe): da textura de difusor al volumen de luz
    gel_path = f"{FOLDER}/Textures/T_Gel_Panel"
    panel = None
    if eal.does_asset_exist(gel_path):
        tile = scalar_param(mat, "TileSize", 200.0, -1100, -450)
        uv = world_aligned_uv(mat, tile)
        tex = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -600, -300)
        tex.set_editor_property("parameter_name", "Pattern")
        tex.set_editor_property("texture", eal.load_asset(gel_path))
        mel.connect_material_expressions(uv, "", tex, "UVs")
        one = constant(mat, 1.0, -200)
        panel = mel.create_material_expression(mat, unreal.MaterialExpressionLinearInterpolate, -400, -250)
        mel.connect_material_expressions(one, "", panel, "A")
        mel.connect_material_expressions(tex, "R", panel, "B")
        mel.connect_material_expressions(constant(mat, 0.5, -120), "", panel, "Alpha")

    # titileo de lámpara: 1 + Pulse * sin(Time * 3)
    time = mel.create_material_expression(mat, unreal.MaterialExpressionTime, -1100, 800)
    speed = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -950, 800)
    speed.set_editor_property("const_b", 3.0)
    mel.connect_material_expressions(time, "", speed, "A")
    sine = mel.create_material_expression(mat, unreal.MaterialExpressionSine, -800, 800)
    mel.connect_material_expressions(speed, "", sine, "")
    amp = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -650, 800)
    mel.connect_material_expressions(sine, "", amp, "A")
    mel.connect_material_expressions(pulse, "", amp, "B")
    flicker = mel.create_material_expression(mat, unreal.MaterialExpressionAdd, -500, 800)
    flicker.set_editor_property("const_a", 1.0)
    mel.connect_material_expressions(amp, "", flicker, "B")

    edge = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -500, 300)
    mel.connect_material_expressions(fres, "", edge, "A")
    mel.connect_material_expressions(edge_glow, "", edge, "B")
    energy = mel.create_material_expression(mat, unreal.MaterialExpressionAdd, -300, 200)
    mel.connect_material_expressions(glow, "", energy, "A")
    mel.connect_material_expressions(edge, "", energy, "B")
    lit = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -150, 50)
    mel.connect_material_expressions(color, "", lit, "A")
    mel.connect_material_expressions(energy, "", lit, "B")
    if panel:
        lit2 = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, 0, 0)
        mel.connect_material_expressions(lit, "", lit2, "A")
        mel.connect_material_expressions(panel, "", lit2, "B")
        lit = lit2
    final = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, 150, 100)
    mel.connect_material_expressions(lit, "", final, "A")
    mel.connect_material_expressions(flicker, "", final, "B")
    eye_inverse = getattr(unreal, "MaterialExpressionEyeAdaptationInverse", None)
    if eye_inverse:  # brillo estable aunque la auto-exposure suba en la escena oscura (si no, se quema)
        inv = mel.create_material_expression(mat, eye_inverse, 300, 100)
        mel.connect_material_expressions(final, "", inv, "LightValue")
        final = inv
    mel.connect_material_property(final, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

    rim = mel.create_material_expression(mat, unreal.MaterialExpressionAdd, -300, 550)
    rim.set_editor_property("const_a", 1.0)
    mel.connect_material_expressions(fres, "", rim, "B")
    op = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -150, 600)
    mel.connect_material_expressions(opacity, "", op, "A")
    mel.connect_material_expressions(rim, "", op, "B")
    sat = mel.create_material_expression(mat, unreal.MaterialExpressionSaturate, 0, 600)
    mel.connect_material_expressions(op, "", sat, "")
    mel.connect_material_property(sat, "", unreal.MaterialProperty.MP_OPACITY)

    mel.recompile_material(mat)
    if not eal.save_asset(LIGHT_PATH, only_if_is_dirty=False):
        fail(f"No se pudo guardar {LIGHT_PATH} (git lfs lock Game/Content/Materials/M_TDCLightPiece.uasset)")
    unreal.log(f"[TDC] {LIGHT_PATH} creado y guardado.")


try:
    build()
    build_light_piece()
finally:
    # Lanzado con -ExecutePythonScript (tdc.ps1 python -InEditor): cerrar el editor al terminar, también si falló.
    if "-executepythonscript" in unreal.SystemLibrary.get_command_line().lower():
        unreal.SystemLibrary.quit_editor()
