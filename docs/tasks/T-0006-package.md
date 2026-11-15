---
id: T-0006
titulo: Package Win64 para la Epic AI MegaJam (build + zip + prueba del exe)
estado: ok
tipo: build
rama: t/T-0006-package
creada: 2026-11-15
---

# T-0006 — Package Win64 (jam)

## Objetivo
Generar el build empaquetado de Windows que se entrega en la Epic AI MegaJam (cierre 2026-11-15 22:59 ART, subir a las 22:00)
y comprobar que el **zip** arranca y se ve bien: L_Slice con colores de lectura, el director Rodin animado y los extras.
Checklist de entrega: [`docs/jam/entrega.md`](../jam/entrega.md).

## Contexto
- La rama está sobre `main` con T-0012 (director a escala, locomoción, ambientación noir). Este es el package **de prueba**;
  si después entran props o fixes, se repite con un envío corto.
- `tdc.ps1 package`: RunUAT BuildCookRun Win64 **Development** (Shipping oculta los mensajes en pantalla, que hoy son el HUD),
  archiva en `Game\Saved\Package` y arma `Game\Saved\stevearagonsite_TheDirectorsCut.zip`.
- `tdc.ps1 smoke` (nuevo): descomprime el zip en `Game\Saved\PackageTest` (ignorado), arranca `TheDirectorsCut.exe` en ventana,
  saca una captura a `docs/evidence/<Task>/<Name>.png`, imprime un extracto del log del juego y lo cierra (`-Keep` lo deja abierto).
- `DefaultGame.ini` cocina siempre `/Game/Characters` y `/Game/Materials`: el director, sus animaciones y `M_TDCColor` se
  cargan por ruta y sin eso no entrarían al package.

## Pasos
1. La rama se rehízo desde main (force-push): `git fetch` y `git checkout -B t/T-0006-package origin/t/T-0006-package`.
   `.\Tools\scripts\tdc.ps1 stop`: el juego de la tarea anterior tiene Live Coding y bloquea el build.
2. `.\Tools\scripts\tdc.ps1 build` (por si cambió el C++) y `.\Tools\scripts\tdc.ps1 package`.
   Si falla: los primeros errores reales de `Game\Saved\Logs\tdc-package.log` (no la cascada) y reportar `fallo`. Si es del
   script `tdc.ps1`, arreglarlo (permitido) y explicarlo.
3. `.\Tools\scripts\tdc.ps1 smoke -Task T-0006 -Name 01-package`. Revisá la salida:
   - `seguía corriendo a los 40 s: sí`;
   - en el log, `LoadMap` a `/Game/Maps/L_Slice`, sin `Fatal` ni `Failed to load`/`Couldn't find` de `M_TDCColor`, `SK_Director`
     ni `AS_Director_*`;
   - en la captura, piezas de colores (no todo gris), el director Rodin (no el cilindro) y el fondo con cielo.
   Si la captura sale negra porque tomó el primer frame, repetí con `-Wait 60`.
4. Commit del Reporte y la evidencia, push y `[TDC-REPORT]`. **El zip no va al repo.** Anotá el tamaño del zip.
5. Dejá el build empaquetado abierto para Steve, que graba el video desde ahí:
   `.\Tools\scripts\tdc.ps1 smoke -Task T-0006 -Name 02-final -Keep` (esta captura no hace falta commitearla).

## Criterios de aceptación
- [ ] `tdc.ps1 package` termina sin errores y el zip pesa menos de 1 GB (anotar el tamaño).
- [ ] El exe del zip arranca y a los 40 s sigue corriendo; el log carga `L_Slice` y no tiene `Fatal` ni assets faltantes.
- [ ] `docs/evidence/T-0006/01-package.png` muestra colores de lectura y el director.
- [ ] El build empaquetado quedó abierto en la PC.

## Permite
- Editar `Tools/scripts/tdc.ps1` solo para que `package`/`smoke` funcionen.

## No hacer
- No commitear el zip, `Game/Saved/` ni binarios del build, ni `Tools/unreal/_diag_*` (temporales de T-0012: borralos).
- No cambiar C++ ni el mapa: reportar.

## Evidencia pedida
Tamaño del zip, extracto del log del exe que imprime `smoke` (ya redactado) y `01-package.png`.

---

## Reporte (lo completa Windows)
- estado: ok
- sha: bd31aae (base) + commit de este reporte
- resumen: `build` al día (sin cambios de C++). `package` Win64 Development: BUILD SUCCESSFUL en 2m53s (523 paquetes,
  ~152 MiB UAsset en IoStore). **Zip: 319,2 MB** (`Game\Saved\stevearagonsite_TheDirectorsCut.zip`, < 1 GB). `smoke`: el exe
  sigue corriendo a los 40 s y a los 60 s; el log hace `LoadMap /Game/Maps/L_Slice` sin `Fatal` ni assets faltantes.
  Captura: el director Rodin (no el cilindro), una pieza roja, plataformas y el telón pintado de cielo noir.
- errores: primer intento de `package` falló: `Invalid configuration '$Config'` — PowerShell no expande `$Config` en un argumento
  sin comillas que empieza con `-`. Arreglado en `Tools/scripts/tdc.ps1` (`-clientconfig="$Config"`). El log del package solo
  tiene el warning de compilador VS 14.52 no preferido.
- evidencia: `docs/evidence/T-0006/01-package.png`. Extracto del log del exe (smoke, `-Wait 60`):
  ```
  LogWindows: Failed to load 'aqProf.dll' / 'VtuneApi.dll' / 'VtuneApi32e.dll' / 'WinPixGpuCapturer.dll' (GetLastError=126)  ← profilers opcionales, normal
  LogAudio: Display: No default SoundConcurrencyObject specified (or failed to load).
  LogLoad: LoadMap: /Game/Maps/L_Slice?Name=Player
  LogLoad: Took 0.051819 seconds to LoadMap(/Game/Maps/L_Slice)
  LogGlobalStatus: UEngine::LoadMap Load map complete /Game/Maps/L_Slice
  ```
- notas / preguntas:
  - La primera captura (`-Wait 40`) salió solo con un gradiente azul: `HighResShot` va por `-ExecCmds` y se dispara en el
    frame ~6, antes de que la escena termine de renderizar. Con `-Wait 60` salió bien, pero `-Wait` no cambia cuándo se toma
    la foto: es una carrera. Sugerencia para la Mac: en `smoke`, sacar la captura después del `Start-Sleep` (p. ej. captura
    de la ventana) en vez de por `-ExecCmds`.
  - En la captura se ve una sola pieza de color (roja); el resto del encuadre son plataformas grises y el telón.
  - `Tools/unreal/_diag_*` (temporales de T-0012) no se pudieron borrar desde la sesión sin interfaz (permiso de borrado
    denegado); quedan sin trackear y no se commitearon. Borrarlos a mano.
  - Build empaquetado dejado abierto con `smoke -Task T-0006 -Name 02-final -Keep` (captura no commiteada).
