---
id: T-0035
titulo: Menú inicial, pausa e ícono del ejecutable
estado: nueva
tipo: mixto
rama: t/T-0035-menu
creada: 2026-11-15
---

# T-0035 — Menú inicial, pausa e ícono del ejecutable

## Objetivo
Steve: "no tengo menú inicial". El juego tiene que arrancar en un menú (Jugar, Controles y Salir) con el arte del título,
mostrar la pantalla de carga "TAKE 1" al elegir Jugar y abrir una pausa con Esc. El exe lleva el ícono de la claqueta.
Diseño: `docs/product/features/menu/design.md`.

## Contexto
- C++ nuevo: `Core/TDCPlayerController` (lo fija `ATDCGameMode`) y `UI/TDCMenuWidget`, en Slate puro. `Build.cs` suma
  `Slate` y `SlateCore`. "¡Acción!" pasa del `BeginPlay` del personaje al momento de Jugar.
- `Tools/unreal/import_ui.py`: importa `Art/Export/UI/*.png` y `HUD/*.png` a `/Game/UI/T_UI_*`. `/Game/UI` se cocina siempre.
- `Game/Build/Windows/Application.ico`: ícono del exe.
- El menú **no** aparece con `-TDCAutoRun`, `-TDCPlaytest`, `-TDCRecord`, `-TDCReplay` ni `-TDCNoMenu`. Por eso
  `autorun` sigue sirviendo para evidencia de juego. `shot` usa HighResShot, que no captura Slate: muestra el nivel en
  pausa detrás del menú.
- Si el C++ no compila, reportá `fallo` con `archivo:línea` y no empaquetes.

## Pasos
1. `git fetch` y `git checkout t/T-0035-menu` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop`.
2. `.\Tools\scripts\tdc.ps1 build`.
3. Si ya existe `Game/Content/UI`: `git ls-files Game/Content/UI | ForEach-Object { git lfs lock $_ }`.
4. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\import_ui.py`. Tiene que decir "texturas de UI importadas: 8"
   (title_bg, title_screen, loading y 5 del HUD), sin `[TDC] ERROR`.
5. `.\Tools\scripts\tdc.ps1 package -Zip stevearagonsite_TheDirectorsCut_dev`
6. **Antes del smoke, avisá por VirtualKvm:** smoke cierra cualquier exe abierto en `Game\Saved\PackageTest`, que puede
   ser el build que Steve está jugando.
   `.\Tools\scripts\tdc.ps1 smoke -Task T-0035 -Name 01-menu -Wait 20 -Zip stevearagonsite_TheDirectorsCut_dev -Keep`
   La captura (`Screenshot showui`) tiene que mostrar el **menú principal**: fondo del título y Jugar / Controles / Salir
   abajo a la izquierda, con "Jugar" en amarillo (foco).
7. `.\Tools\scripts\tdc.ps1 autorun -Task T-0035 -Wait 20`: las capturas muestran el nivel (sin menú) y al director corriendo.
8. Si podés interactuar con la ventana del paso 6:
   - Flecha abajo y Enter sobre "Controles": se ve la lista de controles.
   - Esc vuelve al menú principal.
   - Enter sobre "Jugar": pantalla "TAKE 1", después el juego y "¡Acción!".
   - Esc en el juego abre la pausa.
   Sacá una captura de cada pantalla con Win+Shift+S o `Screenshot showui` desde la consola, en
   `docs/evidence/T-0035/02-controles.png`, `03-pausa.png`. Si no podés interactuar, anotalo y lo prueba Steve.
9. Commit de `Game/Content/UI/**`, la evidencia y el reporte.

## Criterios de aceptación
- [ ] Compila sin errores ni warnings nuevos.
- [ ] `import_ui.py` importa 8 texturas sin `[TDC] ERROR`.
- [ ] `01-menu.png` muestra el menú principal con el arte del título. El log del juego tiene `[TDC-MENU] menú principal visible`.
- [ ] El exe empaquetado tiene el ícono de la claqueta (mirá `TheDirectorsCut.exe` en el Explorador).
- [ ] `autorun` arranca sin menú.

## Permite
Nada fuera de lo habitual.

## No hacer
- No tocar los zips `_mejora*`.
- No editar el C++: si no compila, reportá.

## Evidencia pedida
`docs/evidence/T-0035/01-menu.png`, autorun en `docs/evidence/T-0035/autorun/`, las capturas del paso 8 si se pudo, y un
extracto del log con las líneas `[TDC-MENU]`.

---

## Reporte (lo completa Windows)
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
