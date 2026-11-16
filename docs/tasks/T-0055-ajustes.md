---
id: T-0055
titulo: Pulido (T-0053) + cono de luz (T-0054) + ajustes de fantasmas y muros (T-0055) en una corrida
estado: ok
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
- estado: ok
- sha: el commit "T-0055: ..." de esta rama (va en el [TDC-REPORT])
- resumen: compila (Result: Succeeded, 4 acciones). Los 4 scripts corrieron por commandlet sin `[TDC] ERROR` ni `libpng error`
  (`make_materials.py` importó `T_LightCone` y creó `M_TDCLightCone`; `build_dressing.py`: parallax de 3 capas, 20 cards, L_Slice
  guardado; `apply_toon.py` ok). Playtest `RESULT PASS 20/20` (meta en 16.7 s). Autorun: 24 capturas.
  - **Cono de luz:** bien orientado, sin `CONE_FLIP_V` (quedó en su valor). En `playtest/TDCPlaytest_02_luces.png` la parte estrecha
    sale de la lente del reflector y se abre hacia abajo hasta el plano de juego; no lava al director (`04_luces`). Es bastante
    opaco: en `02_luces` tapa buena parte del skyline detrás.
  - **W1 ausente:** en `playtest/TDCPlaytest_03_luces.png` (L2 apagado) se ve a la derecha como un panel muy tenue con borde
    apenas marcado; ya no es un panel gris sólido, pero el relleno todavía se percibe (más "vidrio" que "solo contorno").
  - **Lluvia y vapor:** translúcidos (rayas blancas y pluma de vapor que dejan ver el fondo) en playtest y autorun.
  - **Muros de ladrillo:** no tapan el skyline; en `01-escena.png` el muro queda a la derecha y se ven las torres por arriba.
  - **Adoquín:** no se ve en `01-escena.png` (el piso visible es la losa clara del escenario; abajo hay una franja blanca).
  - **Director entero y HUD legible** en `01-escena`, en las 11 capturas del playtest y en casi todo el autorun (en
    `TDCAutoRun_13` y `_23` el piso de primer plano le tapa los pies; en `_05`, `_11`, `_16`, `_21` la cámara mira la losa
    y el director no aparece).
  - **VFX pintados:** `autorun/TDCAutoRun_03.png` (impacto de claquetazo: aro dorado con esquirlas) y
    `autorun/TDCAutoRun_08.png` (derrota: "¡Corten! Extra fuera de la toma", aro/disco blanco). Aturdido: el playtest lo
    valida (`claquetazo_resistente: aturdido=1`) pero no encontré una captura donde se distinga el sprite sobre el extra.
- errores: ninguno. Warnings de `build_dressing.py`: 13, todos "No smoothing group information" de los FBX y un
  "Allow CPU Access" de `SM_Ledge_Mid` (los mismos de corridas anteriores).
- evidencia: `docs/evidence/T-0055/` (`01-escena.png`, `playtest/` con 11 capturas + `playtest.log`, `autorun/` con 24 capturas).
- notas / preguntas:
  - `import_vfx.py` importó **10 sheets, no 15**: en `Art/Export/VFX/*.png` de esta rama hay 10 (1 + los 9 "restantes").
    Si tenían que ser 15, faltan 5 PNG en el repo.
  - Limpieza de la T-0053 cancelada: no había locks LFS. Los cambios sin commitear de `Game/Content` (51 modificados + assets
    sin trackear del kit/fachadas) no los descarté: quedaron en `git stash` de esta máquina ("T-0053 cancelada: restos sin
    commitear"). Si confirman que no sirven, lo borro.
  - Assets nuevos: `M_TDCLightCone`, `T_LightCone`, `MI_Card_anim_lluvia_60_30`, `MI_Card_anim_vapor_100_55`,
    `M_Card_mid_pared_ladrillo_50`, `T_Card_mid_pared_ladrillo_50`. Quedaron huérfanos de la versión anterior (no los borré):
    `MI_Card_anim_lluvia_80`, `MI_Card_anim_vapor_100`, `M_/T_Card_mid_pared_ladrillo_65`.
  - Los locks los tomé con globs por carpeta (142 en total) porque el `ForEach-Object` del paso 3 no corre en el worker sin interfaz.
  - No toqué C++, `Tools/` ni `CONE_FLIP_V`. No empaqueté.
