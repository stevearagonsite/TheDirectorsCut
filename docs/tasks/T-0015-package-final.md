---
id: T-0015
titulo: Package FINAL de la jam (main con T-0012 a T-0014 + claquetazo de 300 ms)
estado: ok
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
- estado: ok
- sha: build hecho sobre 8148376 (incluye 50ec38c, el claquetazo de 300 ms; es ba6a3aa rebasado sobre main)
- resumen: `tdc.ps1 build` compila sin errores ni warnings (9 acciones, 25 s). `tdc.ps1 package` termina con
  `BUILD SUCCESSFUL` (cook: 0 errores, 1 warning de DDC lento, inofensivo). **Zip: 328 MB**, en
  `Game\Saved\stevearagonsite_TheDirectorsCut.zip` (no va al repo). `smoke -Wait 60`: el exe del zip arranca, sigue
  corriendo a los 60 s y carga `/Game/Maps/L_Slice`; log sin `Fatal`, `Error:`, `Failed to load` de assets ni `Couldn't find`.
  La captura muestra el director, el telón con skyline, los props (reflector con trípode, cilindro rojo, segundo trípode)
  y el HUD de controles arriba a la izquierda, legible. El build final quedó abierto con `smoke -Name 02-video -Keep`.
- errores: ninguno de build, cook ni carga. Extracto del log de `smoke` (igual en las cuatro corridas):
  ```
  LogWindows: Failed to load 'aqProf.dll' (GetLastError=126)
  LogWindows: Failed to load 'VtuneApi.dll' (GetLastError=126)
  LogWindows: Failed to load 'VtuneApi32e.dll' (GetLastError=126)
  LogWindows: Failed to load 'WinPixGpuCapturer.dll' (GetLastError=126)
  LogAudio: Display: No default SoundConcurrencyObject specified (or failed to load).
  LogLoad: LoadMap: /Game/Maps/L_Slice?Name=Player
  LogLoad: Took 0.039526 seconds to LoadMap(/Game/Maps/L_Slice)
  LogGlobalStatus: UEngine::LoadMap Load map complete /Game/Maps/L_Slice
  ```
  Los cuatro `Failed to load` son DLL de profilers opcionales de un build Development (no son assets).
- evidencia: `docs/evidence/T-0015/01-final.png` (con HUD) y `02-video.png` (el build que quedó abierto). La primera
  corrida, con `HighResShot`, mostraba la misma escena sin HUD y más oscura; esa captura se sobrescribió al repetir.
- notas / preguntas:
  - **Ojo, se ve en el build de entrega**: arriba del HUD aparece en rojo `LIGHTING NEEDS TO BE REBUILT (6 unbuilt
    objects)` y debajo `Run console command 'DumpUnbuiltLightInteractions'...`, más una línea gris
    `'DisableAllScreenMessages' to suppress`. Es un mensaje del motor en builds Development (ver `01-final.png`). No lo
    toqué (No hacer: mapas/assets). Opciones para la Mac: rebuild de luces de `L_Slice` (necesita otra tarea con lock del
    mapa) o volver las 6 luces/objetos movibles. Va a salir en el video y en lo que vea el jurado.
  - **Cambio en `Tools/scripts/tdc.ps1`** (Permite): `smoke` ahora captura con `Screenshot showui` en vez de
    `HighResShot 1280x720`. `HighResShot` no incluye los `AddOnScreenDebugMessage`, así que el HUD nunca salía en la
    captura (mismo problema que en T-0014). La ventana ya es de 1280x720, la captura sale del mismo tamaño.
  - La captura con UI se toma en el primer frame: la exposición todavía no se adaptó y se ve más clara que el juego
    asentado (la de `HighResShot` salía más oscura).
  - Quedó sin commitear `docs/evidence/T-0015/01-final-ui.png` (duplicado de `01-final.png`; el worker no me dejó
    borrarlo). Se puede borrar a mano.
  - Una de las corridas con `-Keep` terminó antes de los 60 s: no fue un crash. El log muestra reflectores prendidos y
    apagados (alguien jugando en la PC) y un cierre de ventana normal (`RequestExit(0, UGameEngine::Tick.ViewportClosed)`).
    Lo volví a abrir con `smoke -Name 02-video -Keep` y a los 60 s seguía corriendo.
  - En esta PC `tdc.ps1` hay que llamarlo con `powershell -ExecutionPolicy Bypass -File Tools\scripts\tdc.ps1 ...`
    (la política de ejecución bloquea los scripts); no cambié la política del sistema.
  - `package` tardó 2 min (cook incremental). El zip pesa 328 MB contra 319 MB de T-0006.
