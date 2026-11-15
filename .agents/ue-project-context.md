# UE Project Context

*Engine: **UE 5.8** (la misma versión a la que apuntan las skills `ue-*`, D-013) · Last updated: 2026-11-15*

## Engine & Project
**Engine version:** 5.8, launcher build (`EngineAssociation: "5.8"` en `Game/TheDirectorsCut.uproject`, lo ajusta T-0001). Instalado solo en Windows (`C:\Program Files\Epic Games\UE_5.8`), con Visual Studio y el .NET Framework 4.8 SDK; la Mac no tiene Unreal.
**Project name:** The Director's Cut · **Type:** game · **Genre:** metroidvania 2.5D
**Description:** Metroidvania 2.5D donde el jugador es un director que dirige la escena (luz, tiempo, corte, escenografía). Diseño: `docs/product/vision.md`.
**Target platforms:** no declarado en `.uproject` (`TargetPlatforms` vacío). Win64 de hecho (`docs/pipeline/comandos-windows.md`). Resto [unknown].

## Modules
**Primary game module:** `TheDirectorsCut`

| Module | Host type | Public deps | Private deps | Notes |
|---|---|---|---|---|
| TheDirectorsCut | Runtime (LoadingPhase Default) | Core, CoreUObject, Engine, InputCore, EnhancedInput | — | Todo el gameplay. `PCHUsage = UseExplicitOrSharedPCHs`, `PublicIncludePaths += Public` |

No hay módulo de editor.

## Plugins
| Plugin | Maturity | Used for |
|---|---|---|
| EnhancedInput | stable | Input del personaje (mapeo por defecto armado en C++) |
| PythonScriptPlugin | stable | Scripts de editor (`Tools/unreal/build_slice.py` genera `L_Slice`) |
| EditorScriptingUtilities | stable | Soporte de los scripts de editor |
| ModelingToolsEditorMode | stable | Solo editor (`TargetAllowList: Editor`) |

**In-house plugins:** ninguno (no hay `Game/Plugins/`).
**Fab / marketplace:** ninguno.

## Coding conventions
**Prefixes:** estándar F/U/A/E/I + prefijo de proyecto `TDC` (`ATDCCharacter`, `UTDCDirectorComponent`, `ETDCPower`).
**Object references:** `TObjectPtr` en `UPROPERTY`; `TWeakObjectPtr` para registros no dueños (luces en `UTDCStageSubsystem`).
**API macro style:** `THEDIRECTORSCUT_API` por declaración.
**Log categories:** `LogTDC` (todo el módulo, `Public/TheDirectorsCut.h`).
**Assertions:** no se usan `check`/`ensure` hasta ahora; política [unknown].
**Header layout:** `Public/` y `Private/` por módulo, una carpeta por sistema: `Core/`, `Character/`, `Director/`, `Stage/`, `Tests/` (privado).
**Enforced rules:** comentarios y docs en español, código y nombres en inglés (`CLAUDE.md`). Tests de automatización con prefijo `TDC.` (`docs/pipeline/convenciones.md`).

## Gameplay framework
| Role | Class |
|---|---|
| GameMode | `ATDCGameMode` (`AGameModeBase`; `GlobalDefaultGameMode` en `DefaultEngine.ini`) |
| GameState | default del engine |
| PlayerController | default del engine |
| PlayerState | default del engine |
| Pawn / Character | `ATDCCharacter` (`ACharacter`, movimiento restringido al plano XZ) |
| GameInstance | default del engine |

**Subsystems:** `UTDCStageSubsystem` (`UWorldSubsystem`): registro de reflectores, evento de cambio de luz y modo global del tiempo (Play/Rewind).
**Components:** `UTDCDirectorComponent` (poderes del jugador, bitmask `ETDCPower`), `UTDCLightReactiveComponent`, `UTDCTimelineComponent`.

## GAS
**Enabled:** no. Los poderes son un componente propio con bitmask de desbloqueo.

## Networking
**Model:** single-player.

## Input
**Stack:** EnhancedInput (`DefaultPlayerInputClass` / `DefaultInputComponentClass` de EnhancedInput en `DefaultInput.ini`).
**Mapping contexts:** no hay assets todavía; `ATDCCharacter::BuildDefaultInput()` crea contexto y acciones en C++ si los `UPROPERTY` de input están vacíos. **Added by:** `ATDCCharacter::SetupPlayerInputComponent` (prioridad 0).

## UI
**Stack:** [unknown]. No hay UI todavía (solo `AddOnScreenDebugMessage`).

## AI
**Stack:** [unknown]. Sin enemigos todavía.

## Streaming
**Model:** [unknown]. Un solo mapa: `/Game/Maps/L_Slice` (`GameDefaultMap` y `EditorStartupMap`).

## Saves
Sin guardado (fuera de alcance del prototipo).

## Build
**Targets:** Game (`TheDirectorsCut`), Editor (`TheDirectorsCutEditor`); ambos con `BuildSettingsVersion.Latest` e `IncludeOrderVersion.Latest`.
**Defines:** ninguno.
**Third-party:** ninguno.
**Engine modifications:** ninguna.

## Team
**Size & roles:** Steve (solo) + dos agentes Claude Code: la Mac (diseño, C++, docs, merge) y Windows (Unreal, Blender, compilación, evidencia).
**Source control:** Git + Git LFS (binarios con `git lfs lock`) · **Branching:** una rama `t/T-XXXX-slug` por tarea, `main` solo por PR.
**Review bar:** la Mac revisa y mergea · **Docs:** `docs/` (protocolo en `docs/pipeline/protocolo-mac-windows.md`).
