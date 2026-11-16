---
id: T-0045
titulo: Claquetazo que se lee en el cuerpo (squash & stretch + cierre de la claqueta) y encuadre del autorun
estado: enviada
tipo: test
rama: t/T-0045-claquetazo-feel
creada: 2026-11-15
---

# T-0045 — Feel del claquetazo

## Objetivo
En T-0027 el claquetazo "no tenía pose propia": el director quedaba igual que quieto. Ahora, durante el golpe (300 ms), el mesh hace
squash & stretch (±10 %) y la claqueta en la mano gira −35° y vuelve. Solo C++ de `ATDCCharacter`. La rama contiene T-0043 (brazo del
autorun 1300, para que la cámara no atraviese la utilería) y T-0040/T-0038 (menú, HUD, cámara). Sin package.

## Pasos
1. `git fetch` y `git checkout t/T-0045-claquetazo-feel` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop`.
2. `.\Tools\scripts\tdc.ps1 build`. Si no compila, reportá `fallo` con `archivo:línea`.
3. `.\Tools\scripts\tdc.ps1 autorun -Task T-0045 -Wait 45`. Describí: (a) ¿en las capturas de claquetazo el director se ve **distinto** que
   quieto (más ancho y bajo, claqueta girada)? (b) ¿la cámara ya no atraviesa la utilería de primer plano (sin capturas tapadas)?
4. Commit de evidencia y Reporte. Push y `[TDC-REPORT]`.

## Criterios de aceptación
- [ ] Compila.
- [ ] Al menos una captura de claquetazo con el director visiblemente en squash; capturas sin utilería tapando.

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
