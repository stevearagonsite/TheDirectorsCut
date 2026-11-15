# Tiempo: implementation

**Updated:** 2026-11-15 · [Design](design.md) · [Testing](testing.md)

## Approach
`UTDCStageSubsystem` tiene un modo global (`Play` / `Rewind`). Cada pieza en toma tiene un `UTDCTimelineComponent` que, en `Play`, graba frames (transform + velocidades + si simula física) en un buffer circular `FTDCTimeHistory`, y en `Rewind` los consume en orden inverso (LIFO). El director solo cambia el modo global.

## Code map
| Archivo | Rol |
|---|---|
| `Public/Stage/TDCTimelineComponent.h` | Graba y rebobina una pieza |
| `Public/Stage/TDCStageTypes.h` (`FTDCTimeHistory`, `FTDCTimeFrame`, `ETDCTimeMode`) | Buffer circular y frame |
| `Public/Stage/TDCStageSubsystem.h` | Modo global del tiempo y su evento |
| `Public/Stage/TDCSetPiece.h` (`bRecordTime`) | Activa el timeline en la pieza |
| `Public/Stage/TDCStageTriggers.h` (`ATDCFallTrigger`) | Suelta piezas (activa la física) para crear algo que rebobinar |
| `Public/Director/TDCDirectorComponent.h` | `BeginRewind` / `EndRewind` |

Las rutas son relativas a `Game/Source/TheDirectorsCut/`.

## Datos y config
- **Constantes:** `UTDCTimelineComponent::RecordSeconds` = memoria · `SamplesPerSecond` = muestras · `RewindSpeed` = velocidad de rebobinado. La capacidad del buffer es `ceil(RecordSeconds × SamplesPerSecond)`, es decir 450 frames.
- **Gating:** el bit `ETDCPower::Rewind` en `UnlockedPowers` (desbloqueado por defecto en el prototipo).
- **Persistencia:** ninguna.

## Desvíos respecto del diseño
Ninguno con esta nota. "Adelantar" no está implementado y figura como pregunta abierta en el diseño.

## History
- 2026-11-15: documentado desde el código en `main` (d81f112). Todavía no compiló (T-0001 pendiente).
