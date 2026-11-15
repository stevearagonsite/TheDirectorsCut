# The Director's Cut — Visión de producto

> Estado: **borrador v0.1** (2026-11-15). Lo marcado con 🔶 es una interpretación a validar.

## Una línea

Un metroidvania 2.5D donde sos el director de la película: no peleás contra el mundo, lo **dirigís**.
Prendés y apagás luces, rebobinás y adelantás la toma, cortás escenas y cambiás la escenografía para avanzar.

## Pilar: DIRECCIÓN

Toda decisión de diseño se mide contra una pregunta: **¿esto hace que el jugador dirija la escena?**

- El jugador resuelve problemas **cambiando la puesta en escena**, no (solo) con reflejos.
- Cada poder es una herramienta real de un director de cine y tiene una **lectura visual de cine** (claqueta, foco, moviola, tramoya).
- El mundo es un **set de filmación**: los niveles son escenas, las zonas son sets, el progreso es el rodaje.

Sub-pilares 🔶: *Legibilidad* (siempre se entiende qué cambió al dirigir) · *Combinación* (los poderes se encadenan) · *Fantasía de autor* (el jugador siente que la película es suya).

## Formato

- **2.5D**: personajes y escenarios 3D, cámara lateral, movimiento restringido a un plano horizontal (con capas de profundidad decorativas).
- **Metroidvania**: mundo interconectado, backtracking, progreso gateado por poderes.
- Motor: **Unreal Engine 5** (C++ para sistemas + Blueprints para contenido/tuning). Arte 3D en **Blender**.

## Poderes del director (verbos)

| Poder | Fantasía | Mecánica base 🔶 | Gating metroidvania 🔶 |
|---|---|---|---|
| **Luces** (Reflector) | "¡Luces!" | Prender/apagar fuentes de luz del set. Ciertos objetos solo existen o solo son tangibles en luz o en sombra. | Plataformas de luz, enemigos que solo se mueven a oscuras |
| **Tiempo** (Moviola) | "Rebobinar la toma" | Rebobinar/adelantar el tiempo de objetos "en toma" (no del jugador). | Puentes caídos que se rebobinan, puertas que se adelantan hasta abrirse |
| **Corte** (Tijera / Claqueta) | "¡Corten!" | Marcar una toma y **cortar** a ella: salto instantáneo entre dos encuadres / eliminar un fragmento de escena. | Atravesar huecos, saltar segmentos peligrosos |
| **Escenografía** (Tramoya) | "Cambio de set" | Intercambiar piezas del set entre variantes (ej. día/noche, ruinas/intacto). | Rutas alternativas en la misma sala |

Orden de obtención propuesto 🔶: Luces → Tiempo → Escenografía → Corte (el corte rompe el movimiento, conviene al final).

## Prototipo (vertical slice gris)

Objetivo: probar que **dirigir se siente bien** antes de producir arte.

- 1 sala greybox (set de filmación) jugable de punta a punta en 2–3 minutos.
- Personaje 3D placeholder: correr, saltar, movimiento en plano 2.5D, cámara lateral.
- Poderes: **Luces** y **Tiempo** (los dos que más contrastan). Un puzzle que exija combinarlos.
- 1 asset hecho en Blender exportado a Unreal (prueba del pipeline Mac ⇄ Windows ⇄ Blender ⇄ Unreal).
- Criterio de éxito: un jugador nuevo entiende y resuelve el puzzle sin explicación.

## Fuera de alcance del prototipo

Combate, narrativa, arte final, audio, guardado, mapa, menús.

## Preguntas abiertas

1. ¿Quién es el protagonista? ¿El director dentro de su película, un actor que dirige, la cámara misma?
2. ¿Hay combate? ¿Los poderes son también la forma de enfrentar enemigos?
3. ¿Los poderes tienen costo (película, rollo, presupuesto)?
4. Tono visual: ¿cine negro, cine mudo, ciencia ficción de los 50, varios géneros por zona?
5. ¿"Corte" mueve al jugador, mueve el mundo, o ambas?

## Privacidad del repo

El repo es **público**. Nada de claves, tokens ni credenciales (lo audita gitleaks en cada commit y en CI).
Spoilers, guion y documentos de negocio sensibles van en `private/` (ignorado por git) o fuera del repo.
