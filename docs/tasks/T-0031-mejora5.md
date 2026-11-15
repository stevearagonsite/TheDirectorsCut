---
id: T-0031
titulo: Package de mejora 5: toma rebobinada visible + extras con movimiento (sobre mejora 4)
estado: ok
tipo: mixto
rama: t/T-0031-toma-extras
creada: 2026-11-15
---

# T-0031 — Toma visible + extras vivos + package de mejora 5

## Objetivo
Dos cosas rotas que reportó Steve jugando mejora 4:
1. "Los personajes enemigos no tienen animaciones": el Puppet anima siempre (sin optimización por visibilidad) y suma rebote y
   balanceo procedural al caminar, temblor al estar aturdido y respiración quieto. Además `check_anims.py` diagnostica los clips.
2. "La plataforma que vuelve en el tiempo no muestra que quedó afectada": al rebobinar, las piezas en toma que se movieron brillan
   cian con pulso y llevan el flipbook de rebobinado; al soltar queda un resplandor que parpadea y se apaga en ~2.5 s.
Genera `Game\Saved\stevearagonsite_TheDirectorsCut_mejora5.zip`. **No toques** los otros zips.

## Contexto
- La rama sale de `main` con mejora 4 (#31): no hay assets que rehacer, solo C++.
- Si el build falla, reportá `fallo` con `archivo:línea` y no empaquetes.

## Pasos
1. `git fetch` y `git checkout t/T-0031-toma-extras` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop` (cerrá también el build abierto).
2. `.\Tools\scripts\tdc.ps1 build`.
3. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\check_anims.py` y **pegá la salida completa en el Reporte** (por clip:
   si tiene movimiento real y si el esqueleto coincide con el SK del extra).
4. `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0031 -Name 01-mejora5`.
5. `.\Tools\scripts\tdc.ps1 package -Zip stevearagonsite_TheDirectorsCut_mejora5`
6. `.\Tools\scripts\tdc.ps1 smoke -Task T-0031 -Name 02-package -Wait 60 -Zip stevearagonsite_TheDirectorsCut_mejora5`
7. Commit de la evidencia y el Reporte (con el tamaño del zip). Push y `[TDC-REPORT]`.
8. Dejá abierto el build: `.\Tools\scripts\tdc.ps1 smoke -Task T-0031 -Name 03-video -Keep -Zip stevearagonsite_TheDirectorsCut_mejora5`.

## Criterios de aceptación
- [ ] Compila; salida de `check_anims.py` en el Reporte.
- [ ] `_mejora5.zip` de menos de 1 GB; el exe corre a los 60 s, sin `Fatal` ni assets faltantes.

## Permite
- Editar `Tools/unreal/check_anims.py` solo para compatibilidad de API.

## No hacer
- No cambiar C++, mapas ni assets. No pisar los otros zips. No commitear zips ni `Tools/unreal/_diag_*`.

## Evidencia pedida
Salida de `check_anims.py`, `01-mejora5.png`, `02-package.png`, tamaño del zip y extracto del log de `smoke`.

---

## Reporte (lo completa Windows)
- estado: ok
- sha: 4077501 (base) + el commit de este reporte
- resumen: compila sin errores ni warnings (9 acciones, 45 s). `check_anims.py` corrido (salida abajo). Package Development OK
  (BuildCookRun 157.8 s, `BUILD SUCCESSFUL`). `Game\Saved\stevearagonsite_TheDirectorsCut_mejora5.zip` = **346.1 MB**
  (362.881.735 bytes). Smoke: el exe seguía corriendo a los 60 s, carga `/Game/Maps/L_Slice`, sin `Fatal`, sin `Error:` y sin
  assets faltantes. Los otros zips no se tocaron (conservan sus fechas: 18:23, 18:39, 18:56 y 19:32; mejora5 es de 19:53).
- errores: ninguno.
- evidencia: `docs/evidence/T-0031/01-mejora5.png` (editor `-game`), `docs/evidence/T-0031/02-package.png` (build empaquetado).

### Salida de `check_anims.py` (completa, con el fix de API explicado en notas)
```
[TDC] Director/AS_Director_Idle: 8.33s, frames=250, pistas=12, mismo esqueleto=True, movimiento=4.50
[TDC] Director/AS_Director_Walk: 2.57s, frames=77, pistas=12, mismo esqueleto=True, movimiento=29.54
[TDC] Director/AS_Director_Run: 1.17s, frames=35, pistas=12, mismo esqueleto=True, movimiento=104.59
[TDC] Comun/AS_Extra_Comun_Idle: 8.33s, frames=250, pistas=12, mismo esqueleto=False, movimiento=4.55
[TDC] Comun/AS_Extra_Comun_Walk: 1.03s, frames=31, pistas=12, mismo esqueleto=False, movimiento=18.30
[TDC] Resistente/AS_Extra_Resistente_Idle: 8.33s, frames=250, pistas=12, mismo esqueleto=False, movimiento=4.55
[TDC] Resistente/AS_Extra_Resistente_Walk: 1.03s, frames=31, pistas=12, mismo esqueleto=False, movimiento=18.30
[TDC] Sombra/AS_Extra_Sombra_Idle: 8.33s, frames=250, pistas=12, mismo esqueleto=False, movimiento=4.55
[TDC] Sombra/AS_Extra_Sombra_Walk: 1.03s, frames=31, pistas=12, mismo esqueleto=False, movimiento=18.30
```
Lectura por clip:
- Los 9 clips existen y **todos tienen movimiento real** (ninguno está quieto).
- Director: el esqueleto de los 3 clips coincide con el de `SK_Director`.
- Extras (Comun, Resistente, Sombra): **el esqueleto de los 6 clips NO coincide con el del SK** (`mismo esqueleto=False`).
  Es la causa más probable de "los enemigos no tienen animaciones" (hipótesis: no probé reproducir el clip sobre el mesh).
- Los tres extras dan valores idénticos (4.55 / 18.30): parecen compartir el mismo par de clips.

Primera corrida, con el script tal como llegó (sin el fix): movimiento 0.00 / 5.25 / 35.89 en el Director (Idle/Walk/Run),
0.00 en los tres Idle de extras y 0.77 / 0.73 / 0.85 en los Walk; `mismo esqueleto` igual que arriba.

### Extracto del log de `smoke`
```
seguía corriendo a los 60 s: sí
captura: docs/evidence/T-0031/02-package.png
LogWindows: Failed to load 'aqProf.dll' (GetLastError=126)
LogWindows: Failed to load 'VtuneApi.dll' (GetLastError=126)
LogWindows: Failed to load 'VtuneApi32e.dll' (GetLastError=126)
LogWindows: Failed to load 'WinPixGpuCapturer.dll' (GetLastError=126)
LogAudio: Display: No default SoundConcurrencyObject specified (or failed to load).
LogLoad: LoadMap: /Game/Maps/L_Slice?Name=Player
LogLoad: Took 0.087590 seconds to LoadMap(/Game/Maps/L_Slice)
LogGlobalStatus: UEngine::LoadMap Load map complete /Game/Maps/L_Slice
```
Los `Failed to load` son DLLs opcionales de profilers del motor, no assets del juego.

- notas / preguntas:
  - **Cambio en `Tools/unreal/check_anims.py` (compatibilidad de API, dentro de `Permite`)**: en UE 5.8 `unreal.Quat` no tiene
    `get_angular_distance`; se llama `angular_distance`. El `hasattr` daba False y la rotación se ignoraba en silencio, por eso
    los Idle daban 0.00 (un falso "clip quieto"). Ahora usa `angular_distance` y cae a `get_angular_distance` si existiera.
  - `tdc.ps1 python` no muestra los `unreal.log` por consola (el commandlet solo imprime `Display`): la salida se lee en
    `Game\Saved\Logs\TheDirectorsCut.log`.
  - **No verificado visualmente**: las dos capturas son del cuadro inicial (director y extra parados). No muestran el brillo
    cian de la toma rebobinada ni el rebote de los extras al caminar; eso queda para que Steve lo vea en el build abierto
    (paso 8) o para un autorun que rebobine.
  - `tdc.ps1 stop` informó que no había editor abierto. No pude listar procesos por fuera del script (permisos de la sesión);
    el build empaquetado anterior lo cierra `smoke`, que mata lo que corre desde `Game\Saved\PackageTest`.
  - Quedó sin trackear `Tools/unreal/_diag_quat_api.py` (sonda de la API de Quat): la sesión no me dejó borrarlo. No está
    commiteado; se puede borrar a mano.
  - Sugerencia: para que los extras reproduzcan sus clips hay que reimportar o retargetear los `AS_Extra_*` contra el skeleton
    de cada `SK_Extra_*`. Es trabajo de assets en Windows; hace falta una tarea aparte.
