---
id: T-0053
titulo: Pulido post-integración: VFX pintados completos, lluvia/vapor translúcidos, HUD legible, encuadre de capturas
estado: enviada
tipo: mixto
rama: t/T-0053-pulido
creada: 2026-11-15
---

# T-0053 — Pulido

## Objetivo
Lo que marcó la PC en T-0049, sobre T-0052:
- Lluvia y vapor translúcidos: `M_TDCFlipbook` tiene `Opacity`, y cada card la lee de `cards.json` (lluvia 0.3, vapor 0.55).
- Los 9 VFX que faltaban (derrota, aturdido, rebote, rebobinado, light_toggle y 4 transiciones de pieza) pasan a pintados, en bilinear.
- HUD ~1.5x (ilegible a 720p).
- En autorun, replay y bot el director ya no sale cortado abajo (SocketOffset Z 100–110, brazo 1100).

## Pasos
1. `git fetch` y `git checkout t/T-0053-pulido` (verificá el sha). `git lfs pull`. `.\Tools\scripts\tdc.ps1 stop`.
2. `.\Tools\scripts\tdc.ps1 build`. Si no compila, reportá `fallo` con `archivo:línea`.
3. Locks: `git lfs lock Game/Content/Maps/L_Slice.umap` y
   `git ls-files Game/Content/Materials Game/Content/Dressing Game/Content/VFX Game/Content/Characters | ForEach-Object { git lfs lock $_ }`.
4. `make_materials.py` → `import_vfx.py` (15 sheets, sin `libpng error`) → `build_dressing.py` → `apply_toon.py`.
5. `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0053 -Name 01-camara-juego` (con `-TDCNoMenu`: **cámara de juego**, brazo 1900).
   Describí: ¿se ve la **calle de adoquín** abajo? ¿la lluvia y el vapor son translúcidos? ¿el director entra entero?
6. `.\Tools\scripts\tdc.ps1 playtest -Task T-0053` → RESULT PASS. En sus capturas: ¿el director entra entero? ¿el HUD se lee?
7. `.\Tools\scripts\tdc.ps1 autorun -Task T-0053 -Wait 45`: elegí 2 capturas con VFX pintados (claquetazo, derrota o aturdido).
8. Commit de `Game/Content/**` (LFS), evidencia y Reporte. `git lfs unlock`. Push y `[TDC-REPORT]`.
9. Dejá abierto el juego: `.\Tools\scripts\tdc.ps1 play`.

## Criterios de aceptación
- [ ] Compila; scripts sin `[TDC] ERROR` ni `libpng error`.
- [ ] `01-camara-juego.png` con cámara de juego; adoquín visible (o reportar que no se ve con esa cámara).
- [ ] Playtest RESULT PASS; capturas con el director entero y HUD legible.

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
