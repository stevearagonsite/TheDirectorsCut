# Combate

**Status:** in progress · **Updated:** 2026-11-15 · [Visión](../../vision.md)

El director no pelea: **dirige**. Su único golpe es el **claquetazo** (F), que saca de la toma a los extras que se meten donde no deben. Los extras que el golpe no resuelve se vencen dirigiendo la escena con los otros poderes, por ejemplo con la luz (D-002, D-003).

## Reglas
1. El claquetazo sale hacia donde mira el director, sobre el plano de juego, y tiene cooldown.
2. **Extra común** (acento rojo): un claquetazo lo derrota ("¡Corten!").
3. **Extra resistente** (acento naranja): un claquetazo lo aturde y lo empuja; no lo derrota.
4. **Extra de guion** (sombra): el claquetazo rebota. Solo existe a oscuras: si la luz lo saca de la toma, queda fuera para siempre.
5. El cambio de presencia de un extra solo cuenta después de que la escena arrancó: al cargar, un extra ausente queda oculto, no derrotado (T-0044).
6. Feedback del golpe: hit-stop (90/60/40 ms según el resultado), squash & stretch del director, la claqueta que cierra, el VFX del impacto, el sonido y un toast en el HUD.

## Números
| Nombre | Valor | Unidad | Qué mirar en playtest |
|---|---|---|---|
| Alcance del golpe (centro) | 90 | cm | ¿se siente "cuerpo a cuerpo" sin tener que pegarse? |
| Radio del golpe | 75 | cm | ¿pega a un extra que está un poco más alto o más bajo? |
| Cooldown | 0.45 | s | ¿se puede spamear? ¿se siente lento? |
| Duración de la animación | 0.3 | s | ¿se lee el golpe? |
| Aturdido (resistente) | 1.5 | s | ¿da tiempo a pasar? |
| Empuje (resistente) | 120 | cm | — |
| Patrulla (por defecto) | 250 / 150 | cm / cm/s | — |
| Hit-stop | 90 / 60 / 40 | ms | derrota / aturde / rebota |

## Casos borde
- Golpe al aire: suena y se ve el impacto, sin hit-stop.
- Varios extras en el alcance: se resuelven todos y el hit-stop toma el resultado más fuerte.
- Extra de guion en una zona que alterna luz: si se prende la luz mientras está presente, queda fuera para siempre.

## Preguntas abiertas
- ¿El claquetazo gasta Rollo? (Rollo está fuera del prototipo, D-004).
- Rediseño visual de los extras (sombras con ojos que brillan): propuesta en [extras-v2](../../arte/extras-v2/notas.txt), pendiente de aprobación.
