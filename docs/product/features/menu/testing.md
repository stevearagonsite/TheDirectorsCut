# Menú inicial y pausa: testing

**Updated:** 2026-11-15 · [Design](design.md) · [Implementation](implementation.md)

## Criterios de aceptación
- [ ] El exe arranca en el menú principal con el arte del título y el log dice `[TDC-MENU] menú principal visible`. *(not proven: T-0035)*
- [ ] Jugar → carga → "¡Acción!" → el director se mueve; antes de Jugar no se mueve. *(not proven: T-0035)*
- [ ] Todo el menú se maneja solo con teclado y solo con gamepad. *(not proven: T-0035)*
- [ ] Esc abre la pausa; Reiniciar vuelve a la toma sin menú; Salir al menú vuelve al menú. *(not proven: T-0035)*
- [ ] `-TDCAutoRun` arranca sin menú. *(not proven: T-0035)*

## Pruebas de playtest
1. Abrir el juego, ir a Controles con flechas, volver con Esc y Jugar con Enter.
2. En la toma, Esc → Reiniciar: el director vuelve al inicio sin pasar por el menú.

## History
- 2026-11-15: criterios escritos.
