"""
Ajusta PuppetScale de los extras de L_Slice (X1/X2/X3) sin recompilar.

Uso en Windows (PowerShell), con el lock de L_Slice tomado:
  $env:TDC_PUPPET_SCALE = "1"; .\\Tools\\scripts\\tdc.ps1 python -Script Tools\\unreal\\set_extra_scale.py
Criterio: el extra tiene que medir lo mismo que la cápsula (~180 cm), como el director.
Si se ven gigantes con 100 -> 1. Si se ven diminutos -> 10000.
"""

import os

import unreal

MAP_PATH = "/Game/Maps/L_Slice"


def fail(message):
    unreal.log_error(f"[TDC] ERROR: {message}")
    raise RuntimeError(message)


def main():
    raw = os.environ.get("TDC_PUPPET_SCALE")
    if not raw:
        fail("falta la variable TDC_PUPPET_SCALE (ej. 1 o 10000)")
    scale = float(raw)
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    les.load_level(MAP_PATH)
    extra_class = unreal.load_class(None, "/Script/TheDirectorsCut.TDCExtra")
    count = 0
    for actor in eas.get_all_level_actors():
        if isinstance(actor, extra_class):
            actor.set_editor_property("puppet_scale", scale)
            count += 1
    if not count:
        fail("no hay extras en L_Slice")
    if not les.save_current_level():
        fail("No se pudo guardar L_Slice (¿read-only por LFS? git lfs lock Game/Content/Maps/L_Slice.umap)")
    unreal.log(f"[TDC] puppet_scale={scale} en {count} extras; L_Slice guardado.")


try:
    main()
finally:
    if "-executepythonscript" in unreal.SystemLibrary.get_command_line().lower():
        unreal.SystemLibrary.quit_editor()
