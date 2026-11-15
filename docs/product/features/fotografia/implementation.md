# Dirección de fotografía (luz): implementation

**Updated:** 2026-11-15 · [Design](design.md) · [Testing](testing.md)

## Approach
Un `UWorldSubsystem` (`UTDCStageSubsystem`) registra los reflectores y emite `OnLightingChanged`. Las piezas con `UTDCLightReactiveComponent` escuchan ese evento y se reevalúan con funciones puras (`TDC::IsLit`, `TDC::IsPresent`), que son las que cubren los tests. Se eligió este diseño en lugar de chequear la luz en cada frame: es barato y determinista.

## Code map
| Archivo | Rol |
|---|---|
| `Public/Stage/TDCStageLight.h` (`ATDCStageLight`) | Reflector: estado, radio, visual |
| `Public/Stage/TDCStageSubsystem.h` | Registro de luces y evento de cambio |
| `Public/Stage/TDCLightReactiveComponent.h` | Presencia según la luz (oculta y desactiva la colisión) |
| `Public/Stage/TDCStageTypes.h` | `ETDCLightPresence`, `FTDCLightSample`, `TDC::IsLit`, `TDC::IsPresent` |
| `Public/Stage/TDCSetPiece.h` | Pieza greybox con color de lectura según la presencia |
| `Public/Director/TDCDirectorComponent.h` | `ToggleNearestLight`, `FindNearestLight` |

Las rutas son relativas a `Game/Source/TheDirectorsCut/`.

## Datos y config
- **Constantes:** `UTDCDirectorComponent::LightReach` = alcance del director · `ATDCStageLight::Radius` = radio · `ATDCStageLight::Intensity` · `ATDCStageLight::bStartsOn`.
- **Gating:** el bit `ETDCPower::Lights` en `UnlockedPowers` (desbloqueado por defecto en el prototipo).
- **Persistencia:** ninguna.

## Desvíos respecto del diseño
- El enum del código todavía usa los 4 poderes de vision v0.1 (`Lights`, `Rewind`, `Cut`, `Set`), no los departamentos de D-005. Hay que alinearlo cuando entre el equipamiento.

## History
- 2026-11-15: documentado desde el código en `main` (d81f112). Todavía no compiló (T-0001 pendiente).
