# Movimiento 2.5D: implementation

**Updated:** 2026-11-15 · [Design](design.md) · [Testing](testing.md)

## Approach
`ACharacter` estándar con `CharacterMovementComponent` restringido a un plano (normal +Y) y una cámara en spring arm sin colisión. Si no se asignan assets de Enhanced Input, el personaje arma un mapeo por defecto en C++, así el greybox corre sin assets.

## Code map
| Archivo | Rol |
|---|---|
| `Game/Source/TheDirectorsCut/Public/Character/TDCCharacter.h` / `Private/Character/TDCCharacter.cpp` | Movimiento, cámara, input, reaparición |
| `Game/Source/TheDirectorsCut/Public/Stage/TDCStageTriggers.h` (`ATDCRespawnVolume`) | Volumen de pozo que dispara `Respawn()` |

## Datos y config
- **Constantes** (constructor de `ATDCCharacter`): `MaxWalkSpeed` = velocidad máxima, `JumpZVelocity` = velocidad inicial de salto, `GravityScale`, `AirControl`, `GroundFriction`, `BrakingDecelerationWalking`, `RotationRate`, `CameraBoom->TargetArmLength` = distancia de cámara, `SocketOffset.Z` = altura de cámara, `CameraLagSpeed`.
- **Input por defecto:** A/D o flechas o stick izquierdo = mover · Espacio / gamepad A = saltar · E / gamepad X = luz · Q / gatillo izquierdo (mantener) = rebobinar.
- **Persistencia:** ninguna.

## Desvíos respecto del diseño
Ninguno conocido.

## History
- 2026-11-15: documentado desde el código en `main` (d81f112). Todavía no compiló (T-0001 pendiente).
