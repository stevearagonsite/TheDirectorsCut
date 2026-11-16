"""
Texturas de interfaz (T-0035): menú inicial, carga e íconos del HUD. Correr en el editor:
  .\\Tools\\scripts\\tdc.ps1 python -Script Tools\\unreal\\import_ui.py
Antes, si ya existen: git lfs lock de Game/Content/UI/** (ver docs/tasks/T-0035-menu.md).

Art/Export/UI/*.png      -> /Game/UI/T_UI_<nombre>
Art/Export/UI/HUD/*.png  -> /Game/UI/HUD/T_UI_<nombre>   (sin hud_preview, que es una maqueta)
Ajustes de UI: compresión UserInterface2D, sin mips, grupo UI, sin streaming. /Game/UI está en DirectoriesToAlwaysCook.
ATDCPlayerController las carga por ruta; si falta alguna, el menú se arma igual sobre colores lisos.
"""

import glob
import os

import unreal

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
SRC = os.path.join(REPO, "Art", "Export", "UI")
SKIP = {"hud_preview", "icon_256"}  # maqueta y fuente del .ico: no se usan en el juego
eal = unreal.EditorAssetLibrary


def log(msg):
    unreal.log("[TDC] " + msg)


def fail(message):
    unreal.log_error(f"[TDC] ERROR: {message}")
    raise RuntimeError(message)


def require_writable(asset_path):
    rel = asset_path.replace("/Game/", "", 1) + ".uasset"
    disk = os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_content_dir()), rel)
    if os.path.exists(disk) and not os.access(disk, os.W_OK):
        fail(f"Game/Content/{rel} es read-only (lockable de LFS sin lock). Antes: git lfs lock Game/Content/{rel}")


def import_png(png, dest):
    stem = os.path.splitext(os.path.basename(png))[0]
    name = f"T_UI_{stem}"
    path = f"{dest}/{name}"
    require_writable(path)
    task = unreal.AssetImportTask()
    task.filename = png
    task.destination_path = dest
    task.destination_name = name
    task.replace_existing = True
    task.automated = True
    task.save = False
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    tex = eal.load_asset(path) if eal.does_asset_exist(path) else None
    if not tex:
        fail(f"no se importó {png}")
    tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    tex.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    tex.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
    tex.set_editor_property("never_stream", True)
    tex.set_editor_property("srgb", True)
    if not eal.save_asset(path, only_if_is_dirty=False):
        fail(f"no se pudo guardar {path} (¿lock de LFS?)")
    return path


def main():
    done = []
    for folder, dest in ((SRC, "/Game/UI"), (os.path.join(SRC, "HUD"), "/Game/UI/HUD")):
        for png in sorted(glob.glob(os.path.join(folder, "*.png"))):
            if os.path.splitext(os.path.basename(png))[0] in SKIP:
                continue
            done.append(import_png(png, dest))
    if not done:
        fail("no hay PNG en Art/Export/UI")
    for path in done:
        log(f"UI: {path}")
    log(f"texturas de UI importadas: {len(done)}")


try:
    main()
finally:
    if "-executepythonscript" in unreal.SystemLibrary.get_command_line().lower():
        unreal.SystemLibrary.quit_editor()
