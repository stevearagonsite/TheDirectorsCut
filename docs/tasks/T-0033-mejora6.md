---
id: T-0033
titulo: Package de mejora 6: extras que animan (skeleton correcto en los clips)
estado: parcial
tipo: editor
rama: t/T-0033-extras-skeleton
creada: 2026-11-15
---

# T-0033 — Extras animados de verdad + package de mejora 6

## Objetivo
Steve: "los personajes enemigos no tienen animaciones". `check_anims.py` (T-0031) mostró que los 6 clips `AS_Extra_*` apuntan a un
skeleton que no es el de su `SK_Extra_*`. Causa: `import_extras.py` renombraba el Skeleton y no guardaba los clips. Ya está arreglado.
Genera `Game\Saved\stevearagonsite_TheDirectorsCut_mejora6.zip`. **No toques** los otros zips.
**URGENTE (22:25, la jam cierra 22:59): Steve tiene abierto el build de `_mejora5` en `Game\Saved\PackageTest`. NO corras `smoke` (lo cerraría).**

## Pasos
1. `git fetch` y `git checkout t/T-0033-extras-skeleton` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop`.
2. `.\Tools\scripts\tdc.ps1 build` (no debería recompilar nada).
3. Locks: `git ls-files Game/Content/Characters/Extras | ForEach-Object { git lfs lock $_ }`.
4. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\import_extras.py` (hace clean de la carpeta: borra también MI_Toon_* y T_Extra_*).
5. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\apply_toon.py` (**después** de import_extras; si no, los extras quedan sin toon).
6. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\check_anims.py` → los 9 clips con `mismo esqueleto=True`. Pegá la salida.
   Si algún clip de extra sigue en `False`, reportá `parcial` y **no empaquetes**.
7. `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0033 -Name 01-mejora6` (el extra rojo con toon, no gris).
8. `.\Tools\scripts\tdc.ps1 package -Zip stevearagonsite_TheDirectorsCut_mejora6`
9. (sin smoke: no cierres el build abierto)
10. Commit de `Game/Content/Characters/Extras/**` (LFS), evidencia y Reporte (con tamaño del zip). `git lfs unlock`. Push y `[TDC-REPORT]`.
11. Mandá el `[TDC-REPORT]` apenas esté el zip, con su tamaño: Steve lo sube a mano.

## Criterios de aceptación
- [ ] `check_anims.py`: los 6 clips de extras con `mismo esqueleto=True`.
- [ ] `01-mejora6.png`: extras con cel shading (no grises).
- [ ] `_mejora6.zip` de menos de 1 GB, `BUILD SUCCESSFUL`.

## Permite
- Editar `Tools/unreal/import_extras.py` y `apply_toon.py` solo para compatibilidad de API.

## No hacer
- No cambiar C++ ni el mapa. No pisar los otros zips. No commitear zips ni `Tools/unreal/_diag_*`.

## Evidencia pedida
Salida de `check_anims.py`, `01-mejora6.png`, tamaño del zip y extracto del log de `smoke`.

---

## Reporte (lo completa Windows)
- estado: parcial (zip listo y `BUILD SUCCESSFUL`; el cel shading de los extras **no quedó verificado en imagen**)
- sha: el del commit de este reporte (ver `[TDC-REPORT]`)
- resumen:
  - `build` ok (recompiló `TDCCharacter.cpp`, sin errores ni warnings). `stop`: no había editor abierto. No se corrió `smoke` ni se tocó `Game\Saved\PackageTest`.
  - `import_extras.py` ok (0 errores). `apply_toon.py` ok en el segundo intento: `toon aplicado a 4/4 personajes`.
  - `check_anims.py`: los 9 clips con `mismo esqueleto=True` (salida abajo).
  - `package`: `BUILD SUCCESSFUL`, `Game\Saved\stevearagonsite_TheDirectorsCut_mejora6.zip` = **357.7 MB**. Los otros zips no se tocaron.
