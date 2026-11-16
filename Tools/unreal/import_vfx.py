"""
VFX de sprite sheet (pixelfx) para The Director's Cut (T-0018). Correr en el editor:
  .\\Tools\\scripts\\tdc.ps1 python -Script Tools\\unreal\\import_vfx.py
Antes: git lfs lock de Game/Content/VFX/* si ya existen.

- Importa Art/Export/VFX/<nombre>.png como /Game/VFX/T_<nombre> (filtro Nearest, sin mips, UserInterface2D).
- Crea /Game/VFX/M_TDCFlipbook: unlit translúcido de dos caras; parámetros Sheet, Cols, Rows, Frames, FPS, Loop,
  StartTime e Intensity. ATDCFlipbookFx (C++) crea un MID por efecto y elige el frame con el tiempo del mundo.
Idempotente.
"""

import glob
import os

import unreal

DEST = "/Game/VFX"
REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
SRC = os.path.join(REPO, "Art", "Export", "VFX")
eal = unreal.EditorAssetLibrary
mel = unreal.MaterialEditingLibrary
tools = unreal.AssetToolsHelpers.get_asset_tools()


def log(msg):
    unreal.log("[TDC] " + msg)


def fail(message):
    unreal.log_error(f"[TDC] ERROR: {message}")
    raise RuntimeError(message)


def require_writable(asset_path):
    rel = asset_path.replace("/Game/", "", 1) + ".uasset"
    disk = os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_content_dir()), rel)
    if os.path.exists(disk) and not os.access(disk, os.W_OK):
        fail(f"Game/Content/{rel} es read-only (LFS sin lock). Antes: git lfs lock Game/Content/{rel}")


# Hojas pintadas (no pixel art, T-0048): filtro bilinear; con Nearest se veían escalonadas al agrandarse.
PAINTED = {"claquetazo_impact"}


def import_sheet(png):
    name = "T_" + os.path.splitext(os.path.basename(png))[0]
    require_writable(f"{DEST}/{name}")
    task = unreal.AssetImportTask()
    task.filename = png
    task.destination_path = DEST
    task.destination_name = name
    task.replace_existing = True
    task.automated = True
    task.save = False
    tools.import_asset_tasks([task])
    path = f"{DEST}/{name}"
    tex = eal.load_asset(path) if eal.does_asset_exist(path) else None
    if not tex:
        fail(f"no se importó {name}")
    painted = True  # T-0053: todos los VFX pasaron a estilo pintado (PAINTED queda como referencia)
    tex.set_editor_property("filter", unreal.TextureFilter.TF_BILINEAR if painted else unreal.TextureFilter.TF_NEAREST)
    tex.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    tex.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
    eal.save_asset(path)
    return tex


def scalar(mat, name, default, x, y):
    e = mel.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, x, y)
    e.set_editor_property("parameter_name", name)
    e.set_editor_property("default_value", default)
    return e


def binop(mat, cls, a, b, x, y, a_out="", b_out=""):
    e = mel.create_material_expression(mat, cls, x, y)
    mel.connect_material_expressions(a, a_out, e, "A")
    mel.connect_material_expressions(b, b_out, e, "B")
    return e


def flipbook_material(any_tex):
    path = f"{DEST}/M_TDCFlipbook"
    require_writable(path)
    mat = eal.load_asset(path) if eal.does_asset_exist(path) else None
    if not mat:
        mat = tools.create_asset("M_TDCFlipbook", DEST, unreal.Material, unreal.MaterialFactoryNew())
    mel.delete_all_material_expressions(mat)
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    mat.set_editor_property("two_sided", True)

    cols = scalar(mat, "Cols", 4.0, -1400, 0)
    rows = scalar(mat, "Rows", 4.0, -1400, 100)
    frames = scalar(mat, "Frames", 16.0, -1400, 200)
    fps = scalar(mat, "FPS", 30.0, -1400, 300)
    loop = scalar(mat, "Loop", 0.0, -1400, 400)
    start = scalar(mat, "StartTime", 0.0, -1400, 500)
    intensity = scalar(mat, "Intensity", 1.5, -400, 300)
    opacity = scalar(mat, "Opacity", 1.0, 120, 600)  # T-0053: las cards de lluvia/vapor pintadas eran demasiado opacas
    one = mel.create_material_expression(mat, unreal.MaterialExpressionConstant, -1400, 600)
    one.set_editor_property("r", 1.0)

    time = mel.create_material_expression(mat, unreal.MaterialExpressionTime, -1400, 700)
    elapsed = binop(mat, unreal.MaterialExpressionSubtract, time, start, -1200, 600)
    framef = binop(mat, unreal.MaterialExpressionMultiply, elapsed, fps, -1050, 600)
    last = binop(mat, unreal.MaterialExpressionSubtract, frames, one, -1050, 450)
    once = binop(mat, unreal.MaterialExpressionMin, framef, last, -900, 500)
    looped = binop(mat, unreal.MaterialExpressionFmod, framef, frames, -900, 650)
    pick = mel.create_material_expression(mat, unreal.MaterialExpressionLinearInterpolate, -750, 550)
    mel.connect_material_expressions(once, "", pick, "A")
    mel.connect_material_expressions(looped, "", pick, "B")
    mel.connect_material_expressions(loop, "", pick, "Alpha")
    frame = mel.create_material_expression(mat, unreal.MaterialExpressionFloor, -620, 550)
    mel.connect_material_expressions(pick, "", frame, "")

    col = binop(mat, unreal.MaterialExpressionFmod, frame, cols, -500, 450)
    rowf = binop(mat, unreal.MaterialExpressionDivide, frame, cols, -500, 600)
    row = mel.create_material_expression(mat, unreal.MaterialExpressionFloor, -380, 600)
    mel.connect_material_expressions(rowf, "", row, "")
    offset = binop(mat, unreal.MaterialExpressionAppendVector, col, row, -260, 500)
    uv = mel.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -260, 350)
    added = binop(mat, unreal.MaterialExpressionAdd, uv, offset, -140, 400)
    grid = binop(mat, unreal.MaterialExpressionAppendVector, cols, rows, -140, 550)
    final_uv = binop(mat, unreal.MaterialExpressionDivide, added, grid, -20, 450)

    sheet = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, 120, 300)
    sheet.set_editor_property("parameter_name", "Sheet")
    sheet.set_editor_property("texture", any_tex)
    sheet.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_COLOR)
    mel.connect_material_expressions(final_uv, "", sheet, "UVs")
    emissive = binop(mat, unreal.MaterialExpressionMultiply, sheet, intensity, 320, 250, a_out="RGB")
    mel.connect_material_property(emissive, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    alpha = binop(mat, unreal.MaterialExpressionMultiply, sheet, opacity, 320, 450, a_out="A")
    mel.connect_material_property(alpha, "", unreal.MaterialProperty.MP_OPACITY)
    mel.recompile_material(mat)
    eal.save_asset(path)
    log("M_TDCFlipbook listo")


def main():
    pngs = sorted(glob.glob(os.path.join(SRC, "*.png")))
    if not pngs:
        fail("no hay sheets en Art/Export/VFX")
    texs = [import_sheet(p) for p in pngs]
    log(f"{len(texs)} sheets importadas: " + ", ".join(t.get_name() for t in texs))
    flipbook_material(texs[0])


main()
