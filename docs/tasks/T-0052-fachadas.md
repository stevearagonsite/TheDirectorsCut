---
id: T-0052
titulo: Kit de plataformas, director legible y fachadas 2D (T-0050 + T-0051 + T-0052)
estado: enviada
tipo: mixto
rama: t/T-0052-fachadas
creada: 2026-11-15
---

# T-0052 — Kit, director legible y fachadas

## Objetivo
Sobre main con T-0049 (integración):
- **T-0050:** cornisa del kit (`/Game/Dressing/Kit/SM_Ledge_Mid`) en el borde frontal de las piezas fijas (solo visual).
  `build_dressing` mide la escala y la rotación del FBX y hacia dónde mira la cornisa (`kit_front_yaw`).
- **T-0051:** rim cálido del director y cámara a 1900 (el director medía 11 % del alto de pantalla).
- **T-0052:** fachadas 2D en `cards.json`: cine art déco detrás de la marquesina (que sube a z=265) y pared de ladrillo detrás de cada afiche.
- `tdc.ps1 shot` ahora pasa `-TDCNoMenu`, así la captura sale con la cámara de juego.

## Pasos
1. `git fetch` y `git checkout t/T-0052-fachadas` (verificá el sha). `git lfs pull`. `.\Tools\scripts\tdc.ps1 stop`.
2. `.\Tools\scripts\tdc.ps1 build`. Si no compila, reportá `fallo` con `archivo:línea`.
3. Locks: `git lfs lock Game/Content/Maps/L_Slice.umap` y
   `git ls-files Game/Content/Materials Game/Content/Dressing Game/Content/Characters | ForEach-Object { git lfs lock $_ }`.
4. `make_materials.py` → `build_dressing.py` (pegá las líneas `kit SM_Ledge_Mid: …` y `cornisa: voladizo … -> KitTrimYaw …`) → `apply_toon.py`.
5. `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0052 -Name 01-escena`. Describí: ¿la cornisa mira a la cámara y queda al ras
   de la tapa? ¿se ven las fachadas (cine detrás de la marquesina, ladrillo detrás de los afiches)? ¿el director se distingue mejor (rim cálido)?
   ¿la calle de adoquín se ve abajo?
6. `.\Tools\scripts\tdc.ps1 playtest -Task T-0052` → **RESULT PASS** (no cambia ninguna colisión). Pegá el resultado.
7. Commit de `Game/Content/**` (LFS), evidencia y Reporte. `git lfs unlock`. Push y `[TDC-REPORT]`.
8. Dejá abierto el editor en juego con `.\Tools\scripts\tdc.ps1 play` para que Steve lo vea.

## Criterios de aceptación
- [ ] Compila; scripts sin `[TDC] ERROR`.
- [ ] `01-escena.png` con cámara de juego, cornisas al frente y fachadas visibles.
- [ ] Playtest RESULT PASS.

## Permite
- Editar `Tools/unreal/*.py` y `Tools/scripts/tdc.ps1` solo para compatibilidad de API.

## No hacer
- No cambiar C++. No empaquetar. No commitear `Tools/unreal/_diag_*`.

---

## Reporte (lo completa Windows)
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
