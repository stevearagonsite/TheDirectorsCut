# Playtest automático

Recorre `L_Slice` sin que nadie juegue, verifica la escena y deja capturas. Lo implementa `ATDCPlaytestRunner`
(`Game/Source/TheDirectorsCut/Private/Debug/`), que crea `ATDCCharacter` según la línea de comandos.

| Comando (Windows) | Modo | Qué hace |
|---|---|---|
| `tdc.ps1 record` | `-TDCRecord` | Steve juega. Graba los inputs (mover, saltar, E, Q apretado/soltado, F) con su tiempo y, cada 0.5 s, la posición y velocidad del director. Guarda `Game/Saved/TDCRuns/run-<fecha>.json`. Si llega a la salida, la copia a `best.json`, loguea `[TDC-REPLAY] GOAL en Xs`, cierra el juego y `tdc.ps1` la deja en `Tools/playtest/runs/best.json` para commitearla. |
| `tdc.ps1 replay -Task T-XXXX [-Run <archivo.json>]` (string; si se omite, `best.json`) | `-TDCReplay[=archivo]` | Reproduce una corrida (por defecto `best.json`). En cada checkpoint, si el director se desvió más de 30 cm, lo corrige y loguea `drift`. |
| `tdc.ps1 playtest -Task T-XXXX` | `-TDCPlaytest` | Bot guionado: salta entre beats teletransportando al director (cada salto queda en el log) y usa los poderes. Si no hay `best.json`, su corrida se guarda como el primero. |

Los tres modos corren a paso fijo de 1/60 s (y 60 fps como máximo) para que grabación y replay coincidan.
`replay` y `playtest` acercan la cámara (como `-TDCAutoRun`) y salen solos al terminar.
`tdc.ps1` devuelve código distinto de 0 si hay algún `FAIL` o si no llegó a la meta.

## Qué verifica

Cada verificación es una línea del log: `[TDC-PLAYTEST] PASS|FAIL <paso>: <detalle>`. Al final aparece `[TDC-PLAYTEST] RESULT PASS|FAIL n/m`.

| Paso | Cuándo | Qué comprueba |
|---|---|---|
| `luces_<L>_<on/off>_<pieza>` | al arrancar y 0.2 s después de cada E | Las piezas de L1 (P1), L2 (W1, S1) y L3 (estante, P3) están presentes **y con colisión** solo cuando su luz lo indica. Ausente = sin colisión, aunque se vea el fantasma. |
| `claquetazo_comun/resistente/guion` | 0.25 s después de un claquetazo con un extra al alcance | El común queda derrotado, el resistente aturdido (no derrotado) y el de guion sigue en pie. |
| `puente_cae` / `puente_rebobinado` | al caer / al soltar Q | Las tablas bajan más de 1 m y, después de rebobinar, vuelven a menos de 25 cm de su lugar. Si cayó y nunca volvió: FAIL al final. |
| `meta` | siempre | El director llega parado al techo de `Exit_Ledge` (X entre 7200 y 7600, Z > 400). |
| Solo el bot: `p1_sostiene`, `sombra_aparece`, `sombra_fuera_de_toma`, `puente_sostiene`, `caja_cae` | en su beat | P1 lo sostiene con L1 prendido; apagar L2 trae la sombra y prenderla de nuevo la saca de la toma; el puente rebobinado lo sostiene; apagar L3 tira la caja. |
| `escena_*` | al arrancar | Encontró cada actor. |

Los actores se buscan por clase y por cercanía a su posición en `Tools/unreal/build_slice.py`, así funciona también en el build empaquetado (sin labels de editor). Si se mueve algo en `build_slice.py`, hay que actualizar las constantes de `TDCPlaytestRunner.cpp`.

Capturas: `TDCPlaytest_<NN>_<paso>.png` (inicio, luces, claquetazo, rebobinando, puente, meta). `tdc.ps1` las copia junto con el extracto del log a `docs/evidence/<Task>/<comando>/`.

## Agregar un paso

- **Verificación de escena:** en `ATDCPlaytestRunner::NotifyAction`, programá un chequeo con `After(segundos, [this]() { ... Report(TEXT("mi_paso"), bOk, Detalle); })`. Un PASS se cuenta una sola vez por nombre de paso; los FAIL se cuentan todos.
- **Paso del bot:** en `BuildBot()`, agregá `Step(espera, [this]() { ... })`. Para usar poderes, `Act(ETDCPlayAction::Lights, 1.f)`; para saltar de beat, `Teleport(FVector(x, 0, z), 1.f)`.
- **Nueva acción del jugador:** sumala a `ETDCPlayAction` (`Debug/TDCPlaytestTypes.h`), ejecutala en `ATDCCharacter::PerformAction` y agregale su nombre en `ActionName`.

## Formato de la corrida (`*.json`)

`{ "version": 1, "source": "record|bot", "duration": s, "goal": bool, "goalTime": s, "events": [ { "t", "a": "move|jump|lights|rewind|clap|teleport", "v", "x", "y", "z" } ], "checkpoints": [ { "t", "x", "y", "z", "vx", "vy", "vz" } ] }`
