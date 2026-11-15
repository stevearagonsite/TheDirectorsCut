---
id: T-0006
titulo: Primer package Win64 para la Epic AI MegaJam
estado: nueva
tipo: build
rama: t/T-0006-package
creada: 2026-11-15
---

# T-0006 — Primer package Win64 (jam)

## Objetivo
Generar el build empaquetado de Windows que se entrega en la Epic AI MegaJam (cierre 2026-11-15 22:59 ART) y comprobar
que arranca. Es el **primer package de prueba** (meta: 19:00 ART), con la sala greybox y el personaje cápsula.
Depende de T-0001 (compila) y T-0002 (`/Game/Maps/L_Slice` existe; sin el mapa no hay nada que cocinar).

## Contexto
- `tdc.ps1 package` (nuevo en esta rama): RunUAT BuildCookRun Win64, **Development** por defecto (Shipping oculta los
  mensajes en pantalla que hoy son el HUD), archiva en `Game\Saved\Package` y arma `Game\Saved\stevearagonsite_TheDirectorsCut.zip`.
- `DefaultGame.ini` cocina siempre `/Game/Characters` (el GameMode carga `BP_Director` por ruta cuando exista).
- Lo ejecuta un agente sin interfaz: no puede jugar. Solo arranca el exe y lee el log.

## Pasos
1. `git merge origin/main` si la rama quedó atrás (tiene que incluir T-0001 y T-0002).
2. `.\Tools\scripts\tdc.ps1 build` (por si cambió el C++) y `.\Tools\scripts\tdc.ps1 package`.
3. Si falla: extracto de errores de `Game\Saved\Logs\tdc-package.log` (primeros errores reales, no la cascada) y reportar. Si el
   error es del script `tdc.ps1`, arreglarlo (permitido) y explicarlo.
4. Si sale: descomprimir el zip en una carpeta temporal fuera del repo, arrancar `TheDirectorsCut.exe -windowed -ResX=1280 -ResY=720`
   unos 30 s y cerrarlo. Revisar su log (`<carpeta>\TheDirectorsCut\Saved\Logs\TheDirectorsCut.log`): mapa cargado, GameMode,
   errores o `Fatal`. Una captura 1280x720 si se puede (`-ExecCmds="HighResShot 1280x720"`) en `docs/evidence/T-0006/01-package.png`.
5. Commit solo de `docs/tasks/T-0006-package.md` (Reporte) y la evidencia. **El zip no va al repo.**

## Criterios de aceptación
- [ ] `tdc.ps1 package` termina sin errores y el zip pesa menos de 1 GB (anotar el tamaño).
- [ ] El exe arranca, carga `L_Slice` y no tiene `Fatal` ni errores de assets faltantes en el log.

## Permite
- Editar `Tools/scripts/tdc.ps1` solo para que `package` funcione.

## No hacer
- No commitear el zip, `Game/Saved/` ni binarios del build.
- No cambiar C++ ni el mapa: reportar.

## Evidencia pedida
Tamaño del zip, extracto del log del exe (rutas con `%USERPROFILE%`) y la captura si salió.

---

## Reporte (lo completa Windows)
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
