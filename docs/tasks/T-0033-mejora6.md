---
id: T-0033
titulo: Package de mejora 6: extras que animan (skeleton correcto en los clips)
estado: enviada
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
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
