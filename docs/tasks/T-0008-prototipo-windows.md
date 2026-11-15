---
id: T-0008
titulo: Prototipo jugable corriendo en la PC Windows (build + L_Slice + juego abierto)
estado: mergeada
tipo: mixto
rama: t/T-0008-prototipo-windows
creada: 2026-11-15
---

# T-0008 — Prototipo jugable corriendo en Windows

## Objetivo
Que Steve vea el prototipo **corriendo en la pantalla de la PC**: el C++ compila, existe `/Game/Maps/L_Slice` y el juego queda
abierto en una ventana con el personaje en la sala greybox. Reemplaza a T-0001 y T-0002 (nunca reportadas): esta tarea junta lo
mínimo de ambas.

## Contexto
- Proyecto en UE **5.8** (D-013). El `.uproject` todavía dice `"EngineAssociation": "5.6"`.
- C++ en `Game/Source/TheDirectorsCut/` (personaje 2.5D, luces del set, rebobinado). Nunca compiló.
- `Tools/unreal/build_slice.py` arma el mapa (docstring: 4 beats). `DefaultEngine.ini` ya lo pone como mapa de inicio y
  `TDCGameMode` como GameMode por defecto.
- Controles: A/D mover · Espacio saltar · E prende/apaga el reflector cercano · Q (mantener) rebobina.
- Si `tdc.local.ps1`, gitleaks, etc. ya están de una sesión anterior, no los repitas.

## Pasos
0. **Reenvío 6.** test-e4 cerró los `UnrealEditor`/`LiveCodingConsole` que bloqueaban el build por Live Coding. Si vuelve a
   pasar, reportá `bloqueado` sin reintentar. (Reenvío 5:) Permisos del worker restablecidos: `tdc.ps1` en todas sus formas (no `Start-Process` ni `UnrealEditor` directo:
   para eso están `tdc.ps1 play`, `editor`, `shot` y `python -InEditor`). test-e4 instaló el .NET Framework 4.8 SDK (VS 2026 Community y BuildTools), gitleaks y la identidad de git. `gh` puede seguir sin autenticar: si el push falla, reportá igual por VirtualKvm.
   En el clon: `git restore --staged --worktree Game/TheDirectorsCut.uproject docs/tasks/T-0008-prototipo-windows.md` y `git pull`.
   El stash de T-0001 dejalo como está.
1. `.\Tools\scripts\tdc.ps1 doctor` (UE 5.8 tiene que figurar `True`).
2. `Game/TheDirectorsCut.uproject`: `"EngineAssociation": "5.8"` (o el identificador que use tu instalación).
3. `.\Tools\scripts\tdc.ps1 projectfiles` y `.\Tools\scripts\tdc.ps1 build`.
   - Si hay errores de C++: **pará**, reportá `estado: fallo` con `archivo:línea` y el mensaje exacto de los primeros errores
     reales (no la cascada). La Mac los arregla y reenvía.
4. `.\Tools\scripts\tdc.ps1 test` (3 tests `TDC.Stage.*`). Si fallan, anotalo y seguí.
5. Generar el mapa: `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\build_slice.py` → tiene que loguear
   `[TDC] L_Slice construido y guardado.` Si el commandlet no puede usar los subsistemas del editor, repetí con `-InEditor`
   (abre el editor sin ventana y lo cierra al terminar). Si falla por API de Python de 5.8, arreglá `build_slice.py` sin
   cambiar el layout.
6. Captura: `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0008 -Name 01-juego`.
   Revisala: tiene que verse el personaje (cilindro rojo) y las plataformas. Si está todo negro o quemado, ajustá en `build_slice.py`
   la intensidad de `Ambient_Sun` (o sumá un SkyLight), regenerá el mapa (paso 5) y anotá los valores.
7. **Dejá el juego abierto en la pantalla** para Steve (no lo cierres): `.\Tools\scripts\tdc.ps1 play`.
   Confirmá en el reporte que la ventana quedó abierta.
8. Commit de `Game/Content/Maps/L_Slice.umap` (LFS), el `.uproject`, la evidencia y el Reporte. Push y `[TDC-REPORT]`.
   Si el commit falla (hook, identidad), igual dejá el juego abierto y reportá `parcial` con el error.

