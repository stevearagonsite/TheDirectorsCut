---
id: T-0017
titulo: Package de mejora: parallax de 3 capas + VFX de mecánicas (sobre el package final T-0015)
estado: enviada
tipo: mixto
rama: t/T-0017-parallax
creada: 2026-11-15
---

# T-0017 — Parallax + VFX + package de mejora

## Objetivo
Pedidos de Steve: fondo con parallax en varias capas y VFX en las mecánicas (T-0018 incluida acá). Es un package **de mejora**: el zip de T-0015
(`Game\Saved\stevearagonsite_TheDirectorsCut.zip`) es la entrega segura y **no se toca**. Este genera
`Game\Saved\stevearagonsite_TheDirectorsCut_mejora.zip`; si la captura se ve peor o algo falla, se entrega T-0015.

## Contexto
- `build_dressing.py` (cc4ff4d): si existen `Art/Export/Dressing/Parallax/{L0_cielo,L1_skyline_lejano,L2_edificios_cercanos}.png`,
  arma 3 planos unlit a Y −4500, −3000 y −1600 en `/Game/Dressing/Parallax/` (L1 y L2 con alpha). Si no, queda el telón único.
- C++ nuevo de T-0018: `ATDCFlipbookFx` (quad con sprite sheet) en el claquetazo, los extras (derrota, aturdido, rebote),
  las luces y el rebobinado. Si faltan los assets de `/Game/VFX`, no hace nada.
- `Tools/unreal/import_vfx.py`: importa `Art/Export/VFX/*.png` a `/Game/VFX/T_*` y arma `/Game/VFX/M_TDCFlipbook`.
- Si el build falla por T-0018, reportá `fallo` con `archivo:línea` y no empaquetes: la Mac reenvía sin VFX.

## Pasos
1. `git fetch` y `git checkout t/T-0017-parallax` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop` (y cerrá el build de T-0015 si quedó abierto).
2. `.\Tools\scripts\tdc.ps1 build`.
3. Locks: `git lfs lock Game/Content/Maps/L_Slice.umap` y **todos** los `.uasset` ya trackeados de Dressing:
   `git ls-files Game/Content/Dressing | ForEach-Object { git lfs lock $_ }` (los props se reimportan y fallan sin lock).
4. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\build_dressing.py`. Tiene que armar las 3 capas sin `[TDC] ERROR`.
4b. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\import_vfx.py` (los `/Game/VFX/*` son nuevos; si ya existieran, lockealos).
   Tiene que crear los 6 `T_*` y `M_TDCFlipbook` sin errores.
5. `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0017 -Name 01-parallax-a` y después `-Name 01-parallax-b`.
   Compará con `docs/evidence/T-0015/01-final.png`: ¿se ven las 3 capas, sin halos, sin huecos ni bordes del plano en cuadro?
   ¿el director y los colores de lectura se siguen leyendo? ¿Ya **no** aparece "LIGHTING NEEDS TO BE REBUILT" (build_dressing
   ahora fuerza luz 100% dinámica)? Si se ve peor, reportá `parcial` y **no empaquetes**.
6. `.\Tools\scripts\tdc.ps1 package -Zip stevearagonsite_TheDirectorsCut_mejora`.
7. `.\Tools\scripts\tdc.ps1 smoke -Task T-0017 -Name 02-package -Wait 60 -Zip stevearagonsite_TheDirectorsCut_mejora`
   (mismos chequeos que T-0015: corre a los 60 s, log sin `Fatal` ni assets faltantes).
8. Commit de `Game/Content/**` (LFS, incluye `/Game/VFX`), evidencia y Reporte (con el tamaño del zip). `git lfs unlock` de lo bloqueado. Push y `[TDC-REPORT]`.
9. Dejá abierto el build de mejora: `.\Tools\scripts\tdc.ps1 smoke -Task T-0017 -Name 03-video -Keep -Zip stevearagonsite_TheDirectorsCut_mejora`.

## Criterios de aceptación
- [ ] Compila con T-0018; `import_vfx.py` crea los 6 sheets y `M_TDCFlipbook`.
- [ ] `01-parallax-b.png` muestra las 3 capas y se ve igual o mejor que T-0015.
- [ ] `..._mejora.zip` de menos de 1 GB; el exe corre a los 60 s, sin `Fatal` ni assets faltantes.
- [ ] En `02-package.png` no aparece "LIGHTING NEEDS TO BE REBUILT".
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
