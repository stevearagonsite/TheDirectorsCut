"""
Cel shading de los personajes (T-0023). Correr DESPUÉS de import_director/fix_director_look e import_extras:
  .\\Tools\\scripts\\tdc.ps1 python -Script Tools\\unreal\\apply_toon.py   (si el material no compila: -InEditor)
Locks: Game/Content/Materials/M_TDCToon.uasset, Game/Content/Characters/** (SK_* y MI_Toon_*).

M_TDCToon (Unlit): la escena noir es oscura y los personajes se perdían. El toon no depende de las luces del nivel:
  - luz clave fija (LightDir) en 3 bandas duras (Bands) con la sombra teñida azul (ShadowColor), no negra;
  - rim light frío (RimColor/RimPower/RimStrength) que recorta la silueta contra el fondo;
  - Brightness global. Nada de amarillo/violeta/cian: esos colores son de las mecánicas.
Un MI_Toon_<nombre> por personaje con su textura difusa de Rodin. Las de los extras se importan de
Art/Export/Extras/<Tipo>/texture_diffuse.png si el FBX no las trajo.
Si algo falla con un personaje, queda su material anterior (M_Director / M_Extra_*). Idempotente.
"""

import os

import unreal

eal = unreal.EditorAssetLibrary
mel = unreal.MaterialEditingLibrary
tools = unreal.AssetToolsHelpers.get_asset_tools()
REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
MASTER = "/Game/Materials/M_TDCToon"

# nombre, carpeta, SK, tinte, textura en disco (fallback)
CHARACTERS = [
    ("Director", "/Game/Characters/Director", "SK_Director", (1.0, 1.0, 1.0), None),
    ("Extra_Comun", "/Game/Characters/Extras/Comun", "SK_Extra_Comun", (0.9, 0.9, 0.9), "Comun"),
    ("Extra_Resistente", "/Game/Characters/Extras/Resistente", "SK_Extra_Resistente", (0.9, 0.9, 0.9), "Resistente"),
    ("Extra_Sombra", "/Game/Characters/Extras/Sombra", "SK_Extra_Sombra", (0.75, 0.75, 0.8), "Sombra"),
]


def log(msg):
    unreal.log("[TDC] " + msg)


def fail(message):
    unreal.log_error(f"[TDC] ERROR: {message}")
    raise RuntimeError(message)


def expr(mat, cls, x, y, **props):
    node = mel.create_material_expression(mat, cls, x, y)
    for k, v in props.items():
        node.set_editor_property(k, v)
    return node


def scalar(mat, name, value, x, y):
    return expr(mat, unreal.MaterialExpressionScalarParameter, x, y, parameter_name=name, default_value=value)


def vector(mat, name, rgb, x, y):
    return expr(mat, unreal.MaterialExpressionVectorParameter, x, y, parameter_name=name,
                default_value=unreal.LinearColor(rgb[0], rgb[1], rgb[2], 1.0))


def link(a, b, pin_b="", pin_a=""):
    mel.connect_material_expressions(a, pin_a, b, pin_b)


