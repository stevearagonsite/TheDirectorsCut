# Dirección de fotografía (luz)

**Status:** in progress · **Updated:** 2026-11-15 · [Visión](../../vision.md) · [Implementation](implementation.md) · [Testing](testing.md)

El director prende y apaga los reflectores del set, y la luz decide qué existe: hay piezas que solo están con luz y otras que solo están a oscuras. Es el primer departamento (D-010) y el que enseña a "dirigir" (pilar Dirección).

En el prototipo es un solo poder: prender/apagar. Las ideas de spot y de luz guía que avanza sin detenerse quedan para después.

## Reglas
1. Al usar el poder, el director prende o apaga el reflector **más cercano** dentro de su alcance, esté prendido o apagado. Si no hay ninguno al alcance, no pasa nada.
2. Una pieza está iluminada si **algún** reflector prendido está a una distancia de su caja menor o igual que el radio de ese reflector. Alcanza con que la luz toque el borde.
3. Según su presencia, una pieza puede existir siempre, solo con luz o solo a oscuras. Si no existe, es invisible y no tiene colisión.
4. Cada vez que cambia una luz, todas las piezas reactivas se reevalúan en el mismo frame, y la física se despierta para que caiga lo que estaba apoyado en una pieza que desapareció.
5. La luz no se bloquea con paredes: solo cuenta la distancia.

## Números
| Nombre | Valor | Unidad | Qué mirar en playtest |
|---|---|---|---|
| Alcance del director hasta un reflector | 800 | cm | ¿se entiende cuál reflector se va a tocar? |
| Radio de un reflector (por defecto) | 500 | cm (mín. 50) | ¿coincide el área visible iluminada con lo que reacciona? |
| Intensidad del reflector | 40 | cd | ¿se lee luz vs sombra en greybox? |
| Reflector prendido al inicio | sí | — | — |
| Costo en Rollo | — | — | sin definir (Rollo fuera del prototipo) |

## Casos borde
- Dos reflectores a la misma distancia: gana el último registrado. No es predecible para el jugador.
- Una pieza que aparece encima del personaje: no está manejado y puede trabarlo dentro de la colisión.
- Cambiar la luz mientras se rebobina: está permitido. Las piezas en toma se rebobinan y además se reevalúan por luz.

## Preguntas abiertas
- [ASSUMPTION] Falta un resaltado del reflector que se va a tocar (existe `FindNearestLight` para la UI, pero no hay UI).
- [ASSUMPTION] La luz atraviesa paredes. ¿Hay que sumar oclusión para que lo que se ve iluminado coincida con lo que reacciona?
- ¿Cómo convive "prender/apagar" con el spot y la luz guía? ¿Son poderes distintos del mismo departamento, a elegir en el descanso?

## History
- 2026-11-15: nota escrita contra el código del prototipo.
