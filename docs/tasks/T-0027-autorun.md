---
id: T-0027
titulo: Validar la locomoción del director (autorun con la cámara centrada) sobre mejora 4
estado: enviada
tipo: test
rama: t/T-0029-autorun-camara
creada: 2026-11-15
---

# T-0027 — Locomoción del director en capturas

## Objetivo
Objetivo abierto de Steve: "ver el personaje con los movimientos básicos corriendo en Unreal PC Windows". Confirmar con capturas que el
director **anima** (quieto → corriendo → en el aire) con el contenido de mejora 4. No se empaqueta nada.

## Contexto
- La rama trae el resultado de mejora 4 (T-0026) + una línea de C++: en `-TDCAutoRun` la cámara se centra en el director
  (`TargetOffset = 0`, brazo de 900). El juego normal no cambia.

## Pasos
1. `git fetch` y `git checkout t/T-0029-autorun-camara` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop` (cerrá el build de mejora 4 si está abierto).
2. `.\Tools\scripts\tdc.ps1 build`.
3. `.\Tools\scripts\tdc.ps1 autorun -Task T-0027 -Wait 45` (24 capturas en `docs/evidence/T-0027/autorun/`).
4. Elegí 4 capturas y describilas en el Reporte: **quieto** (antes de 1 s), **corriendo** (zancada: piernas separadas), **en el aire**
   (pose de salto) y **claquetazo** (si alguna lo agarra). Para cada una: número de captura, qué pose tiene el director, si está a
   escala, si mira hacia donde avanza, si patina o atraviesa el piso.
5. Commit solo de la evidencia (las 4 elegidas + las demás) y el Reporte. Push y `[TDC-REPORT]`.
6. Volvé a dejar abierto el build de mejora 4: `.\Tools\scripts\tdc.ps1 smoke -Task T-0027 -Name 99-video -Keep -Zip stevearagonsite_TheDirectorsCut_mejora4`
   (si `_mejora4.zip` no existe, `_mejora2`). No commitees esa captura.

## Criterios de aceptación
- [ ] Compila.
- [ ] Hay capturas con el director en cuadro: quieto, corriendo y en el aire, con poses distintas.

## Permite
- Nada.

## No hacer
- No cambiar C++, mapas ni assets. No empaquetar.

## Evidencia pedida
`docs/evidence/T-0027/autorun/` + la descripción de 4 capturas en el Reporte.

---

## Reporte (lo completa Windows)
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
