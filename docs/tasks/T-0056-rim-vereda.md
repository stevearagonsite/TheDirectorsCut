---
id: T-0056
titulo: Rim cálido del director, vereda de adoquín en cuadro, cono más tenue y diagnóstico del bloque gris
estado: enviada
tipo: mixto
rama: t/T-0056-rim-vereda
creada: 2026-11-16
---

# T-0056 — Rim y vereda

## Objetivo
Feedback de T-0052/T-0055:
- `MI_Toon_Director`: RimColor ámbar (1, .6, .28), RimStrength 1.4, RimPower 2. El director tiene que ser lo más cálido del plano.
- Vereda de adoquín: `FOREGROUND_Y` (210, 950), arranca en el frente de los pisos para entrar en cuadro con la cámara de juego.
- `ConeIntensity` 1.8 → 1.0 (C++): el haz tapaba el skyline.
- `list_pieces.py`: diagnóstico del bloque gris liso que aparece a la derecha de `01-escena`.

## Pasos
1. `git fetch` y `git checkout t/T-0056-rim-vereda` (verificá el sha). `git lfs pull`. `.\Tools\scripts\tdc.ps1 stop`.
2. `.\Tools\scripts\tdc.ps1 build`. Si no compila, reportá `fallo` con `archivo:línea`.
3. Locks: `git lfs lock Game/Content/Maps/L_Slice.umap` y
   `git ls-files Game/Content/Materials Game/Content/Dressing Game/Content/Characters | ForEach-Object { git lfs lock $_ }`.
4. `apply_toon.py` → `build_dressing.py`.
5. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\list_pieces.py` y **pegá las primeras ~15 líneas** en el Reporte.
6. `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0056 -Name 01-escena` (cámara de juego). Describí: ¿el director tiene borde
   anaranjado y es lo más cálido del plano? ¿se ve la **vereda de adoquín** abajo? ¿sigue el bloque gris a la derecha?
7. `.\Tools\scripts\tdc.ps1 playtest -Task T-0056` → RESULT PASS. En `*_luces`: ¿el cono ya no tapa el skyline?
8. Commit de `Game/Content/**` (LFS), evidencia y Reporte. `git lfs unlock`. Push y `[TDC-REPORT]`.
9. `.\Tools\scripts\tdc.ps1 play`.

## Criterios de aceptación
- [ ] Compila; scripts sin `[TDC] ERROR`; salida de `list_pieces.py` en el Reporte.
- [ ] Rim anaranjado visible; vereda en cuadro; playtest RESULT PASS.

## Permite
- Editar `Tools/unreal/*.py` solo para compatibilidad de API.

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
