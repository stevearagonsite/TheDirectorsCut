# Tiempo

**Status:** in progress · **Updated:** 2026-11-15 · [Visión](../../vision.md) · [Implementation](implementation.md) · [Testing](testing.md)

Es el poder de Edición que rebobina la toma. Mientras se mantiene el botón, todo lo que está "en toma" vuelve atrás por el camino que hizo: un puente que se cayó vuelve a su lugar, una caja vuelve a donde estaba. Al soltar, la toma sigue desde ahí. El jugador no se rebobina, porque es el director, no un actor de la toma.

## Reglas
1. Solo afecta a piezas marcadas "en toma" (celestes en greybox).
2. Una pieza en toma graba su posición solo cuando se mueve, así los períodos quietos no gastan memoria.
3. Mientras se mantiene el poder, todas las piezas en toma retroceden a la velocidad de rebobinado. La física se apaga durante el rebobinado.
4. El rebobinado se detiene en el primer frame grabado (el estado de partida de la toma). Nunca va más atrás.
5. Al soltar, la pieza retoma la física con la velocidad que tenía en ese instante.
6. Si la toma grabó más de la memoria disponible, se pierde lo más viejo.

## Números
| Nombre | Valor | Unidad | Qué mirar en playtest |
|---|---|---|---|
| Memoria de movimiento | 15 | s | ¿alcanza para el puzzle más largo? |
| Muestras por segundo | 30 | Hz | ¿el rebobinado se ve fluido? |
| Velocidad de rebobinado | 1.5 | × | ¿se puede soltar en el momento justo? |
| Tolerancia para "se movió" | 0.1 | (Transform.Equals) | — |
| Costo en Rollo | — | — | sin definir (Rollo fuera del prototipo) |

## Casos borde
- Soltar a mitad de una caída: la pieza sigue cayendo con su velocidad de ese momento.
- Rebobinar algo que nunca se movió: no pasa nada.
- Una pieza rebobinada a un lugar donde está el personaje: no está manejado (puede empujarlo o trabarlo).
- Una pieza en toma que además depende de la luz: se rebobina aunque esté oculta.

## Preguntas abiertas
- [ASSUMPTION] Por ahora solo hay rebobinado. La visión decía "rebobinar y adelantar": ¿adelantar entra (puertas que se adelantan hasta abrirse) o queda afuera?
- ¿Hay un límite de rebobinado por uso, más allá de la memoria? Depende del costo en Rollo (D-004).

## History
- 2026-11-15: nota escrita contra el código del prototipo.
