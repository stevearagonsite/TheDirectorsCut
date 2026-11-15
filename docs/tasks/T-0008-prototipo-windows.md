---
id: T-0008
titulo: Prototipo jugable corriendo en la PC Windows (build + L_Slice + juego abierto)
estado: enviada
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
0. **Reenvío 3.** Steve verificó en la PC: existe `NETFXSDK\4.8`, `gitleaks version` responde, `git config --global user.email`
   tiene valor y reinició vkvm-worker. `Game/.vsconfig` ahora está en `.gitignore`. En el clon: `git restore --staged --worktree Game/TheDirectorsCut.uproject docs/tasks/T-0008-prototipo-windows.md`
   y `git pull` (esta versión ya trae `EngineAssociation` 5.8). Seguí desde el paso 1 (ver **Permite**). El stash de T-0001 dejalo como está.
1. `.\Tools\scripts\tdc.ps1 doctor` (UE 5.8 tiene que figurar `True`).
2. `Game/TheDirectorsCut.uproject`: `"EngineAssociation": "5.8"` (o el identificador que use tu instalación).
3. `.\Tools\scripts\tdc.ps1 projectfiles` y `.\Tools\scripts\tdc.ps1 build`.
   - Si hay errores de C++: **pará**, reportá `estado: fallo` con `archivo:línea` y el mensaje exacto de los primeros errores
     reales (no la cascada). La Mac los arregla y reenvía.
4. `.\Tools\scripts\tdc.ps1 test` (3 tests `TDC.Stage.*`). Si fallan, anotalo y seguí.
5. Generar el mapa: `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\build_slice.py` → tiene que loguear
   `[TDC] L_Slice construido y guardado.` Si `-run=pythonscript` no puede usar los subsistemas del editor, usá
   `UnrealEditor-Cmd.exe "<proj>" -ExecutePythonScript="<repo>\Tools\unreal\build_slice.py" -unattended -nosplash` o el
   Output Log del editor. Si falla por API de Python de 5.8, arreglá `build_slice.py` sin cambiar el layout.
6. Captura: `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0008 -Name 01-juego`.
   Revisala: tiene que verse el personaje (cilindro rojo) y las plataformas. Si está todo negro o quemado, ajustá la luz
   `Ambient_Sun` en el mapa (o sumá un SkyLight) y anotá los valores para pasarlos al script.
7. Commit de `Game/Content/Maps/L_Slice.umap` (LFS), el `.uproject`, la evidencia y el Reporte. Push y `[TDC-REPORT]`.
8. **Dejá el juego abierto en la pantalla** para Steve (no lo cierres):
   `& "$UE\Engine\Binaries\Win64\UnrealEditor.exe" "<repo>\Game\TheDirectorsCut.uproject" /Game/Maps/L_Slice -game -windowed -ResX=1600 -ResY=900`
   (con `Start-Process`, sin esperar). Confirmá en el reporte que la ventana quedó abierta y con foco.

## Criterios de aceptación
- [ ] `TheDirectorsCutEditor Win64 Development` compila sin errores (listar warnings de nuestro código si hay).
- [ ] Tests `TDC.Stage.*`: resultado anotado.
- [ ] `Game/Content/Maps/L_Slice.umap` commiteado y abre como mapa de inicio.
- [ ] `docs/evidence/T-0008/01-juego.png` muestra el personaje y la sala.
- [ ] El juego quedó corriendo en una ventana en la PC.

## Permite
- Steve instaló a mano en la PC (2026-11-15) el .NET Framework 4.8 Developer Pack, gitleaks y la identidad global de git.
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
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
