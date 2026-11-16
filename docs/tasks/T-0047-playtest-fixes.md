---
id: T-0047
titulo: Playtest: chequeos del claquetazo, capturas sin blur, checkpoints del bot
estado: enviada
tipo: test
rama: t/T-0047-playtest-fixes
creada: 2026-11-15
---

# T-0047 — Salto asistido

## Objetivo
Los 4 puntos del reporte de T-0032: (1) `NotifyPreClap` junta los targets antes del golpe y un claquetazo del bot sin extra al alcance
da FAIL (`claquetazo_alcance`); (2) `ShotHoldUntil` espera 0.35 s después de cada teleport antes de capturar; (3) el bot graba
checkpoints en `best.json`; (4) `playtest.md` aclara `-Run <archivo.json>`. Sin package.

## Pasos
1. `git fetch` y `git checkout t/T-0047-playtest-fixes` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop`.
2. `.\Tools\scripts\tdc.ps1 build`. Si no compila, reportá `fallo` con `archivo:línea`.
3. `.\Tools\scripts\tdc.ps1 playtest -Task T-0047`: tiene que dar **RESULT PASS** y ahora deben aparecer las líneas `claquetazo_*` (pegá todas las `[TDC-PLAYTEST]`).
   Revisá que las capturas post-teleport ya no salgan con motion blur fuerte.
4. `.\Tools\scripts\tdc.ps1 replay -Task T-0047 -Run best.json` (pegá el resumen: ahora tiene que medir drift contra los checkpoints).
5. Commit de evidencia y Reporte. Push y `[TDC-REPORT]`.

## Criterios de aceptación
- [ ] Compila.
- [ ] Playtest y replay en RESULT PASS, con `claquetazo_*` en el log y checkpoints en `best.json`.

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
