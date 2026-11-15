# Movimiento 2.5D

**Status:** in progress · **Updated:** 2026-11-15 · [Visión](../../vision.md) · [Implementation](implementation.md) · [Testing](testing.md)

El director corre y salta en un plano lateral mientras la cámara lo sigue de costado. Es la base de todo el plataformeo y tiene que ser preciso y legible, para que la dificultad esté en dirigir la escena y no en el control (pilar Dirección, sub-pilar *Legibilidad*).

## Reglas
1. El personaje se mueve solo en el plano XZ de Unreal (X horizontal, Z vertical). Y es profundidad decorativa.
2. Gira para mirar hacia donde se mueve.
3. El salto tiene altura fija: soltar el botón no lo corta.
4. Al aterrizar sobre un piso con tag `TDC.Safe`, ese punto pasa a ser el de reaparición.
5. Al caer a un pozo (volumen de reaparición) o fuera del mundo, reaparece en el último piso seguro, o en el inicio si no pisó ninguno.
6. La cámara es lateral y fija (no la rota el jugador), con un leve retraso al seguir.

## Números
| Nombre | Valor | Unidad | Qué mirar en playtest |
|---|---|---|---|
| Velocidad máxima | 600 | cm/s | ¿se siente lento entre puzzles? |
| Velocidad inicial de salto | 750 | cm/s | — |
| Escala de gravedad | 1.5 | × (g = 1470 cm/s²) | caída "pesada" vs flotante |
| Altura de salto (derivada) | ≈ 190 | cm | tiene que cubrir los escalones del greybox |
| Tiempo en el aire (derivado, piso a piso) | ≈ 1.0 | s | — |
| Distancia de salto a toda velocidad (derivada) | ≈ 610 | cm | ancho máximo de pozo diseñable |
| Control aéreo | 0.8 | 0–1 | corrección en el aire |
| Fricción en el piso | 8 | — | — |
| Frenado caminando | 2400 | cm/s² | ¿frena en seco o patina? |
| Velocidad de giro | 1080 | °/s | — |
| Distancia de cámara | 2200 | cm | ¿se ve el tramo que viene y el reflector a tiempo? (D-012; antes 1400) |
| FOV de cámara | 60 | ° | ancho visible ≈ 2540 cm a esa distancia |
| Altura de la cámara sobre el personaje | 120 | cm | — |
| Velocidad del retraso de cámara | 6 | — | mareo vs rigidez |

Los valores derivados salen de v²/2g y 2v/g. Son métricas para diseñar salas con `level-design` y hay que volver a calcularlas si cambian velocidad, salto o gravedad.

## Casos borde
- Una pieza que desaparece bajo los pies (luz u oscuridad): el personaje cae, y es intencional.
- Una pieza que aparece encima del personaje: no está contemplado (ver [fotografía](../fotografia/design.md)).
- Reaparecer durante un rebobinado: el personaje no está en toma, así que no se rebobina.

## Preguntas abiertas
- [ASSUMPTION] Salto de altura fija. Falta definir si se suman salto variable, coyote time y buffer de salto (ver skill `platformer`). Hay que decidirlo con el playtest de T-0002.
- [ASSUMPTION] Por ahora no hay doble salto ni dash. ¿Son mejoras de metroidvania?

## History
- 2026-11-15: nota escrita contra el código del prototipo.
- 2026-11-15: cámara a 2200 cm / FOV 60 tras el primer playtest de T-0008 (solo entraba Floor_A).
