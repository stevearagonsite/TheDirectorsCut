---
id: T-0055
titulo: Pulido (T-0053) + cono de luz (T-0054) + ajustes de fantasmas y muros (T-0055) en una corrida
estado: enviada
tipo: mixto
rama: t/T-0055-ajustes
creada: 2026-11-15
---

# T-0055 — Pulido, cono de luz y ajustes

## Objetivo
Una sola corrida sobre main con T-0052 (#44):
- **T-0053:** lluvia y vapor translúcidos (`opacity` en cards.json), los 9 VFX restantes pintados, HUD ~1.5x, director entero en las
  capturas de autorun, replay y bot.
- **T-0054:** cono de luz pintado (`M_TDCLightCone`, aditivo) en los reflectores encendidos, de la lente al plano de juego.
- **T-0055:** la pared W1 ausente se ve como contorno (opacity 0.07 y EdgeGlow 1.8), no como panel gris; muros de ladrillo de 480 cm con
  darken 0.5 (tapaban el skyline).

## Pasos
1. `git fetch` y `git checkout t/T-0055-ajustes` (verificá el sha). `git lfs pull`. `.\Tools\scripts\tdc.ps1 stop`.
2. `.\Tools\scripts\tdc.ps1 build`. Si no compila, reportá `fallo` con `archivo:línea`.
3. Locks: `git lfs lock Game/Content/Maps/L_Slice.umap` y
   `git ls-files Game/Content/Materials Game/Content/Dressing Game/Content/VFX Game/Content/Characters | ForEach-Object { git lfs lock $_ }`.
4. `make_materials.py` (con `M_TDCLightCone`) → `import_vfx.py` (15 sheets, sin `libpng error`) → `build_dressing.py` → `apply_toon.py`.
5. `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0055 -Name 01-escena` (cámara de juego). Describí: lluvia y vapor
   translúcidos; muros de ladrillo sin tapar el skyline; ¿se ve el adoquín?
6. `.\Tools\scripts\tdc.ps1 playtest -Task T-0055` → RESULT PASS. En las capturas `*_luces`: **cono de luz** con la parte estrecha arriba,
   ancho = lo que ilumina, sin lavar a los personajes (si sale cabeza abajo, `CONE_FLIP_V = True` en make_materials.py, repetí 4 y 6 y anotalo);
   la pared W1 ausente como contorno tenue; director entero; HUD legible.
7. `.\Tools\scripts\tdc.ps1 autorun -Task T-0055 -Wait 45`: 2 capturas con VFX pintados (claquetazo, derrota o aturdido).
8. Commit de `Game/Content/**` (LFS), evidencia y Reporte. `git lfs unlock`. Push y `[TDC-REPORT]`.
9. `.\Tools\scripts\tdc.ps1 play`.

## Criterios de aceptación
- [ ] Compila; scripts sin `[TDC] ERROR` ni `libpng error`.
- [ ] Playtest RESULT PASS; cono de luz bien orientado; W1 ausente como contorno.
- [ ] Lluvia/vapor translúcidos; director entero; HUD legible.

## Permite
- Editar `Tools/unreal/*.py` y `Tools/scripts/tdc.ps1` solo para compatibilidad de API, y `CONE_FLIP_V`.

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
