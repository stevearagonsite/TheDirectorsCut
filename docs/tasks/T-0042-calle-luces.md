---
id: T-0042
titulo: Calle: adoquín en el primer plano, luces de farolas y marquesina a nivel calle
estado: enviada
tipo: mixto
rama: t/T-0042-calle-luces
creada: 2026-11-15
---

# T-0042 — Calle de adoquines

## Objetivo
Sigue la auditoría visual: **DR_ForegroundStage** (la franja negra del 22 % inferior) pasa a adoquín; luces ámbar desaturado en las
farolas y en la marquesina (`DR_CardLight_*`); la marquesina y los afiches bajan a nivel de calle y dejan de flotar sobre los techos.
Solo `build_dressing.py` y `cards.json`. La rama trae T-0041 y T-0039; los scripts rehacen todo (son idempotentes).

## Pasos
1. `git fetch` y `git checkout t/T-0042-calle-luces` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop`.
2. `.\Tools\scripts\tdc.ps1 build`. Si no compila, reportá `fallo` con `archivo:línea`.
3. Locks: `git lfs lock Game/Content/Maps/L_Slice.umap` y
   `git ls-files Game/Content/Materials Game/Content/Dressing Game/Content/VFX | ForEach-Object { git lfs lock $_ }`.
4. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\make_materials.py` (tiene que importar `T_Street_Cobble`).
5. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\import_vfx.py` y `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\build_dressing.py`.
6. `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0042 -Name 01-calle-a` y `-Name 01-calle-b`. Describí: ¿la franja de abajo (primer plano)
   es adoquín mojado (no negro plano)? ¿la marquesina y los afiches están a nivel calle, con luz ámbar? ¿el plano de juego sigue siendo lo más legible, por encima del fondo y del piso?
7. Commit de `Game/Content/**` (LFS), evidencia y Reporte. `git lfs unlock`. Push y `[TDC-REPORT]`.

## Criterios de aceptación
- [ ] Compila; scripts sin `[TDC] ERROR`.
- [ ] `01-calle-b.png`: adoquín visible abajo, sin restarle lectura al plano de juego.

## Permite
- Editar `Tools/unreal/*.py` solo para compatibilidad de API.

## No hacer
- No cambiar C++. No empaquetar. No commitear `Tools/unreal/_diag_*`.

---

## Reporte (lo completa Windows)
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
