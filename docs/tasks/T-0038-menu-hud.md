---
id: T-0038
titulo: Menú inicial + pausa + HUD con Canvas (T-0035 + T-0038 juntas)
estado: enviada
tipo: mixto
rama: t/T-0038-hud
creada: 2026-11-15
---

# T-0038 — Menú inicial, pausa y HUD nuevo

## Objetivo
Steve: "no tengo menú inicial". Esta rama trae T-0035 (menú en Slate: Jugar, Controles, Salir; carga "TAKE 1"; pausa con Esc/Start;
ícono del exe) y T-0038 (HUD con Canvas: controles con íconos, prompt [E], toasts y banner de rebobinado, en lugar de los
mensajes de debug). Detalle de T-0035 en [`T-0035-menu.md`](T-0035-menu.md).

## Contexto
- El menú no aparece con `-TDCAutoRun`, `-TDCPlaytest`, `-TDCRecord`, `-TDCReplay` ni `-TDCNoMenu`.
- En PIE, Esc cierra PIE: la pausa se prueba en `-game` o en el exe.
- Genera `Game\Saved\stevearagonsite_TheDirectorsCut_dev.zip` (no toca los zips de la jam).

## Pasos
1. `git fetch` y `git checkout t/T-0038-hud` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop`.
2. `.\Tools\scripts\tdc.ps1 build`. Si no compila, reportá `fallo` con `archivo:línea`.
3. Locks de `Game/Content/UI` si ya existe (`git ls-files Game/Content/UI | ForEach-Object { git lfs lock $_ }`).
   `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\import_ui.py` (importa `/Game/UI` y `/Game/UI/HUD/T_UI_*`).
4. `.\Tools\scripts\tdc.ps1 autorun -Task T-0038 -Wait 40`: sale sin menú; en las capturas tiene que verse el **HUD nuevo**
   (panel de controles con íconos arriba a la izquierda) y **no** los mensajes de debug de texto plano.
5. `.\Tools\scripts\tdc.ps1 package -Zip stevearagonsite_TheDirectorsCut_dev` y
   `.\Tools\scripts\tdc.ps1 smoke -Task T-0038 -Name 01-menu -Wait 30 -Zip stevearagonsite_TheDirectorsCut_dev`:
   la captura tiene que mostrar el **menú inicial** (Jugar / Controles / Salir). El ícono del exe: anotá si `TheDirectorsCut.exe`
   del zip tiene ícono propio (propiedades del archivo, si lo podés ver).
6. Commit de `Game/Content/UI/**` (LFS), evidencia y Reporte (con el tamaño del zip). `git lfs unlock`. Push y `[TDC-REPORT]`.
7. Dejá abierto el build para Steve: `.\Tools\scripts\tdc.ps1 smoke -Task T-0038 -Name 02-abierto -Keep -Zip stevearagonsite_TheDirectorsCut_dev`.

## Criterios de aceptación
- [ ] Compila; `import_ui.py` sin `[TDC] ERROR`.
- [ ] Autorun con el HUD nuevo (sin mensajes de debug).
- [ ] El exe del zip arranca en el menú inicial; corre a los 30 s sin `Fatal`; quedó abierto.

## Permite
- Editar `Tools/unreal/import_ui.py` solo para compatibilidad de API.

## No hacer
- No cambiar C++. No tocar los zips de la jam. No commitear zips ni `Tools/unreal/_diag_*`.

## Evidencia pedida
`autorun/` con el HUD, `01-menu.png` y el Reporte.

---

## Reporte (lo completa Windows)
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
