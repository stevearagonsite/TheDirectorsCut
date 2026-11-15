"""
Diagnóstico (T-0031): ¿las animaciones de los extras y del director tienen movimiento real?
  .\\Tools\\scripts\\tdc.ps1 python -Script Tools\\unreal\\check_anims.py
Imprime por clip: duración, frames, esqueleto (¿coincide con el del SK?) y cuánto se mueve un hueso entre t=0 y t=mitad.
"Movimiento 0" = clip quieto (Rodin lo exportó sin keys o el import las perdió) -> el extra se desliza en pose fija.
"""

import unreal

eal = unreal.EditorAssetLibrary
SETS = {
    "Director": ("/Game/Characters/Director/SK_Director", ["AS_Director_Idle", "AS_Director_Walk", "AS_Director_Run"]),
    "Comun": ("/Game/Characters/Extras/Comun/SK_Extra_Comun", ["AS_Extra_Comun_Idle", "AS_Extra_Comun_Walk"]),
    "Resistente": ("/Game/Characters/Extras/Resistente/SK_Extra_Resistente",
                   ["AS_Extra_Resistente_Idle", "AS_Extra_Resistente_Walk"]),
    "Sombra": ("/Game/Characters/Extras/Sombra/SK_Extra_Sombra", ["AS_Extra_Sombra_Idle", "AS_Extra_Sombra_Walk"]),
}


def motion(anim, bones):
    best = 0.0
    length = anim.get_play_length()
    for bone in bones:
        try:
            a = unreal.AnimationLibrary.get_bone_pose_for_time(anim, bone, 0.0, False)
            b = unreal.AnimationLibrary.get_bone_pose_for_time(anim, bone, length * 0.5, False)
        except Exception:
            continue
        dq = a.rotation.get_angular_distance(b.rotation) if hasattr(a.rotation, "get_angular_distance") else 0.0
        dt = (a.translation - b.translation).length()
        best = max(best, dt + dq * 57.3)
    return best


for name, (sk_path, clips) in SETS.items():
    sk = eal.load_asset(sk_path) if eal.does_asset_exist(sk_path) else None
    if not sk:
        unreal.log(f"[TDC] {name}: sin {sk_path}")
        continue
    folder = sk_path.rsplit("/", 1)[0]
    for clip in clips:
        path = f"{folder}/{clip}"
        anim = eal.load_asset(path) if eal.does_asset_exist(path) else None
        if not anim:
            unreal.log(f"[TDC] {name}/{clip}: NO EXISTE")
            continue
        bones = list(unreal.AnimationLibrary.get_animation_track_names(anim))[:12]
        same = anim.get_editor_property("skeleton") == sk.get_editor_property("skeleton")
        unreal.log(f"[TDC] {name}/{clip}: {anim.get_play_length():.2f}s, frames={unreal.AnimationLibrary.get_num_frames(anim)}, "
                   f"pistas={len(bones)}, mismo esqueleto={same}, movimiento={motion(anim, bones):.2f}")
