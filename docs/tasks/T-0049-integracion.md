---
id: T-0049
titulo: Integración post-jam en una sola corrida (T-0037 a T-0048)
estado: ok
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
0. **Reenvío (c1fd9b2):** los 3 PNG pintados ya están completos en LFS (lluvia 765642 B, vapor 3553471 B, claquetazo 103241 B) y
   LightReactive evalúa la luz con los bounds del root (la sombra de guion fallaba porque el puppet escalado inflaba la caja). Hacé
   `git lfs pull` después del checkout. Corré **todos** los pasos de nuevo (los scripts son idempotentes).
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
- [x] Compila; los 5 scripts sin `[TDC] ERROR`.
- [x] Playtest y replay en RESULT PASS (con `claquetazo_*`).
- [x] El exe del zip arranca en el menú; 3 extras con escala ajustada; quedó abierto.

## Permite
- Editar `Tools/unreal/*.py` y `Tools/scripts/tdc.ps1` solo para compatibilidad de API.

## No hacer
- No cambiar C++. No tocar los zips de la jam. No commitear zips ni `Tools/unreal/_diag_*`.

## Evidencia pedida
`01-escena.png`, líneas del playtest y replay, `autorun/`, `02-menu.png`, tamaño del zip.

---

## Reporte (lo completa Windows)
- estado: ok
- sha: (el del commit del reporte; ver `[TDC-REPORT]`)
- resumen: Reenvío, rama y sha `925df4d` verificados; `git lfs pull` trajo los 3 PNG completos (765642 / 3553471 / 103241 B).
  Se corrieron **todos** los pasos de nuevo.
  - **Paso 2 OK**: compila (`Result: Succeeded`, 0 errores, 0 warnings de C++).
  - **Paso 3 OK**: 121 locks (mapa + 120 assets), todos tomados en esta corrida.
  - **Paso 4 OK**: los 5 scripts por commandlet, `Python script executed successfully`, 0 `[TDC] ERROR`, sin `libpng error`.
    `build_dressing.py` necesitó un ajuste de API (ver notas) y se corrió dos veces; la segunda sin warnings `[TDC]`.
  - **Paso 5: captura hecha, pero no sirve para lo pedido** (ver notas): `shot` ya no encuadra el plano de juego.
    La descripción de la escena sale de `autorun/` y `playtest/`.
  - **Paso 6 OK**: playtest **RESULT PASS 20/20** (`claquetazo_comun`, `claquetazo_resistente`, y ahora también
    `sombra_aparece` y `sombra_fuera_de_toma`).
  - **Paso 7 OK**: replay de `best.json` **RESULT PASS 15/15**, 0 correcciones de drift.
  - **Paso 8 OK**: autorun, 24 capturas.
  - **Paso 9 OK**: `BUILD SUCCESSFUL`; zip `Game\Saved\stevearagonsite_TheDirectorsCut_dev.zip` = **370.2 MB**. El exe del zip
    arranca en el **menú inicial** (`02-menu.png`: título, Jugar / Controles / Salir) y en su log están las 3 líneas de escala.
  - **Paso 11 OK**: el build quedó abierto (`smoke -Keep`, `03-abierto.png`).
- errores: ninguno que corte. Warnings:
  - `build_dressing.py` (1.ª corrida): `[TDC] niebla volumétrica no disponible (ExponentialHeightFogComponent: Failed to find property
    'volumetric_fog' ...)` → corregido (ver notas).
  - `build_dressing.py`: 6 warnings de Interchange `No smoothing group information was found for this mesh 'model' in the FBX file`
    (SM_Spotlight, SM_Crates, SM_FilmCamera, SM_StageLight_Hanging, SM_DirectorChair, SM_BoomMic). No cortan.
  - 1.er intento de `smoke`: `Remove-Item ... PackageTest\Engine\Binaries\ThirdParty\DbgHelp\dbghelp.dll: Access to the path is denied`
    (`Tools/scripts/tdc.ps1:278`): había un exe viejo de `PackageTest` abierto; el script lo mata y borra sin esperar. Al reintentar anduvo.
