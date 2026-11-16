---
id: T-0049
titulo: Integración post-jam en una sola corrida (T-0037 a T-0048)
estado: fallo
tipo: mixto
rama: t/T-0049-integracion
creada: 2026-11-15
---

# T-0049 — Integración

## Objetivo
Reemplaza la cola de 10 tareas sueltas por una sola corrida. `main` + T-0037 (hit-stop), T-0044 (extras al arrancar), T-0046 (salto
asistido, que trae T-0040/T-0043/T-0045: menú, pausa, HUD, cámara, feel del claquetazo), T-0047 (fixes del playtest) y T-0048 (VFX pintados,
que trae T-0039/T-0041/T-0042: arte, valores de luz, calle). Genera `Game\Saved\stevearagonsite_TheDirectorsCut_dev.zip`.
Si algo falla, reportá qué paso y el error: se bisecta con las ramas sueltas.

## Pasos
1. `git fetch` y `git checkout t/T-0049-integracion` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop`.
2. `.\Tools\scripts\tdc.ps1 build`. Si no compila, **pará** y reportá `fallo` con `archivo:línea`.
3. Locks: `git lfs lock Game/Content/Maps/L_Slice.umap` y
   `git ls-files Game/Content/Materials Game/Content/Dressing Game/Content/VFX Game/Content/UI Game/Content/Characters | ForEach-Object { git lfs lock $_ }`.
4. Scripts, en este orden (cada uno sin `[TDC] ERROR`; si alguno no anda por commandlet, `-InEditor`):
   `make_materials.py` → `import_vfx.py` → `import_ui.py` → `build_dressing.py` → `apply_toon.py`.
5. `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0049 -Name 01-escena`. Describí: valores de luz (el plano de juego le gana
   al fondo), calle de adoquín abajo, marquesina a nivel calle, reflector colgante, extras a tamaño humano y cámara más alta.
6. `.\Tools\scripts\tdc.ps1 playtest -Task T-0049` → **RESULT PASS**, con las líneas `claquetazo_*`. Pegá todas las `[TDC-PLAYTEST]`.
7. `.\Tools\scripts\tdc.ps1 replay -Task T-0049 -Run best.json` → pegá el resumen.
8. `.\Tools\scripts\tdc.ps1 autorun -Task T-0049 -Wait 45`: HUD nuevo (sin texto de debug), squash en el claquetazo, sin utilería tapando.
9. `.\Tools\scripts\tdc.ps1 package -Zip stevearagonsite_TheDirectorsCut_dev` y
   `.\Tools\scripts\tdc.ps1 smoke -Task T-0049 -Name 02-menu -Wait 30 -Zip stevearagonsite_TheDirectorsCut_dev`: la captura tiene que mostrar
   el **menú inicial**. En el log del exe, las 3 líneas `-> escala del puppet` (la sombra incluida).
10. Commit de `Game/Content/**` (LFS), evidencia, `Tools/playtest/runs/best.json` y Reporte (con tamaño del zip). `git lfs unlock`. Push y `[TDC-REPORT]`.
11. Dejá abierto el build para Steve: `.\Tools\scripts\tdc.ps1 smoke -Task T-0049 -Name 03-abierto -Keep -Zip stevearagonsite_TheDirectorsCut_dev`.

## Criterios de aceptación
- [ ] Compila; los 5 scripts sin `[TDC] ERROR`.
- [ ] Playtest y replay en RESULT PASS (con `claquetazo_*`).
- [ ] El exe del zip arranca en el menú; 3 extras con escala ajustada; quedó abierto.

## Permite
- Editar `Tools/unreal/*.py` y `Tools/scripts/tdc.ps1` solo para compatibilidad de API.

## No hacer
- No cambiar C++. No tocar los zips de la jam. No commitear zips ni `Tools/unreal/_diag_*`.

## Evidencia pedida
`01-escena.png`, líneas del playtest y replay, `autorun/`, `02-menu.png`, tamaño del zip.

---

## Reporte (lo completa Windows)
- estado: fallo
- sha: (el del commit del reporte; ver `[TDC-REPORT]`)
- resumen: Rama y sha `f4aeb73` verificados. **Paso 2 OK**: compila (`Result: Succeeded`, 0 errores, 0 warnings de C++).
  **Paso 3 OK**: 112 locks (mapa + 111 assets). **Paso 4 falla en `import_vfx.py` y en `build_dressing.py`**: los tres PNG
  pintados de T-0048 (commit `dd544f2`) están en el repo **truncados a 1024 bytes exactos** (el objeto LFS mismo mide 1024; no es un
  problema de checkout). `make_materials.py`, `import_ui.py` y `apply_toon.py` corrieron sin `[TDC] ERROR`.
  **Paso 6 (playtest): RESULT FAIL 18/20** (fallan `sombra_aparece` y `sombra_fuera_de_toma`; los dos `claquetazo_*` pasan).
  **Paso 7 (replay de `best.json`): RESULT PASS 15/15**, 0 correcciones de drift.
  **No se corrieron** los pasos 5 (shot), 8 (autorun), 9 (package + smoke) ni 11 (dejar abierto): sin `build_dressing.py` el mapa
  no se rearmó y el zip no sería el entregable. No hay zip `_dev` nuevo.
- errores:
  - `import_vfx.py` y `build_dressing.py`: `UnrealEditor-Cmd.exe : libpng error: IDAT: unknown compression method`
    (`Tools/scripts/tdc.ps1:133`; con `$ErrorActionPreference = 'Stop'` esa línea de stderr corta el wrapper y mata el commandlet).
  - Archivos truncados (cabecera PNG válida, datos cortados a 1024 bytes):
    - `Art/Export/VFX/claquetazo_impact.png` — IHDR 768x640, LFS oid `58b66ed355…`, size 1024
    - `Art/Export/Dressing/Cards/anim_lluvia.png` — IHDR 2048x2048, LFS oid `500ed83c5a…`, size 1024
    - `Art/Export/Dressing/Cards/anim_vapor.png` — IHDR 1792x1792, LFS oid `1d01d243de…`, size 1024
  - `build_dressing.py` llegó a loguear `[TDC] parallax de 3 capas armado` y murió al importar las cards animadas.
  - Playtest:
    ```
    [TDC-PLAYTEST] START modo=bot
    [TDC-PLAYTEST] escena: 3 reflectores, 4 tablas, caja=1, extras C/G/R=1/1/1
    [TDC-PLAYTEST] PASS luces_L1_off_P1: inicio (L1=off) P1: presente=0 (esperado 0), colision=0
    [TDC-PLAYTEST] PASS luces_L2_on_W1: inicio (L2=on) W1: presente=1 (esperado 1), colision=1
    [TDC-PLAYTEST] PASS luces_L2_on_S1: inicio (L2=on) S1: presente=0 (esperado 0), colision=0
    [TDC-PLAYTEST] PASS luces_L3_on_SH: inicio (L3=on) SH: presente=1 (esperado 1), colision=1
    [TDC-PLAYTEST] PASS luces_L3_on_P3: inicio (L3=on) P3: presente=0 (esperado 0), colision=0
    [TDC-PLAYTEST] teleport a X=692 Z=100 (salto de beat del bot)
    [TDC-PLAYTEST] PASS claquetazo_comun: extra común derrotado
    [TDC-PLAYTEST] teleport a X=1000 Z=100 (salto de beat del bot)
    [TDC-PLAYTEST] PASS luces_L1_on_P1: después de E (L1=on) P1: presente=1 (esperado 1), colision=1
    [TDC-PLAYTEST] teleport a X=1600 Z=100 (salto de beat del bot)
    [TDC-PLAYTEST] PASS p1_sostiene: director sobre P1 con L1 prendido (Z=92)
    [TDC-PLAYTEST] teleport a X=2400 Z=100 (salto de beat del bot)
    [TDC-PLAYTEST] PASS luces_L2_off_W1: después de E (L2=off) W1: presente=0 (esperado 0), colision=0
    [TDC-PLAYTEST] PASS luces_L2_off_S1: después de E (L2=off) S1: presente=1 (esperado 1), colision=1
    [TDC-PLAYTEST] FAIL sombra_aparece: con L2 apagado la sombra de guion está en escena
    [TDC-PLAYTEST] FAIL sombra_fuera_de_toma: prender L2 de nuevo saca a la sombra de la toma
    [TDC-PLAYTEST] teleport a X=4300 Z=100 (salto de beat del bot)
    [TDC-PLAYTEST] teleport a X=4560 Z=100 (salto de beat del bot)
    [TDC-PLAYTEST] PASS puente_cae: las tablas bajaron 103 cm
    [TDC-PLAYTEST] PASS puente_rebobinado: tablas a 0 cm o menos de su lugar
    [TDC-PLAYTEST] teleport a X=5250 Z=100 (salto de beat del bot)
    [TDC-PLAYTEST] PASS puente_sostiene: director sobre el puente rebobinado (Z=92)
    [TDC-PLAYTEST] teleport a X=6152 Z=100 (salto de beat del bot)
    [TDC-PLAYTEST] PASS claquetazo_resistente: aturdido=1 derrotado=0
    [TDC-PLAYTEST] teleport a X=6850 Z=100 (salto de beat del bot)
    [TDC-PLAYTEST] PASS luces_L3_off_SH: después de E (L3=off) SH: presente=0 (esperado 0), colision=0
    [TDC-PLAYTEST] PASS luces_L3_off_P3: después de E (L3=off) P3: presente=1 (esperado 1), colision=1
    [TDC-PLAYTEST] PASS caja_cae: con L3 apagado la caja bajó 302 cm
    [TDC-PLAYTEST] teleport a X=7400 Z=480 (salto de beat del bot)
    [TDC-PLAYTEST] PASS meta: llegó a la salida en 16.7 s
    [TDC-REPLAY] GOAL en 16.7s
    [TDC-REPLAY] corrida guardada: Saved/TDCRuns/bot-20261115-230924.json (18 eventos, meta=1)
    [TDC-PLAYTEST] RESULT FAIL 18/20
    ```
  - Replay (resumen): `corrida best.json: 18 eventos, 0 checkpoints, 16.7 s` · `GOAL en 16.7s` · `correcciones de drift: 0` ·
    `RESULT PASS 15/15` (el replay no chequea `p1_sostiene`, `puente_sostiene`, `caja_cae` ni los dos de la sombra).
- evidencia: `docs/evidence/T-0049/playtest/` (11 capturas + `playtest.log`) y `docs/evidence/T-0049/replay/` (11 capturas +
  `replay.log`). No hay `01-escena.png`, `autorun/` ni `02-menu.png` (pasos no corridos).
- notas / preguntas:
  - **Para la Mac:** re-exportar y commitear los tres PNG completos (`python3 Art/Source/VFX/pintados/vfx_pintados.py <dir>`).
    Acá no se pudieron regenerar: esta sesión sin interfaz no tiene permiso para ejecutar Python suelto.
  - La falla de la sombra **no está diagnosticada**. El playtest corrió con `L_Slice` sin rearmar (`build_dressing.py` no terminó),
    así que puede ser mapa viejo contra C++ nuevo o una regresión real del C++ integrado; hay que repetirlo con los 5 scripts OK
    antes de bisectar.
  - Las capturas del playtest muestran la escena rota (cámara muy cerca, cards sin acomodar): son del estado intermedio, no sirven
    para juzgar luz ni arte.
  - En el log del editor (`-game`) aparecen las 3 líneas `-> escala del puppet` (TDCExtra_3/4/5 ≈ 110.3 / 109.1 / 109.4); falta
    confirmarlo en el exe empaquetado.
  - `tdc.ps1 replay -Run best.json` toma primero `Game\Saved\TDCRuns\best.json` y recién después el del repo. Esta vez eran
    idénticos (`cmp`), pero puede reproducir una corrida local distinta de la commiteada.
  - Contenido commiteado: solo la salida de los scripts que terminaron (`Materials/`, `UI/` nuevo, `Characters/` por `apply_toon`).
    Los assets de `Dressing/Parallax` que dejó a medias `build_dressing.py` se descartaron con `git restore`. `L_Slice.umap` sin cambios.
    `Tools/playtest/runs/best.json` sin cambios.
  - Al llegar había un cambio sin commitear de otra tarea en `t/T-0039-luz-valores` (`docs/tasks/T-0036-arte-integracion.md`,
    `estado: enviada → en-curso`). Quedó en `git stash` ("T-0036 estado en-curso (resto previo a T-0049)"), sin descartar.
  - Locks: los 112 se soltaron al terminar (63 ya estaban tomados por esta misma cuenta desde una tarea anterior).
