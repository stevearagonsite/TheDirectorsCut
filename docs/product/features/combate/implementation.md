# Combate: implementation

**Updated:** 2026-11-15 · [Design](design.md) · [Testing](testing.md)

## Approach
`UTDCDirectorComponent::Clap()` hace un overlap de esfera delante del director y le pasa el golpe a cada `ATDCExtra` (`ReceiveClap`). El resultado sale de `TDC::ResolveClap(Vulnerability)`: derrota, aturde o rebota. El feedback (hit-stop, toast, VFX, sonido) lo arma el director según el resultado más fuerte. La presencia de los extras de guion la decide `UTDCLightReactiveComponent`, igual que en las piezas de luz.

## Code map
| Archivo | Rol |
|---|---|
| `Public/Director/TDCDirectorComponent.h` | `Clap()`, alcance, radio y cooldown; hit-stop |
| `Public/Combat/TDCExtra.h` | Extra: vulnerabilidad, patrulla, aturdido, puppet animado (Rodin) con escala auto-ajustada |
| `Public/Combat/TDCCombatTypes.h` (`ETDCVulnerability`, `ETDCClapResult`, `TDC::ResolveClap`) | Tabla vulnerabilidad → resultado |
| `Public/Character/TDCCharacter.h` | Input F, clip y squash & stretch del claquetazo (`ApplyClapPunch`) |
| `Public/UI/TDCHUD.h` | Toast del resultado (`TDC::HudMessage`, clave 7002) |

Las rutas son relativas a `Game/Source/TheDirectorsCut/`.

## Datos y config
- **Constantes:** `ClapReach`, `ClapRadius`, `ClapCooldown` (director) · `StunSeconds`, `KnockbackDistance`, `PatrolDistance`, `PatrolSpeed`, `WalkBobCm`, `WalkSwayDegrees` (extra) · `ClapDuration`, `ClapStartFraction` (personaje). Valores en el [diseño](design.md#números).
- **Assets:** `/Game/Characters/Extras/<Comun|Resistente|Sombra>/SK_Extra_*` y `AS_Extra_*_Idle/_Walk` (`Tools/unreal/import_extras.py`, `apply_toon.py`).
- **Persistencia:** ninguna.

## Desvíos respecto del diseño
Ninguno conocido.

## History
- 2026-11-15: documentado desde el código de `t/T-0049-integracion`.