- playtest (`docs/evidence/T-0049/playtest/playtest.log`):
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
  [TDC-PLAYTEST] PASS sombra_aparece: con L2 apagado la sombra de guion está en escena
  [TDC-PLAYTEST] PASS sombra_fuera_de_toma: prender L2 de nuevo saca a la sombra de la toma
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
  [TDC-REPLAY] corrida guardada: Saved/TDCRuns/bot-20261115-232744.json (18 eventos, meta=1)
  [TDC-PLAYTEST] RESULT PASS 20/20
  ```
- replay (resumen; completo en `docs/evidence/T-0049/replay/replay.log`): `corrida best.json: 18 eventos, 0 checkpoints, 16.7 s` ·
  `GOAL en 16.7s` · `correcciones de drift: 0` · `RESULT PASS 15/15` (los 15 chequeos incluyen `claquetazo_comun` y
  `claquetazo_resistente`; el replay no evalúa `p1_sostiene`, `puente_sostiene`, `caja_cae` ni los dos de la sombra).
- log del exe empaquetado (`smoke`):
  ```
  LogTDC: TDCExtra_3: pose 1.4 cm, mesh 175.0 cm -> escala del puppet 110.300
  LogTDC: TDCExtra_4: pose 1.5 cm, mesh 180.0 cm -> escala del puppet 109.112
  LogTDC: TDCExtra_5: pose 1.6 cm, mesh 190.0 cm -> escala del puppet 109.395
  ```
- escena (de `autorun/TDCAutoRun_02/08/16/22.png` y `playtest/TDCPlaytest_00/03.png`):
  - **Valores de luz: sí.** El piso del plano de juego es crema claro y los personajes se leen con color (director gris, extra de traje
    rojo); el telón y los edificios quedan en azules oscuros. El plano de juego le gana al fondo.
  - **Calle de adoquín: no la vi.** En ninguna captura aparece adoquín: debajo del plano de juego el encuadre termina en el borde del
    piso o en el pozo oscuro (`TDCAutoRun_22`). `T_Street_Cobble` se importó bien; no pude confirmar que se vea en cámara.
  - **Marquesina a nivel calle: sí**, apoyada justo detrás del piso de juego, a la altura del director.
  - **Reflector colgante: sí**, se ve arriba a la derecha en `TDCAutoRun_16` y `_22`.
  - **Extras a tamaño humano: sí.** El extra de rojo es apenas más alto que el director (`TDCAutoRun_02`).
  - **Cámara más alta: sí**, el piso queda en el quinto inferior del cuadro. En `TDCPlaytest_00_inicio` y `_03_luces` el director sale
    **cortado por el borde de abajo** (se le ven cabeza y torso); en el autorun entra entero. A revisar por la Mac.
  - **HUD nuevo: sí.** Panel de teclas arriba a la izquierda (moverse, saltar, claquetazo, prender/apagar reflector, rebobinar) con
    leyenda de colores; carteles `[E] ¡Luces! Prender el reflector`, `¡Claquetazo!`, `¡Corten! Extra fuera de la toma`. Sin texto de debug.
  - **Squash en el claquetazo: no se puede juzgar en fotos fijas.** Sí se ve el VFX pintado del impacto (destello blanco sobre el
    director en `TDCAutoRun_16` y `_22`).
  - **Utilería: no tapa al director** en las capturas. La farola de primer plano cruza el cuadro en `TDCAutoRun_16` sin taparlo. Las
    cards de lluvia son bastante opacas y cruzan todo el cuadro; el vapor es una mancha blanca fuerte (`TDCPlaytest_03`).
- evidencia: `docs/evidence/T-0049/` — `01-escena.png`, `02-menu.png`, `03-abierto.png`, `autorun/` (24), `playtest/` (11 +
  `playtest.log`), `replay/` (11 + `replay.log`).
- notas / preguntas:
  - **`01-escena.png` no muestra el encuadre de juego.** Se sacó dos veces con el mismo resultado: vista a lo largo del eje X, desde
    atrás de un personaje, con mallas sin material en la primera. `tdc.ps1 shot` dispara `HighResShot` al arrancar, y desde T-0046 el
    juego arranca en el menú: la toma sale antes de que exista la cámara lateral (y `HighResShot` no incluye la UI). No lo toqué porque
    no es compatibilidad de API. Sugerencia: que `shot` use un flag que saltee el menú, o `Screenshot showui` con espera como `smoke`.
  - **Cambio en `Tools/unreal/build_dressing.py` (compatibilidad de API, permitido):** en UE 5.8 la propiedad de Python es
    `enable_volumetric_fog` (por `bEnableVolumetricFog`), no `volumetric_fog`. Ahora prueba la nueva y cae a la vieja. Con eso la niebla
    volumétrica quedó **prendida** por primera vez (antes el `try` la salteaba con warning): puede cambiar el look respecto de lo que
    venían viendo.
  - En este entorno `tdc.ps1 python` no muestra las líneas `[TDC]` de los scripts por stdout (solo warnings); se verificó por
    `Python script executed successfully`, el resumen `0 error(s)` y el log del editor.
  - `Tools/playtest/runs/best.json` sin cambios (idéntico al de `Game\Saved\TDCRuns`, verificado con `cmp`).
  - Contenido commiteado: salida de los 5 scripts (`Materials/`, `VFX/` con 4 flipbooks nuevos, `UI/`, `Dressing/` con `Props/` y
    `SetProps/` nuevos, `Characters/`) y `L_Slice.umap`.
  - Sigue en `git stash` el cambio ajeno de la corrida anterior ("T-0036 estado en-curso (resto previo a T-0049)"), sin descartar.
  - No se commitearon zips ni `Tools/unreal/_diag_*`. Locks: los 121 soltados al terminar.
