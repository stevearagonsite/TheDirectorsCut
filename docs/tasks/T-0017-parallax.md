---
id: T-0017
titulo: Package de mejora: fondo con parallax de 3 capas (sobre el package final T-0015)
estado: enviada
tipo: mixto
rama: t/T-0017-parallax
creada: 2026-11-15
---

# T-0017 — Parallax + package de mejora

## Objetivo
Pedido de Steve: fondo con parallax en varias capas. Es un package **de mejora**: el zip de T-0015
(`Game\Saved\stevearagonsite_TheDirectorsCut.zip`) es la entrega segura y **no se toca**. Este genera
`Game\Saved\stevearagonsite_TheDirectorsCut_parallax.zip`; si la captura se ve peor o algo falla, se entrega T-0015.

## Contexto
- `build_dressing.py` (cc4ff4d): si existen `Art/Export/Dressing/Parallax/{L0_cielo,L1_skyline_lejano,L2_edificios_cercanos}.png`,
  arma 3 planos unlit a Y −4500, −3000 y −1600 en `/Game/Dressing/Parallax/` (L1 y L2 con alpha). Si no, queda el telón único.
- Mismo C++ que T-0015 (main + claquetazo de 300 ms). No hay C++ nuevo.

## Pasos
1. `git fetch` y `git checkout t/T-0017-parallax` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop` (y cerrá el build de T-0015 si quedó abierto).
2. `.\Tools\scripts\tdc.ps1 build`.
3. Locks: `git lfs lock Game/Content/Maps/L_Slice.umap` y **todos** los `.uasset` ya trackeados de Dressing:
   `git ls-files Game/Content/Dressing | ForEach-Object { git lfs lock $_ }` (los props se reimportan y fallan sin lock).
4. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\build_dressing.py`. Tiene que armar las 3 capas sin `[TDC] ERROR`.
5. `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0017 -Name 01-parallax-a` y después `-Name 01-parallax-b`.
   Compará con `docs/evidence/T-0015/01-final.png`: ¿se ven las 3 capas, sin halos, sin huecos ni bordes del plano en cuadro?
   ¿el director y los colores de lectura se siguen leyendo? Si se ve peor, reportá `parcial` y **no empaquetes**.
6. `.\Tools\scripts\tdc.ps1 package -Zip stevearagonsite_TheDirectorsCut_parallax`.
7. `.\Tools\scripts\tdc.ps1 smoke -Task T-0017 -Name 02-package -Wait 60 -Zip stevearagonsite_TheDirectorsCut_parallax`
   (mismos chequeos que T-0015: corre a los 60 s, log sin `Fatal` ni assets faltantes).
8. Commit de `Game/Content/**` (LFS), evidencia y Reporte (con el tamaño del zip). `git lfs unlock` de lo bloqueado. Push y `[TDC-REPORT]`.
9. Dejá abierto el build de mejora: `.\Tools\scripts\tdc.ps1 smoke -Task T-0017 -Name 03-video -Keep -Zip stevearagonsite_TheDirectorsCut_parallax`.

## Criterios de aceptación
- [ ] `01-parallax-b.png` muestra las 3 capas y se ve igual o mejor que T-0015.
- [ ] `..._parallax.zip` de menos de 1 GB; el exe corre a los 60 s, sin `Fatal` ni assets faltantes.
- [ ] El zip de T-0015 quedó intacto.

## Permite
- Editar `Tools/unreal/build_dressing.py` solo para compatibilidad de API, sin cambiar profundidades ni tamaños.

## No hacer
- No pisar ni borrar `Game\Saved\stevearagonsite_TheDirectorsCut.zip`. No cambiar C++. No commitear zips ni `Tools/unreal/_diag_*`.

## Evidencia pedida
`01-parallax-b.png`, `02-package.png`, tamaño del zip y extracto del log de `smoke`.

---

## Reporte (lo completa Windows)
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
