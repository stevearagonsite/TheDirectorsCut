---
id: T-0058
titulo: Valores de vereda, tapas y rim del director + limpieza de cards huérfanas (T-0058)
estado: enviada
tipo: editor
rama: t/T-0058-valores
creada: 2026-11-16
---

# T-0058 — Valores + limpieza de cards

## Objetivo
**T-0058 (valores):** la vereda y `Pit_Floor` bajan su color_override de 0.5 a 0.16–0.19 (salían casi blancos); en `M_TDCColor` TopBoost pasa de 1.8 a 1.25
(tapas casi blancas); el rim del director usa RimPower 4.5, fuerza 2.2 y ámbar (1, .55, .22), así queda en el borde y no como tinte.
**T-0057 (limpieza):** Quedaron assets de versiones anteriores sin uso (`MI_Card_anim_lluvia_80`, `MI_Card_anim_vapor_100`, `M_/T_Card_mid_pared_ladrillo_65`, etc.).
`build_dressing.py` ahora borra al final los `MI_/M_/T_Card_*` de `/Game/Dressing/Cards` que nadie referencia y loguea
`cards huérfanas borradas: N`. La rama trae main con T-0056.

## Pasos
1. `git fetch` y `git checkout t/T-0058-valores` (verificá el sha). `git lfs pull`. `.\Tools\scripts\tdc.ps1 stop`.
2. `.\Tools\scripts\tdc.ps1 build`.
3. Locks de `Game/Content/Maps/L_Slice.umap` y de todo `Game/Content/Dressing`, `Game/Content/Materials` y `Game/Content/Characters` (incluido `Cards/`). `ForEach-Object` no corre en tu
   sesión: usá globs por carpeta como en T-0055.
4. `make_materials.py` → `build_dressing.py` → `apply_toon.py`. De build_dressing, → pegá la línea `cards huérfanas borradas: N` y la lista de lo borrado.
5. `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0058 -Name 01-escena` (cámara de juego). Criterio: **adoquín legible abajo**,
   **tapas en gris medio** (no blancas), **contorno ámbar nítido** en el director; cards, lluvia, vapor y fachadas siguen presentes.
6. `.\Tools\scripts\tdc.ps1 playtest -Task T-0058` → RESULT PASS.
7. Commit con **`git add -A Game/Content/Dressing Game/Content/Materials Game/Content/Characters`** (para registrar los borrados), la evidencia y el Reporte. `git lfs unlock`. Push y `[TDC-REPORT]`.

## Criterios de aceptación
- [ ] `cards huérfanas borradas: N` con N > 0, y los `.uasset` borrados figuran en el commit.
- [ ] Adoquín legible, tapas gris medio, rim ámbar en el borde; playtest RESULT PASS.

## Permite
- Editar `Tools/unreal/build_dressing.py` solo para compatibilidad de API.

## No hacer
- No cambiar C++. No empaquetar. No borrar a mano assets que el script no listó.

---

## Reporte (lo completa Windows)
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