def build_master(default_tex):
    if eal.does_asset_exist(MASTER):
        mat = eal.load_asset(MASTER)
        mel.delete_all_material_expressions(mat)
    else:
        mat = tools.create_asset("M_TDCToon", "/Game/Materials", unreal.Material, unreal.MaterialFactoryNew())
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    mat.set_editor_property("used_with_skeletal_mesh", True)

    tex = expr(mat, unreal.MaterialExpressionTextureSampleParameter2D, -1400, -300, parameter_name="BaseTex",
               texture=default_tex)
    tint = vector(mat, "Tint", (1, 1, 1), -1400, -100)
    albedo = expr(mat, unreal.MaterialExpressionMultiply, -1150, -250)
    link(tex, albedo, "A", "RGB")
    link(tint, albedo, "B")

    # Half-lambert con luz fija -> bandas duras. Cámara en +Y mirando -Y: luz desde la cámara, arriba a la izquierda.
    ldir = vector(mat, "LightDir", (-0.35, 0.6, 0.7), -1700, 150)
    ln = expr(mat, unreal.MaterialExpressionNormalize, -1500, 150)
    link(ldir, ln)
    nrm = expr(mat, unreal.MaterialExpressionVertexNormalWS, -1500, 50)
    ndl = expr(mat, unreal.MaterialExpressionDotProduct, -1300, 100)
    link(nrm, ndl, "A")
    link(ln, ndl, "B")
    half = expr(mat, unreal.MaterialExpressionMultiply, -1150, 100, const_b=0.5)
    link(ndl, half, "A")
    half2 = expr(mat, unreal.MaterialExpressionAdd, -1000, 100, const_b=0.5)
    link(half, half2, "A")
    bands = scalar(mat, "Bands", 3.0, -1000, 250)
    scaled = expr(mat, unreal.MaterialExpressionMultiply, -850, 120)
    link(half2, scaled, "A")
    link(bands, scaled, "B")
    stepped = expr(mat, unreal.MaterialExpressionFloor, -700, 120)
    link(scaled, stepped)
    bands_m1 = expr(mat, unreal.MaterialExpressionSubtract, -850, 250, const_b=1.0)
    link(bands, bands_m1, "A")
    norm = expr(mat, unreal.MaterialExpressionDivide, -550, 150)
    link(stepped, norm, "A")
    link(bands_m1, norm, "B")
    sat = expr(mat, unreal.MaterialExpressionSaturate, -420, 150)
    link(norm, sat)
    shadow = vector(mat, "ShadowColor", (0.42, 0.48, 0.68), -550, 300)
    white = expr(mat, unreal.MaterialExpressionConstant3Vector, -550, 400, constant=unreal.LinearColor(1, 1, 1, 1))
    light = expr(mat, unreal.MaterialExpressionLinearInterpolate, -300, 250)
    link(shadow, light, "A")
    link(white, light, "B")
    link(sat, light, "Alpha")

    shaded = expr(mat, unreal.MaterialExpressionMultiply, -150, -100)
    link(albedo, shaded, "A")
    link(light, shaded, "B")
    bright = scalar(mat, "Brightness", 1.15, -150, 50)
    lit = expr(mat, unreal.MaterialExpressionMultiply, 0, -50)
    link(shaded, lit, "A")
    link(bright, lit, "B")

    # Rim light frío (luz de luna) para despegar la silueta del fondo oscuro.
    fres = expr(mat, unreal.MaterialExpressionFresnel, -400, 500, base_reflect_fraction=0.0)
    rim_pow = scalar(mat, "RimPower", 3.0, -600, 550)
    link(rim_pow, fres, "ExponentIn")
    rim_str = scalar(mat, "RimStrength", 0.8, -400, 650)
    rim_col = vector(mat, "RimColor", (0.65, 0.78, 1.0), -400, 750)
    rim_a = expr(mat, unreal.MaterialExpressionMultiply, -200, 550)
    link(fres, rim_a, "A")
    link(rim_str, rim_a, "B")
    rim = expr(mat, unreal.MaterialExpressionMultiply, -50, 600)
    link(rim_a, rim, "A")
    link(rim_col, rim, "B")

    out = expr(mat, unreal.MaterialExpressionAdd, 150, 100)
    link(lit, out, "A")
    link(rim, out, "B")
    mel.connect_material_property(out, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

    mel.recompile_material(mat)
    if not eal.save_asset(MASTER, only_if_is_dirty=False):
        fail("no se pudo guardar M_TDCToon (¿lock de LFS?)")
    log("M_TDCToon creado")
    return mat


def find_diffuse(folder):
    for path in eal.list_assets(folder, recursive=True, include_folder=False):
        asset = eal.load_asset(path)
        if isinstance(asset, unreal.Texture2D):
            low = asset.get_name().lower()
            if "diffuse" in low or "basecolor" in low or "albedo" in low:
                return asset
    return None


def import_texture(png, folder, name):
    if not os.path.isfile(png):
        return None
    task = unreal.AssetImportTask()
    task.filename = png
    task.destination_path = folder
    task.destination_name = name
    task.replace_existing = True
    task.automated = True
    task.save = True
    tools.import_asset_tasks([task])
    path = f"{folder}/{name}"
    return eal.load_asset(path) if eal.does_asset_exist(path) else None


def texture_for(folder, kind):
    tex = find_diffuse(folder)
    if not tex and kind:
        png = os.path.join(REPO, "Art", "Export", "Extras", kind, "texture_diffuse.png")
        tex = import_texture(png, folder, f"T_Extra_{kind}_Diffuse")
    return tex


def apply(master, name, folder, sk_name, tint, kind):
    sk_path = f"{folder}/{sk_name}"
    if not eal.does_asset_exist(sk_path):
        log(f"sin {sk_path}: salteado")
        return False
    tex = texture_for(folder, kind)
    if not tex:
        unreal.log_warning(f"[TDC] {name}: sin textura difusa, queda el material anterior")
        return False
    mi_path = f"{folder}/MI_Toon_{name}"
    if eal.does_asset_exist(mi_path):
        mi = eal.load_asset(mi_path)
    else:
        mi = tools.create_asset(f"MI_Toon_{name}", folder, unreal.MaterialInstanceConstant,
                                unreal.MaterialInstanceConstantFactoryNew())
    mel.set_material_instance_parent(mi, master)
    mel.set_material_instance_texture_parameter_value(mi, "BaseTex", tex)
    mel.set_material_instance_vector_parameter_value(mi, "Tint", unreal.LinearColor(tint[0], tint[1], tint[2], 1.0))
    eal.save_asset(mi_path, only_if_is_dirty=False)

    sk = eal.load_asset(sk_path)
    sk.modify()
    slots = sk.get_editor_property("materials")
    # UE 5.8: iterar el array devuelve copias del struct; hay que reasignar por índice o el cambio se pierde.
    for i in range(len(slots)):
        slot = slots[i]
        slot.set_editor_property("material_interface", mi)
        slots[i] = slot
    sk.set_editor_property("materials", slots)
    eal.save_asset(sk_path, only_if_is_dirty=False)
    log(f"{name}: MI_Toon asignado a {len(slots)} slot(s) con {tex.get_name()}")
    return True


def main():
    default_tex = None
    for _, folder, _, _, kind in CHARACTERS:
        if eal.does_directory_exist(folder):  # does_asset_exist devuelve False para carpetas
            default_tex = texture_for(folder, kind)
            if default_tex:
                break
    if not default_tex:
        fail("no hay ninguna textura de personaje: correr import_director/import_extras antes")
    master = build_master(default_tex)
    done = sum(1 for c in CHARACTERS if apply(master, *c))
    log(f"toon aplicado a {done}/{len(CHARACTERS)} personajes")


try:
    main()
finally:
    if "-executepythonscript" in unreal.SystemLibrary.get_command_line().lower():
        unreal.SystemLibrary.quit_editor()
