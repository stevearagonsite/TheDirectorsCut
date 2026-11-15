---
id: T-0015
titulo: Package FINAL de la jam (main con T-0012 a T-0014 + claquetazo de 300 ms)
estado: enviada
tipo: build
rama: t/T-0015-package-final
creada: 2026-11-15
---

# T-0015 — Package final (jam)

## Objetivo
El zip que se sube a itch.io: `main` con el director a escala, la ambientación noir, los props y el HUD de controles.
Entrega a las 22:00 ART; freeze de contenido a las 20:00. Checklist: [`docs/jam/entrega.md`](../jam/entrega.md).

## Contexto
- Incluye ba6a3aa (T-0016, claquetazo de 300 ms: C++ chico de TDCCharacter). Si **no compila**, reportá `fallo` con
  `archivo:línea` y no empaquetes: la Mac reenvía sin ese commit.
- Mismo procedimiento que T-0006 (package de prueba, ok: zip de 319 MB, el exe arranca y carga L_Slice).
- `smoke` saca la captura con `HighResShot` vía `-ExecCmds`: si sale solo el gradiente azul, repetí con `-Wait 60`.

## Pasos
1. `git fetch` y `git checkout t/T-0015-package-final` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop`. Cerrá también el
   build empaquetado de T-0006 si sigue abierto (lo cierra `smoke` al empezar).
2. `.\Tools\scripts\tdc.ps1 build` y `.\Tools\scripts\tdc.ps1 package`. Si falla: primeros errores reales de
   `Game\Saved\Logs\tdc-package.log` y reportá `fallo`.
3. `.\Tools\scripts\tdc.ps1 smoke -Task T-0015 -Name 01-final -Wait 60`. Verificá: sigue corriendo; el log carga `/Game/Maps/L_Slice` sin
   `Fatal` ni `Failed to load`/`Couldn't find`; la captura muestra el director, el telón con skyline, props y el HUD de controles
   arriba a la izquierda.
4. Commit del Reporte (con el tamaño del zip) y la evidencia, push y `[TDC-REPORT]`. **El zip no va al repo.**
5. Dejá el build final abierto para que Steve grabe el video: `.\Tools\scripts\tdc.ps1 smoke -Task T-0015 -Name 02-video -Keep`.
6. En el Reporte anotá dónde quedó el zip para subir: `Game\Saved\stevearagonsite_TheDirectorsCut.zip`.

## Criterios de aceptación
- [ ] `tdc.ps1 package` sin errores, zip de menos de 1 GB.
- [ ] El exe del zip arranca y sigue corriendo a los 60 s; log sin `Fatal` ni assets faltantes.
- [ ] `docs/evidence/T-0015/01-final.png` muestra director, ambientación y HUD.
- [ ] El build final quedó abierto en la PC.

## Permite
- Editar `Tools/scripts/tdc.ps1` solo para que `package`/`smoke` funcionen.

## No hacer
- No cambiar C++, mapas ni assets: es el build de entrega. Reportá cualquier problema.
- No commitear el zip, `Game/Saved/` ni `Tools/unreal/_diag_*`.

## Evidencia pedida
Tamaño del zip, extracto del log que imprime `smoke` y `01-final.png`.

---

## Reporte (lo completa Windows)
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
