# Tiempo: testing

**Updated:** 2026-11-15 · [Design](design.md) · [Implementation](implementation.md)

## Criterios de aceptación
- [ ] El buffer respeta la capacidad, devuelve frames en orden inverso (LIFO), pisa los más viejos al llenarse y falla al vaciarse. *(test escrito `TDC.Stage.Time.History`; not proven: nunca se corrió, depende de T-0001)*
- [ ] El rebobinado se detiene en el frame de partida y la pieza queda como al inicio de la toma, con la física apagada. *(not proven: no hay test del componente; T-0002, beat 3)*
- [ ] Al soltar a mitad de una caída, la pieza sigue cayendo con su velocidad de ese momento. *(not proven: T-0002, paso 2)*
- [ ] El personaje no se rebobina. *(not proven: T-0002)*

## Pruebas de playtest
1. Beat 3 de `L_Slice`: dejar caer el puente, mantener Q hasta que vuelva y cruzarlo.
2. Soltar Q a mitad del rebobinado: el puente vuelve a caer desde ahí.

## History
- 2026-11-15: criterios escritos.
