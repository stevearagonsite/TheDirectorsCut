# Menú inicial y pausa

**Status:** in progress · **Updated:** 2026-11-15 · [Visión](../../vision.md) · [Implementation](implementation.md) · [Testing](testing.md)

El juego arranca en un menú con el arte del título y no directo en la toma. Desde el juego, Esc abre una pausa.
Es la primera pantalla que ve el jugador, así que tiene que leerse a distancia de sillón y manejarse con teclado,
mouse o gamepad sin cambiar de dispositivo.

## Reglas
1. Al abrir el juego, L_Slice carga en **pausa** debajo del menú principal: Jugar, Controles, Salir.
2. **Jugar** muestra la pantalla de carga ("TAKE 1") unos segundos, despausa, suena "¡Acción!" y el jugador toma el control.
3. **Controles** lista mover, saltar, luces (E), rebobinar (Q, mantener), claquetazo (F) y pausa, con los íconos del HUD.
4. **Esc / Start** durante el juego abre la pausa: Continuar, Reiniciar (vuelve a la toma sin menú) y Salir al menú.
5. Esc / B vuelve atrás: de Controles al menú principal y de la pausa al juego.
6. El foco siempre está en un botón, así que flechas, D-pad o stick y Enter/A alcanzan. El mouse mueve el foco al pasar.
7. Sin menú con `-TDCAutoRun`, `-TDCPlaytest`, `-TDCRecord`, `-TDCReplay` o `-TDCNoMenu`: las demos y los tests arrancan
   directo en la toma.
8. El único color saturado del menú es el amarillo de la luz (botón activo y títulos), como en el título del juego.

## Números
| Nombre | Valor | Unidad | Qué mirar en playtest |
|---|---|---|---|
| Pantalla de carga | 0.8 | s | ¿se lee "TAKE 1" o parece un parpadeo? |
| Tamaño del texto de botones | 34 | pt (base 1080p) | legible a 2–3 m de la pantalla |
