---
id: T-0032
titulo: Playtest automático de los beats (-TDCPlaytest) y replay de la corrida
estado: ok
tipo: test
rama: t/T-0032-playtest
creada: 2026-11-15
---

# T-0032 — Playtest automático

## Objetivo
Pedido de Steve: "un agente para el testing, así no lo hago yo a mano". Un bot recorre los beats (luces, rebobinado del puente,
claquetazo a los extras y salida) con aserciones `[TDC-PLAYTEST] PASS/FAIL` y capturas por paso. Sin package.
Guía: [`docs/pipeline/playtest.md`](../pipeline/playtest.md).

## Contexto
- C++ nuevo: `ATDCPlaytestRunner` (modos `-TDCPlaytest`, `-TDCRecord`, `-TDCReplay`). Módulo `Json` en el Build.cs.
- `tdc.ps1 playtest | record | replay -Run`: corren el editor en `-game`, resumen las líneas `[TDC-*]`, copian capturas a
  `docs/evidence/<Task>/<cmd>/` y salen con código ≠ 0 si no hay `RESULT PASS`.
- La rama trae main con mejora 5. Si T-0033 (mejora 6) ya se mergeó, da igual: el playtest no depende de las animaciones.

## Pasos
0. **Reenvío:** 6a9ed7f corrige `TDCPlaytestRunner.cpp:309` (sobraba un `*Check.Name`) y se auditaron los demás Printf/UE_LOG.
1. `git fetch` y `git checkout t/T-0032-playtest` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop`.
2. `.\Tools\scripts\tdc.ps1 build`. Si no compila, **pará** y reportá `fallo` con `archivo:línea`.
3. `.\Tools\scripts\tdc.ps1 playtest -Task T-0032`. Pegá en el Reporte todas las líneas `[TDC-PLAYTEST]` (PASS/FAIL por paso y RESULT).
4. Si dio `RESULT PASS`: `.\Tools\scripts\tdc.ps1 replay -Task T-0032 -Run` (usa el `best.json` que dejó el bot) y pegá su resumen.
5. Si algún paso dio FAIL, describí qué muestra la captura de ese paso (por ejemplo, el supuesto de que el puente se dispara
   teletransportándose a (4560, 100) no se verificó).
6. Commit de `docs/evidence/T-0032/**`, `Tools/playtest/runs/best.json` (si existe) y el Reporte. Push y `[TDC-REPORT]`.

## Criterios de aceptación
- [x] Compila.
- [x] `tdc.ps1 playtest` termina con un `RESULT` (PASS o FAIL con pasos identificados) y capturas por paso.

## Permite
- Editar `Tools/scripts/tdc.ps1` solo para que `playtest`/`replay` funcionen.

## No hacer
- No cambiar C++, mapas ni assets. No empaquetar. No tocar zips.

## Evidencia pedida
Líneas `[TDC-PLAYTEST]`, capturas por paso en `docs/evidence/T-0032/playtest/` y, si aplica, el resumen del replay.

---

## Reporte (lo completa Windows)
- estado: ok
- sha: 59d8df1 (base probada; el sha del reporte va en el `[TDC-REPORT]`)
- resumen: compila sin errores ni warnings de C++ (el fix de `TDCPlaytestRunner.cpp:309` alcanzó). `tdc.ps1 playtest` dio
  `RESULT PASS 18/18` y llegó a la meta en 16.7 s; `tdc.ps1 replay` de `best.json` dio `RESULT PASS 13/13` con 0 correcciones
  de drift. **Ojo:** las verificaciones `claquetazo_*` no se ejecutaron en ninguna de las dos corridas (ver notas), así que
  el PASS no cubre el claquetazo a los extras.
- errores: ninguno.
- líneas `[TDC-PLAYTEST]` del bot (`docs/evidence/T-0032/playtest/playtest.log`):
  ```
  [TDC-PLAYTEST] START modo=bot
  [TDC-PLAYTEST] escena: 3 reflectores, 4 tablas, caja=1, extras C/G/R=1/1/1
  [TDC-PLAYTEST] PASS luces_L1_off_P1: inicio (L1=off) P1: presente=0 (esperado 0), colision=0
  [TDC-PLAYTEST] PASS luces_L2_on_W1: inicio (L2=on) W1: presente=1 (esperado 1), colision=1
  [TDC-PLAYTEST] PASS luces_L2_on_S1: inicio (L2=on) S1: presente=0 (esperado 0), colision=0
  [TDC-PLAYTEST] PASS luces_L3_on_SH: inicio (L3=on) SH: presente=1 (esperado 1), colision=1
  [TDC-PLAYTEST] PASS luces_L3_on_P3: inicio (L3=on) P3: presente=0 (esperado 0), colision=0
  [TDC-PLAYTEST] teleport a X=692 Z=100 (salto de beat del bot)
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
  [TDC-PLAYTEST] teleport a X=6850 Z=100 (salto de beat del bot)
  [TDC-PLAYTEST] PASS luces_L3_off_SH: después de E (L3=off) SH: presente=0 (esperado 0), colision=0
  [TDC-PLAYTEST] PASS luces_L3_off_P3: después de E (L3=off) P3: presente=1 (esperado 1), colision=1
  [TDC-PLAYTEST] PASS caja_cae: con L3 apagado la caja bajó 302 cm
  [TDC-PLAYTEST] teleport a X=7400 Z=480 (salto de beat del bot)
  [TDC-PLAYTEST] PASS meta: llegó a la salida en 16.7 s
  [TDC-REPLAY] GOAL en 16.7s
  [TDC-REPLAY] corrida guardada: Saved/TDCRuns/bot-20261115-225336.json (18 eventos, meta=1)
  [TDC-REPLAY] best.json actualizado (bot)
  [TDC-PLAYTEST] RESULT PASS 18/18
  ```
- resumen del replay (`docs/evidence/T-0032/replay/replay.log`; mismas líneas `luces_*`, `puente_*` y `meta` en PASS):
  ```
  [TDC-PLAYTEST] START modo=replay
  [TDC-REPLAY] corrida best.json: 18 eventos, 0 checkpoints, 16.7 s
  [TDC-PLAYTEST] escena: 3 reflectores, 4 tablas, caja=1, extras C/G/R=1/1/1
  ... (12 PASS de luces_* y puente_*, sin FAIL)
  [TDC-PLAYTEST] PASS meta: llegó a la salida en 16.7 s
  [TDC-REPLAY] GOAL en 16.7s
  [TDC-REPLAY] correcciones de drift: 0
  [TDC-PLAYTEST] RESULT PASS 13/13
  ```
- evidencia: `docs/evidence/T-0032/playtest/` y `docs/evidence/T-0032/replay/` (11 capturas + log cada una);
  corrida del bot en `Tools/playtest/runs/best.json` (18 eventos, 0 checkpoints).
- notas / preguntas:
  - **`claquetazo_*` nunca reporta.** El bot da los dos claquetazos y pegan (log del juego: `Claquetazo a TDCExtra_3 -> 0`
    en t=1.1 s y `Claquetazo a TDCExtra_5 -> 1` en t=14.0 s), pero no aparece ninguna línea `claquetazo_comun/resistente`.
    Causa probable (leyendo el código, sin tocarlo): `ATDCCharacter::PerformAction` llama a `DoClap()` **antes** de
    `NotifyAction` (`TDCCharacter.cpp:512-519`), y `NotifyAction` arma `Targets` con los extras `!IsDefeated() && !IsHidden()`
    (`TDCPlaytestRunner.cpp:459`). El común ya está derrotado y oculto (`Defeat()` hace `SetActorHiddenInGame(true)`), así que
    `Targets` queda vacío y `CheckClap` no se programa. Para el resistente no lo verifiqué: o también queda fuera del filtro
    (¿el `-> 1` es "derrotado de un golpe"?) o falla el rango/Z. Sugerencia: juntar los `Targets` antes del golpe y
    reportar FAIL si un claquetazo del bot no tuvo ningún extra al alcance.
  - El supuesto del puente se verificó: teletransportarse a (4560, 100) dispara la caída (`puente_cae`: 103 cm) y el
    rebobinado devuelve las tablas (0 cm).
  - Capturas: las que se sacan pegadas a un teleport salen con motion blur fuerte (`01_claquetazo`, `08_claquetazo`,
    `10_meta`; en la de la meta no se distingue al director). Revisé esas tres y `06_rebobinando`, que se ve bien (director
    sobre el piso con el efecto de rebobinado); las demás no las abrí. Sugerencia: capturar unos frames después del
    teleport o apagar el motion blur en los modos de playtest.
  - `best.json` del bot tiene 0 checkpoints, así que el replay no tiene contra qué medir drift ("0 correcciones" no dice mucho).
  - El paso 4 dice `replay -Task T-0032 -Run` sin valor; `-Run` es un parámetro string (default `best.json`), así que
    corrí `replay -Task T-0032 -Run best.json`. No hizo falta tocar `tdc.ps1`.
  - El replay tomó `Game\Saved\TDCRuns\best.json` (la misma corrida que quedó copiada en `Tools/playtest/runs/`).
  - En esta rama (sin T-0033) el log avisa `SK_Extra_Comun/Sombra/Resistente has no skeleton`; es lo que arregla T-0033.
  - `tdc.ps1 stop` no encontró editor del proyecto abierto. No se tocó C++, mapas, assets ni zips; no se empaquetó.
  - Toolchain: MSVC 14.52.37029 (UBT avisa que no es la preferida, 14.50.35717); UE 5.8.
