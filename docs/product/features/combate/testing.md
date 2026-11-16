# Combate: testing

**Updated:** 2026-11-15 · [Design](design.md) · [Implementation](implementation.md)

## Criterios de aceptación
- [ ] Un claquetazo derrota al extra común. *(test: playtest `claquetazo_comun` en `-TDCPlaytest`; not proven hasta la corrida de T-0049 con el fix de T-0047)*
- [ ] Un claquetazo aturde al resistente sin derrotarlo. *(test: playtest `claquetazo_resistente`; not proven, ídem)*
- [ ] El claquetazo rebota en el extra de guion. *(test: playtest `claquetazo_guion`; not proven, ídem)*
- [ ] La sombra de guion no aparece derrotada al cargar el build cocinado. *(not proven: T-0044, log del exe con 3 líneas `-> escala del puppet`)*
- [ ] Los extras se ven a tamaño humano y animados desde el primer frame. *(not proven: shot de T-0049)*

## Pruebas de playtest
1. `tdc.ps1 playtest`: el bot da claquetazos en los beats 1, 2 y 4; mirar `[TDC-PLAYTEST]` en el log.
2. A mano: con L2 apagada aparece la sombra; prender L2 la saca de la toma.

## History
- 2026-11-15: criterios escritos.