## Criterios de aceptación
- [ ] `TheDirectorsCutEditor Win64 Development` compila sin errores (listar warnings de nuestro código si hay).
- [ ] Tests `TDC.Stage.*`: resultado anotado.
- [ ] `Game/Content/Maps/L_Slice.umap` commiteado y abre como mapa de inicio.
- [ ] `docs/evidence/T-0008/01-juego.png` muestra el personaje y la sala.
- [ ] El juego quedó corriendo en una ventana en la PC.

## Permite
- test-e4 instaló en la PC (2026-11-15) el .NET Framework 4.8 SDK, gitleaks y la identidad global de git.
  Si `doctor` o el build siguen marcando que falta algo, reportá `bloqueado` con el detalle; no instales nada.
- Editar `Game/TheDirectorsCut.uproject` (solo `EngineAssociation`), `Tools/scripts/tdc.ps1` y `Tools/unreal/build_slice.py`
  (solo compatibilidad de API o luz ambiente, sin cambiar el layout).

## No hacer
- No corregir C++ en `Game/Source/`: reportarlo.
- No commitear `tdc.local.ps1`, `Binaries/`, `Intermediate/`, `Saved/`.
- No usar rutas `C:\Users\...` en el reporte (`%USERPROFILE%`).

## Evidencia pedida
`docs/evidence/T-0008/01-juego.png` (1280x720) + extracto de build/tests en el Reporte.

---

## Reporte (lo completa Windows)
- estado: ok
- sha: 1515df9 (base)
- resumen: reenvío 6. Paso 0 OK (árbol limpio, `git pull` a 1515df9, stash de T-0001 intacto). Paso 1 `doctor` OK
  (UE_5.8 → True). Paso 2: el `.uproject` ya tenía `"EngineAssociation": "5.8"` (sin cambios). Paso 3: `projectfiles`
  → `Succeeded`; `build` → `Result: Succeeded` (18 acciones, 85 s), **sin errores ni warnings en nuestro código**.
  Paso 4: `tests: ok=3 fallidos=0 warnings=0`. Paso 5: `tdc.ps1 python -Script Tools\unreal\build_slice.py` (commandlet,
  sin `-InEditor`) → `LogPython: [TDC] L_Slice construido y guardado.`; `Game/Content/Maps/L_Slice.umap` creado (LFS).
  Paso 6: `01-juego.png` muestra el personaje (cilindro) parado sobre `Floor_A`, con sombra del `Ambient_Sun`; ni negro
  ni quemado, así que no toqué la luz (intensidad 1.5 sin cambios). Paso 7: `tdc.ps1 play` → `juego abierto:
  /Game/Maps/L_Slice` (ventana 1600x900); el log del juego muestra `LoadMap: /Game/Maps/L_Slice` y la ventana **quedó abierta**.
- errores: ninguno. Warnings:
  ```
  UE_5.8/Engine/Source/Runtime/Core/Public/Misc/Attribute.h(195,10): warning C4864: expected 'template' keyword before dependent template name   (motor, no nuestro)
  Tools/unreal/build_slice.py:139: DeprecationWarning: EditorLevelLibrary.get_editor_world is deprecated - Use the function in Unreal Editor Subsystem
  ```
- evidencia: `docs/evidence/T-0008/01-juego.png` (1280x720).
- notas / preguntas:
  - El personaje se ve **gris**, no rojo: el color del cilindro no se aplica (C++/material, es de la Mac).
  - Fondo negro (sin cielo ni SkyLight); la sala se lee igual. Para ambiente, sumar SkyLight/SkyAtmosphere en `build_slice.py`.
  - La cámara está cerca: en la captura solo entra `Floor_A`; el resto de los beats queda fuera de cuadro.
  - Al correr, el motor agregó a `Game/Config/DefaultEngine.ini` la sección
    `[/Script/AndroidFileServerEditor.AndroidFileServerRuntimeSettings]` con un `SecurityToken`. **No la commiteé** (la descarté
    con `git restore`); puede volver a aparecer. Sugerencia: desactivar el plugin AndroidFileServer en el `.uproject`.
  - El juego abierto levanta Live Coding: mientras siga abierto, un `tdc.ps1 build` falla por el mutex. Cerrar la ventana
    antes de la próxima tarea de build.
  - Quedó un archivo sin trackear `docs/tasks/T-0008-reporte.tmp` (temporal mío; el worker no me deja borrarlo). Borrarlo, no commitear.
