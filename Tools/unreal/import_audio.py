"""
Audio de The Director's Cut (T-0022). Correr en el editor:
  .\\Tools\\scripts\\tdc.ps1 python -Script Tools\\unreal\\import_audio.py
Antes: git lfs lock de Game/Content/Audio/* si ya existen.

Importa Art/Export/Audio/*.wav como /Game/Audio/<nombre> (SoundWave). Los que terminan en _loop o empiezan con amb_
quedan con looping. El C++ (TDC::PlaySfx*) los carga por nombre; si falta uno, ese sonido no suena y nada se rompe.
Créditos y licencias: Art/Export/Audio/creditos.md (todo CC0 o generado con ElevenLabs).
"""

import glob
import os

import unreal

DEST = "/Game/Audio"
REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
SRC = os.path.join(REPO, "Art", "Export", "Audio")
eal = unreal.EditorAssetLibrary


def fail(message):
    unreal.log_error(f"[TDC] ERROR: {message}")
    raise RuntimeError(message)


def main():
    wavs = sorted(glob.glob(os.path.join(SRC, "*.wav")))
    if not wavs:
        fail("no hay WAV en Art/Export/Audio")
    tasks = []
    for wav in wavs:
        name = os.path.splitext(os.path.basename(wav))[0]
        disk = os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_content_dir()), "Audio", name + ".uasset")
        if os.path.exists(disk) and not os.access(disk, os.W_OK):
            fail(f"Game/Content/Audio/{name}.uasset es read-only. Antes: git lfs lock Game/Content/Audio/{name}.uasset")
        t = unreal.AssetImportTask()
        t.filename = wav
        t.destination_path = DEST
        t.destination_name = name
        t.replace_existing = True
        t.automated = True
        t.save = False
        tasks.append(t)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
    done = 0
    for wav in wavs:
        name = os.path.splitext(os.path.basename(wav))[0]
        path = f"{DEST}/{name}"
        snd = eal.load_asset(path) if eal.does_asset_exist(path) else None
        if not snd:
            unreal.log_warning(f"[TDC] no se importó {name}")
            continue
        if name.endswith("_loop") or name.startswith("amb_"):
            snd.set_editor_property("looping", True)
        eal.save_asset(path, only_if_is_dirty=False)
        done += 1
    unreal.log(f"[TDC] audio importado: {done}/{len(wavs)}")


main()
