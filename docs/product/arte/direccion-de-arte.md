# Dirección de arte

**Actualizado:** 2026-11-15 · [Visión](../vision.md) · [Decisiones](../decisiones.md)

Las reglas de arte que siguen todas las features. La dirección general (un género por zona dentro de un estudio) está en [D-009](../decisiones.md).

## Conceptos de referencia

Los dos son conceptos de juego de Steve.

### Personaje: concepto 1

![Concepto del personaje](referencias/personaje-director-v1.png)

**El diseño del protagonista sale de esta imagen** ([D-011](../decisiones.md)). El escenario de esta misma imagen también sirve como referencia de la zona de cine negro.

- **Silueta:** joven, gorra tipo newsboy, gabardina gris corta, bufanda que flamea al correr, chaleco. Lleva la claqueta en la mano: es su herramienta y su arma (el claquetazo, D-002).
- **Proporciones:** estilizadas, con cabeza grande y cuerpo compacto. Se lee bien chico en pantalla.
- **Acabado:** 3D estilizado de facetas simples (low-poly limpio), con colores planos y poca textura.
- **Paleta del personaje:** grises y oliva apagados, para que no compita con los colores de juego (ver abajo).

El concepto 2 tiene otro personaje (barba, boina, gabardina marrón). **No es el protagonista**; esa imagen vale solo por el nivel y la cámara.

### Nivel y cámara: concepto 2

![Concepto de nivel y cámara](referencias/nivel-camara-v1.png)

**El enfoque de niveles y cámara sale de esta imagen** ([D-012](../decisiones.md)). El escenario es protagonista: la cámara muestra lo suficiente para que el jugador **anticipe** los enemigos y lea un tramo de plataformeo antes de encararlo. La referencia de juego es Rayman Origins, por la legibilidad y la lectura del recorrido, no por su estilo 2D dibujado.

- **Encuadre amplio:** el personaje ocupa poco alto de pantalla y se ve el tramo que viene. Verticalidad con pasarelas, escaleras y andamios.
- **Tres capas de profundidad:**
  1. **Primer plano:** siluetas oscuras de utilería del rodaje (cámaras en trípode, estanterías, barriles). Son decorativas y nunca tapan el plano de juego.
  2. **Plano de juego:** las piezas con las que se interactúa, con color y contraste altos.
  3. **Fondo:** el set pintado del género (skyline, lluvia) y la tramoya del estudio, desaturado.
- **La fuente de luz se ve:** reflectores y focos colgados de la parrilla, y el **cono de luz visible marca el área de efecto** del poder de luz.

### Video de concepto: concepto 3

[concept-art-video-v1.mp4](referencias/concept-art-video-v1.mp4) (10 s, 1280×720, con audio).

Recorre una calle de cine negro y entra a un set de ciencia ficción de los 50 (rollos de película, planetas y engranajes), para cerrar en un escenario con cámaras y grúa. Aporta:

- **Transición entre géneros** dentro del mismo estudio, coherente con D-009.
- **Enemigos:** criaturas de sombra con ojos rojos (zona negro) y robots hechos de rollos de película (zona sci-fi). Hay láseres rojos como trampa.
- **Overlay de visor de cámara** (esquinas, batería, REC) como idea de HUD.
- Mantiene el lenguaje de color: plataformas amarillas con luz, violetas y engranajes cian.

`[ASSUMPTION]` El personaje del video (bufanda roja, boina) se parece más al del concepto 2. Para el protagonista rige el concepto 1 (D-011).

## Hoja de modelo del protagonista

Referencias ortográficas para modelar en Blender (T-0005). Las generé con Gemini a partir del concepto 1; los prompts y el modelo están en [model-sheet/manifest.json](model-sheet/manifest.json). **Son referencia, no assets del juego.**

| Vista | Archivo |
|---|---|
| Hoja completa (frente, perfil y espalda sobre el set) | [director-turnaround-v1.png](model-sheet/director-turnaround-v1.png) |
| Frente, fondo liso | [director-front-v1.png](model-sheet/director-front-v1.png) |
| Perfil izquierdo, fondo liso | [director-side-v1.png](model-sheet/director-side-v1.png) |
| Espalda, fondo liso | [director-back-v1.png](model-sheet/director-back-v1.png) |
| Claqueta (frente, perfil y planta) | [clapperboard-v1.png](model-sheet/clapperboard-v1.png) |

![Vistas del protagonista](model-sheet/director-front-v1.png)

Lo que fija la hoja de modelo:
- La gabardina llega a la rodilla, abierta, con bolsillos con solapa y abertura atrás. Es más larga que la "corta" del primer brief; manda la hoja.
- El chaleco es oliva con botones y bolsillos, sobre una camisa clara.
- La bufanda es gris azulada: da una vuelta al cuello y cae atrás en dos colas.
- Pelo castaño que asoma bajo la gorra. Cara con cejas marcadas, ojos simples y nariz facetada.
- Las botas son marrones, con cordones.
- Pose: entre A y T (brazos a unos 30–40°). Para el blockout, usar la A-pose de la spec de T-0005.

Las vistas sueltas tienen el mismo alto de personaje, pero no comparten escala exacta en píxeles. Al montarlas como planos de referencia en Blender, hay que escalar cada una para que la figura mida 180 cm.

## Lenguaje de color de juego

Los conceptos usan los mismos colores que el greybox del prototipo. Ese lenguaje se mantiene en el arte final:

| Color | Significado | Feature |
|---|---|---|
| Amarillo (bañado por el cono) | Existe solo con luz | [Fotografía](../features/fotografia/design.md) |
| Violeta | Existe solo a oscuras | [Fotografía](../features/fotografia/design.md) |
| Cian neón, tipo holograma | En toma (rebobinable) | [Tiempo](../features/tiempo/design.md) |
| Madera, metal y gris neutros | Fijo | — |

El resto del set evita esos tres colores saturados, para que nunca se confunda decorado con mecánica.

## Implicancias para otras features

- **[Fotografía](../features/fotografia/design.md):** si el cono visible marca el área de efecto, tiene que coincidir con el radio de juego (hoy 500 cm) y respetar las paredes. Esto refuerza la pregunta abierta de oclusión.
- **[Movimiento](../features/movimiento/design.md), cámara:** el encuadre amplio del concepto 2 probablemente pide más distancia de cámara y look-ahead hacia donde corre el personaje (skill `camera-systems`). `[ASSUMPTION]` Se valida en el playtest de T-0002 antes de cambiar números.

## Preguntas abiertas

- ¿Cómo se ven los enemigos y cómo se marca el enemigo "de guion" (D-003) dentro de este lenguaje de color?
- ¿Qué tan "abierto" se ve el estudio fuera del set de cada género (backlot, D-009)?

## Historia

- 2026-11-15: conceptos 1 (personaje) y 2 (nivel y cámara) incorporados; video de concepto (concepto 3).
- 2026-11-15: hoja de modelo del protagonista y de la claqueta, generadas con Gemini.
