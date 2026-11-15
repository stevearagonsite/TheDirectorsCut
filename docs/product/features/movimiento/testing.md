# Movimiento 2.5D: testing

**Updated:** 2026-11-15 · [Design](design.md) · [Implementation](implementation.md)

## Criterios de aceptación
- [ ] El personaje nunca se sale del plano XZ. *(not proven: no hay test; se verifica en T-0002)*
- [ ] Salta ≈ 190 cm de altura y ≈ 610 cm de distancia a toda velocidad. *(not proven: no hay medición)*
- [ ] Al caer a un pozo reaparece en el último piso `TDC.Safe` en el mismo frame del overlap. *(not proven: T-0002, paso 2)*
- [ ] Sin pisar ningún piso seguro, reaparece en el inicio. *(not proven: no hay test)*

## Pruebas de playtest
1. Correr y saltar el pozo más ancho del greybox a toda velocidad: tiene que pasar con margen.
2. Caer a un pozo después de pisar un piso seguro: reaparece ahí, no en el inicio.

## History
- 2026-11-15: criterios escritos.
