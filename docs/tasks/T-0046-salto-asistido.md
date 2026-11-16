---
id: T-0046
titulo: Salto asistido (coyote time + jump buffer de 120 ms), validado con el playtest
estado: enviada
tipo: test
rama: t/T-0046-salto-asistido
creada: 2026-11-15
---

# T-0046 — Salto asistido

## Objetivo
Coyote time (saltar hasta 120 ms después de salir del borde) y jump buffer (si apretás saltar hasta 120 ms antes de aterrizar,
salta al tocar el piso). Editables en `TDC|Movement`. La rama trae T-0045 (feel del claquetazo), T-0043/T-0040 (menú, HUD, cámara) y
main con el playtest (T-0032). Sin package.

## Pasos
1. `git fetch` y `git checkout t/T-0046-salto-asistido` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop`.
2. `.\Tools\scripts\tdc.ps1 build`. Si no compila, reportá `fallo` con `archivo:línea`.
3. `.\Tools\scripts\tdc.ps1 playtest -Task T-0046`: tiene que seguir dando **RESULT PASS** (pegá las líneas `[TDC-PLAYTEST]`).
4. `.\Tools\scripts\tdc.ps1 replay -Task T-0046 -Run best.json` (pegá el resumen).
5. Commit de evidencia y Reporte. Push y `[TDC-REPORT]`.

## Criterios de aceptación
- [ ] Compila.
- [ ] Playtest y replay en RESULT PASS.

## Permite
- Nada.

## No hacer
- No cambiar C++, mapas ni assets. No empaquetar.

---

## Reporte (lo completa Windows)
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