- errores:
  - Primer intento de `apply_toon.py` falló: `LogSavePackage: Error: Cannot remove 'Game/Content/Materials/M_TDCToon.uasset' as it is read only!` →
    `[TDC] ERROR: no se pudo guardar M_TDCToon (¿lock de LFS?)`. El paso 3 solo lockea `Characters/Extras`, pero `apply_toon.py` también
    guarda `Materials/M_TDCToon`, `Characters/Director/MI_Toon_Director` y `Characters/Director/SK_Director`. Los lockeé y repetí: ok.
- evidencia: `docs/evidence/T-0033/01-mejora6.png`
- notas / preguntas:
  - **Sin verificar:** en `01-mejora6.png` no aparece ningún extra (la cámara inicial de `L_Slice` solo muestra al director). Probé además
    un `autorun` de 40 s y en los cuadros que revisé (04, 08, 12, 16, 21) tampoco entra un extra en cuadro; no commiteé esas capturas. El
    criterio "extras con cel shading (no grises)" queda respaldado solo por el log de `apply_toon.py`, no por imagen. Tampoco hay prueba
    en runtime de que los extras animen: sin `smoke`, el zip no se ejecutó.
  - El commit incluye 3 assets fuera de `Characters/Extras/**` porque `apply_toon.py` los reescribe: `M_TDCToon`, `MI_Toon_Director`, `SK_Director`.
  - El reimport cambió nombres: se van `SKEL_Extra_*` y `T_Extra_*_Diffuse`; aparecen `SK_Extra_*_Skeleton`, `model`, `texture_diffuse`,
    `texture_normal` y `texture_roughness` en cada carpeta (no siguen los prefijos de `convenciones.md`; no los renombré).
  - `import_extras.py` loguea 3 warnings `MI_Toon_Extra_*.uasset: Error opening file` (los borra el clean y los recrea `apply_toon.py`) y
    3 de FBX `No smoothing group information`.

```
[TDC] Director/AS_Director_Idle: 8.33s, frames=250, pistas=12, mismo esqueleto=True, movimiento=4.50
[TDC] Director/AS_Director_Walk: 2.57s, frames=77, pistas=12, mismo esqueleto=True, movimiento=29.54
[TDC] Director/AS_Director_Run: 1.17s, frames=35, pistas=12, mismo esqueleto=True, movimiento=104.59
[TDC] Comun/AS_Extra_Comun_Idle: 8.33s, frames=250, pistas=12, mismo esqueleto=True, movimiento=4.55
[TDC] Comun/AS_Extra_Comun_Walk: 1.03s, frames=31, pistas=12, mismo esqueleto=True, movimiento=18.30
[TDC] Resistente/AS_Extra_Resistente_Idle: 8.33s, frames=250, pistas=12, mismo esqueleto=True, movimiento=4.55
[TDC] Resistente/AS_Extra_Resistente_Walk: 1.03s, frames=31, pistas=12, mismo esqueleto=True, movimiento=18.30
[TDC] Sombra/AS_Extra_Sombra_Idle: 8.33s, frames=250, pistas=12, mismo esqueleto=True, movimiento=4.55
[TDC] Sombra/AS_Extra_Sombra_Walk: 1.03s, frames=31, pistas=12, mismo esqueleto=True, movimiento=18.30

[TDC] Extra_Comun: MI_Toon asignado a 1 slot(s) con texture_diffuse
[TDC] Extra_Resistente: MI_Toon asignado a 1 slot(s) con texture_diffuse
[TDC] Extra_Sombra: MI_Toon asignado a 1 slot(s) con texture_diffuse
[TDC] toon aplicado a 4/4 personajes

BuildCookRun time: 149.98 s
BUILD SUCCESSFUL
zip:   %USERPROFILE%\Workspace\TheDirectorsCut\Game\Saved\stevearagonsite_TheDirectorsCut_mejora6.zip  (357.7 MB)
```
