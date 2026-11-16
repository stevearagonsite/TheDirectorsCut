---
id: T-0048
titulo: VFX pintados (lluvia, vapor, impacto del claquetazo) en lugar del pixel art
estado: enviada
tipo: mixto
rama: t/T-0048-vfx-pintados
creada: 2026-11-15
---

# T-0048 — Calle de adoquines

## Objetivo
Auditoría visual: lluvia, vapor y `claquetazo_impact` pasan de pixel art a **pintados** (mismos nombres, grilla, frames y fps; la celda del
claquetazo es 2x con la misma proporción). `import_vfx` deja `claquetazo_impact` en bilinear y `build_dressing` pone bilinear en los
flipbooks de las cards. La rama trae T-0042 (calle + luces), T-0041 y T-0039; los scripts rehacen todo (idempotentes).

## Pasos
1. `git fetch` y `git checkout t/T-0048-vfx-pintados` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop`.
2. `.\Tools\scripts\tdc.ps1 build`. Si no compila, reportá `fallo` con `archivo:línea`.
3. Locks: `git lfs lock Game/Content/Maps/L_Slice.umap` y
   `git ls-files Game/Content/Materials Game/Content/Dressing Game/Content/VFX | ForEach-Object { git lfs lock $_ }`.
4. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\make_materials.py` (tiene que importar `T_Street_Cobble`).
5. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\import_vfx.py` y `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\build_dressing.py`.
6. `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0048 -Name 01-calle-a` y `-Name 01-calle-b`. Describí: ¿la lluvia y el vapor se ven pintados y suaves (no pixelados), sin halos ni bordes de cuadro?
   ¿la calle, la marquesina y las luces de T-0042 siguen bien? ¿el plano de juego sigue siendo lo más legible, por encima del fondo y del piso?
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
