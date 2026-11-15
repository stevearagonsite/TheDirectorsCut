---
id: T-0019
titulo: Package de mejora 2: extras animados + cards 2D (sobre T-0017)
estado: enviada
tipo: mixto
rama: t/T-0019-extras
creada: 2026-11-15
---

# T-0019 — Extras animados + cards 2D + package de mejora 2

## Objetivo
Pedido de Steve: aprovechar el 2.5D combinando recursos 2D (cards de primer plano y plano medio, vapor y lluvia animados) y
extras animados en lugar del greybox. Genera `Game\Saved\stevearagonsite_TheDirectorsCut_mejora2.zip`.
**No toques** los zips `stevearagonsite_TheDirectorsCut.zip` (T-0015, entrega segura) ni `..._mejora.zip` (T-0017).

## Contexto
- Contiene todo T-0017 (parallax, VFX, claquetazo de 300 ms, luz dinámica) + T-0020 (cards desde
  `Art/Export/Dressing/Cards/cards.json`, 16 cards) + extras de Rodin (`ATDCExtra` con Puppet skeletal,
  `Tools/unreal/import_extras.py`, `Art/Export/Extras/*.fbx`).
- Los flipbooks de las cards usan `M_TDCFlipbook`: **import_vfx corre antes que build_dressing**.
- Si el build falla, reportá `fallo` con `archivo:línea` y no empaquetes.

## Pasos
1. `git fetch` y `git checkout t/T-0019-extras` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop` (y cerrá el build de T-0017 si quedó abierto).
2. `.\Tools\scripts\tdc.ps1 build`.
3. Locks: `git lfs lock Game/Content/Maps/L_Slice.umap` y todos los `.uasset` ya trackeados de Dressing, VFX y Extras:
   `git ls-files Game/Content/Dressing Game/Content/VFX Game/Content/Characters/Extras | ForEach-Object { git lfs lock $_ }`.
4. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\import_vfx.py`
5. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\import_extras.py` (tiene que importar Comun, Resistente y Sombra con Idle y Walk).
6. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\build_dressing.py` (parallax + cards + luz dinámica, sin `[TDC] ERROR`).
7. `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0019 -Name 01-mejora2-a` y después `-Name 01-mejora2-b`.
   Describí: ¿se ven cards de primer plano y plano medio (farolas, marquesina, afiches), lluvia o vapor? ¿el extra rojo es
   un personaje animado de **tamaño humano** (ni gigante ni diminuto) y mira hacia donde camina? ¿el director y los colores de
   lectura se siguen leyendo? ¿no aparece "LIGHTING NEEDS TO BE REBUILT"?
7b. **Si los extras no miden ~180 cm** (como el director): corregilos sin recompilar y repetí el shot:
   `$env:TDC_PUPPET_SCALE = "1"` si se ven gigantes, o `"10000"` si se ven diminutos, y después
   `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\set_extra_scale.py`. Anotá el valor que quedó.
8. `.\Tools\scripts\tdc.ps1 package -Zip stevearagonsite_TheDirectorsCut_mejora2`
9. `.\Tools\scripts\tdc.ps1 smoke -Task T-0019 -Name 02-package -Wait 60 -Zip stevearagonsite_TheDirectorsCut_mejora2`
10. Commit de `Game/Content/**` (LFS), evidencia y Reporte (con el tamaño del zip). `git lfs unlock`. Push y `[TDC-REPORT]`.
11. Dejá abierto el build: `.\Tools\scripts\tdc.ps1 smoke -Task T-0019 -Name 03-video -Keep -Zip stevearagonsite_TheDirectorsCut_mejora2`.

## Criterios de aceptación
- [ ] Compila; los 3 imports y build_dressing sin errores.
- [ ] `01-mejora2-b.png`: cards 2D visibles, extra animado a escala humana, sin aviso de luces.
- [ ] `_mejora2.zip` de menos de 1 GB; el exe corre a los 60 s, sin `Fatal` ni assets faltantes.
- [ ] Los zips de T-0015 y T-0017 quedaron intactos.

## Permite
- Editar `Tools/unreal/*.py` solo para compatibilidad de API.

## No hacer
- No cambiar C++. No pisar los otros zips. No commitear zips ni `Tools/unreal/_diag_*`.

## Evidencia pedida
`01-mejora2-b.png`, `02-package.png`, tamaño del zip y extracto del log de `smoke`.

---

## Reporte (lo completa Windows)
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
