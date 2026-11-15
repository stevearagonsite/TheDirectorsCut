# Dirección de fotografía (luz): testing

**Updated:** 2026-11-15 · [Design](design.md) · [Implementation](implementation.md)

## Criterios de aceptación
- [ ] `IsLit`: sin luces, o con una luz apagada, no hay iluminación; una luz prendida dentro del radio, o que toca el borde de la caja, ilumina; fuera del radio, no. *(test escrito `TDC.Stage.Lighting.IsLit`; not proven: nunca se corrió, depende de T-0001)*
- [ ] `IsPresent`: las 6 combinaciones de presencia y luz. *(test escrito `TDC.Stage.Lighting.Presence`; not proven: nunca se corrió)*
- [ ] Una pieza "solo con luz" desaparece y pierde la colisión en el mismo frame en que se apaga su reflector. *(not proven: es lógica de componente, los tests cubren solo las funciones puras; se verifica en T-0002, beats 1 y 2)*
- [ ] Lo apoyado sobre una pieza que desaparece cae. *(not proven: T-0002)*
- [ ] El director solo alcanza reflectores a 800 cm o menos. *(not proven: no hay test)*

## Pruebas de playtest
1. Beat 1 de `L_Slice`: prender L1 hace aparecer la plataforma amarilla sobre el pozo.
2. Beat 2: apagar L2 abre la pared amarilla y hace aparecer la plataforma violeta.
3. Prender L2 con el personaje parado sobre la plataforma violeta: cae.

## History
- 2026-11-15: criterios escritos.
