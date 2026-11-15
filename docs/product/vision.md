# The Director's Cut — Visión de producto

**Versión:** 0.4 · **Actualizado:** 2026-11-15 · [Features](#feature-index) · [Decisiones](decisiones.md) · [Cambios](#cambios)

> Lo marcado con 🔶 o `[ASSUMPTION]` es una interpretación a validar.

## Una línea

Un metroidvania 2.5D donde sos un director atrapado en su propia película inconclusa: no solo peleás contra el mundo, lo **dirigís**.
Manipulás la luz, rebobinás la toma, cortás escenas y cambiás la escenografía para terminar el rodaje y salir.

| | |
|---|---|
| Género y cámara | Metroidvania 2.5D: personajes y escenarios 3D, cámara lateral, movimiento en un plano con capas de profundidad decorativas |
| Motor | Unreal Engine **5.6** (C++ para sistemas, Blueprints para contenido y tuning). Arte 3D en Blender |
| Plataforma e input | PC, teclado y gamepad `[ASSUMPTION]` |
| Alcance actual | Prototipo: vertical slice greybox |

## Pilar: DIRECCIÓN

Toda decisión de diseño se mide contra una pregunta: **¿esto hace que el jugador dirija la escena?**

- El jugador resuelve problemas **cambiando la puesta en escena**, no (solo) con reflejos.
- Cada poder es una herramienta real de un director de cine y tiene una **lectura visual de cine** (claqueta, foco, moviola, tramoya).
- El mundo es un **estudio de cine**: los niveles son escenas, las zonas son sets de distintas películas, el progreso es el rodaje.

Sub-pilares 🔶: *Legibilidad* (siempre se entiende qué cambió al dirigir) · *Combinación* (los poderes se encadenan) · *Fantasía de autor* (el jugador siente que la película es suya).

## Protagonista

**El director**, atrapado dentro de su propia película inconclusa. Corre, salta y usa las herramientas del oficio. Su objetivo es terminar el rodaje para salir. ([D-001](decisiones.md))

## Loop

```
explorar un set → dirigir la escena (puzzle / enemigo de guion) → abrir paso o vencer jefe
→ nuevo poder, slot o fragmento → volver a sets anteriores con más herramientas
```

- **Segundos:** moverse, saltar, claquetazo, usar un poder (gasta Rollo).
- **Minutos:** resolver una sala; elegir el equipamiento en una zona de descanso.
- **Horas:** completar el set de un género; jefes; backtracking.

## Poderes: tres departamentos

En las zonas de descanso se equipa 1 poder por departamento. Todos gastan Rollo. ([D-005](decisiones.md) a [D-008](decisiones.md))

| Departamento | Poderes |
|---|---|
| **Edición** | Corte: Salto de toma y Tijera · Tiempo |
| **Escenografía** | Cambio de set: mismo nivel en otra variante (cambian las trampas, no los enemigos) |
| **Dirección de fotografía** | Luz |

## Feature index

| Feature | Status | Alcance | Resumen | Docs |
|---|---|---|---|---|
| Movimiento 2.5D | in progress | Prototipo | Correr y saltar en el plano XZ, cámara lateral, reaparecer tras caer | [design](features/movimiento/design.md) |
| Dirección de fotografía (luz) | in progress | Prototipo | Prender/apagar reflectores; piezas que existen solo con luz o solo a oscuras | [design](features/fotografia/design.md) |
| Tiempo | in progress | Prototipo | Mantener para rebobinar lo que está "en toma" | [design](features/tiempo/design.md) |
| Combate | idea | Post-prototipo | Claquetazo y vulnerabilidad por enemigo (D-002, D-003) | — |
| Rollo | idea | Post-prototipo | Barra compartida que gastan los poderes; latas la alargan (D-004) | — |
| Equipamiento y progresión | idea | Post-prototipo | 1 poder por departamento en descanso; jefes dan slots, fragmentos dan skill tree (D-007, D-008) | — |
| Corte | idea | Post-prototipo | Salto de toma (mueve al jugador) y Tijera (corta un fragmento del mundo) (D-006) | — |
| Escenografía | idea | Post-prototipo | Variante del mismo nivel con otras trampas; utilidades como escudo | — |
| Zonas por género | idea | Post-prototipo | Un set por género dentro del estudio; la primera es cine negro (D-009, D-010) | — |

## Dirección de arte

Un género de cine por zona dentro de un mismo estudio; el backlot (andamios, tramoya, focos) es el hub. El género se expresa con iluminación, post-proceso (LUT) y pocos props héroe sobre un kit base compartido. ([D-009](decisiones.md))

En greybox, los colores de lectura son: gris = fija, amarillo = solo con luz, violeta = solo a oscuras, celeste = en toma (rebobinable), verde = salida.

Personaje, cámara, capas y lenguaje de color: [arte/direccion-de-arte.md](arte/direccion-de-arte.md) (D-011, D-012).

## Prototipo (vertical slice gris)

Objetivo: probar que **dirigir se siente bien** antes de producir arte. Tarea: `docs/tasks/T-0002-slice-greybox.md`.

- 1 sala greybox jugable de punta a punta en 2–3 minutos.
- [Movimiento 2.5D](features/movimiento/design.md), [luz](features/fotografia/design.md) y [Tiempo](features/tiempo/design.md), con un puzzle que exija combinar luz y Tiempo. Son de departamentos distintos, así que se pueden equipar juntos.
- 1 asset hecho en Blender exportado a Unreal (prueba del pipeline Mac ⇄ Windows ⇄ Blender ⇄ Unreal).
- **Hecho cuando:** un jugador nuevo resuelve la sala sin explicación en menos de 3 minutos.

Fuera de alcance: combate, Rollo, equipamiento, narrativa, arte final, audio, guardado, mapa y menús.

## Preguntas abiertas

1. ¿Qué es el **ítem roto** cuyos fragmentos dan puntos de skill tree?
2. ¿Cómo se diferencia **Tijera** de **Escenografía**, si las dos alteran el set?
3. ¿Cuál es el **orden de obtención** de los poderes? Solo está implícito que Fotografía va primero (D-010).
4. ¿Latas (Rollo) y fragmentos (skill tree) son dos coleccionables o uno?
5. ¿Qué **géneros** tiene cada zona?
6. **Riesgo:** ¿la luz y el Tiempo se entienden sin explicación? Se averigua con el playtest de T-0002.

## Privacidad del repo

El repo es **público**. Nada de claves, tokens ni credenciales (lo audita gitleaks en cada commit y en CI).
Spoilers, guion y documentos de negocio sensibles van en `private/` (ignorado por git) o fuera del repo.

## Cambios

- **0.4 (2026-11-15):** dirección de arte con los conceptos de personaje y de nivel y cámara ([arte](arte/direccion-de-arte.md)); D-011 y D-012.
- **0.3 (2026-11-15):** índice de features; notas de [movimiento](features/movimiento/design.md), [fotografía](features/fotografia/design.md) y [tiempo](features/tiempo/design.md) escritas contra el código; las decisiones pasan a [decisiones.md](decisiones.md).
- **0.2 (2026-11-15):** protagonista, combate, Rollo, tres departamentos, equipamiento en descanso y tono por género (D-001 a D-010).
- **0.1 (2026-11-15):** primer borrador.
