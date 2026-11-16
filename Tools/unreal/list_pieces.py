"""
Diagnóstico (T-0056): lista los actores de L_Slice cuyo bounds toca un rango de X, para identificar bloques que se
ven en las capturas sin saber qué son.
  .\\Tools\\scripts\\tdc.ps1 python -Script Tools\\unreal\\list_pieces.py
Rango por defecto X 1300–2600 (lado derecho de la captura de escena). Para otro rango, editar X_MIN/X_MAX abajo.
Loguea: label, clase, presence/record (SetPiece), color_override, ubicación y tamaño del bounds.
"""

import unreal

X_MIN, X_MAX = 1300.0, 2600.0
MAP = "/Game/Maps/L_Slice"

eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(MAP)

rows = []
for actor in eas.get_all_level_actors():
    origin, extent = actor.get_actor_bounds(False)
    x0, x1 = origin.x - extent.x, origin.x + extent.x
    if x1 < X_MIN or x0 > X_MAX:
        continue
    size = (extent.x * 2, extent.y * 2, extent.z * 2)
    extra = ""
    for prop in ("presence", "record_time", "color_override", "kit_trim"):
        try:
            extra += f" {prop}={actor.get_editor_property(prop)}"
        except Exception:
            pass
    rows.append((size[0] * size[2], actor.get_actor_label(), actor.get_class().get_name(), origin, size, extra))

rows.sort(key=lambda r: -r[0])
for _, label, cls, origin, size, extra in rows:
    unreal.log(f"[TDC] {label} ({cls}) centro=({origin.x:.0f},{origin.y:.0f},{origin.z:.0f}) "
               f"tamaño=({size[0]:.0f},{size[1]:.0f},{size[2]:.0f}){extra}")
unreal.log(f"[TDC] {len(rows)} actores en X {X_MIN:.0f}-{X_MAX:.0f}")
