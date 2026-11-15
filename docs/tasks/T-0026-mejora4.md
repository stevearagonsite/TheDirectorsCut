---
id: T-0026
titulo: Package de mejora 4: toon sin quemar + piezas "de luz" (sobre mejora 3)
estado: enviada
tipo: mixto
rama: t/T-0026-luz-tenue
creada: 2026-11-15
---

# T-0026 — Package de mejora 4: toon sin quemar + piezas "de luz"

## Objetivo
Arreglar algo roto y un pedido de Steve, sobre todo lo de mejora 3 (cel shading, audio, texturas):
1. Con el toon los personajes se veían "demasiado claros, como una foto quemada": `apply_toon.py` compensa la auto-exposure
   (EyeAdaptationInverse) y baja el brillo y el rim.
2. "Las plataformas o paredes que se activan con la luz deben verse tenues, como si estuvieran realmente relacionadas con la luz":
   `M_TDCLightPiece` (translúcido unlit, brillo de borde, titileo suave). Las piezas de luz/oscuridad presentes quedan al 50 %
   y brillando; ausentes, como fantasma al 6 % **sin colisión** (antes desaparecían). C++: SetPiece, LightReactive, Visuals.
Genera `Game\Saved\stevearagonsite_TheDirectorsCut_mejora4.zip`. **No toques** los otros zips.

## Contexto
- Si el C++ no compila, reportá `fallo` con `archivo:línea` y no empaquetes.
- **Reenvío:** la rama ya trae el resultado de T-0023 (307b830: binarios, audio, dressing y tus fixes de `apply_toon.py`).
- Tablones: `ATDCSetPiece::ApplyPattern` baja la intensidad del patrón de 0.9 a 0.35 (en T-0023 las plataformas fijas salían casi negras).

## Pasos
1. `git fetch` y `git checkout t/T-0026-luz-tenue` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop` (y cerrá el build de T-0023).
2. `.\Tools\scripts\tdc.ps1 build`.
3. Locks: `git lfs lock Game/Content/Maps/L_Slice.umap` y
   `git ls-files Game/Content/Materials Game/Content/Dressing Game/Content/VFX Game/Content/Audio Game/Content/Characters | ForEach-Object { git lfs lock $_ }`.
4. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\make_materials.py` (M_TDCColor con patrón + **M_TDCLightPiece**).
5. (audio y dressing ya vienen de T-0023: no los repitas)
6. (no corras build_slice ni build_dressing)
7. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\apply_toon.py` (si el commandlet no alcanza, `-InEditor`).
8. `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0026 -Name 01-mejora4-a` y `-Name 01-mejora4-b`.
   Compará con `docs/evidence/T-0023/01-mejora3-b.png` (si existe) y `docs/evidence/T-0019/01-mejora2-b.png`:
   ¿los personajes ya no se ven quemados (cel shading con bandas, ni silueta negra ni blanco lavado)? ¿las plataformas fijas volvieron a gris claro con tablones visibles (no negras)? ¿la plataforma amarilla del
   inicio se ve translúcida y brillante, o como fantasma tenue si su reflector está apagado? ¿el extra común se lee rojo?
9. `.\Tools\scripts\tdc.ps1 package -Zip stevearagonsite_TheDirectorsCut_mejora4`
10. `.\Tools\scripts\tdc.ps1 smoke -Task T-0026 -Name 02-package -Wait 60 -Zip stevearagonsite_TheDirectorsCut_mejora4`
11. Commit de `Game/Content/**` (LFS), evidencia y Reporte (con tamaño del zip). `git lfs unlock`. Push y `[TDC-REPORT]`.
12. Dejá abierto el build: `.\Tools\scripts\tdc.ps1 smoke -Task T-0026 -Name 03-video -Keep -Zip stevearagonsite_TheDirectorsCut_mejora4`.

## Criterios de aceptación
- [ ] Compila; los 4 scripts sin `[TDC] ERROR`.
- [ ] `01-mejora4-b.png`: personajes sin quemar, piezas de luz translúcidas/fantasma, colores de lectura legibles.
- [ ] `_mejora4.zip` de menos de 1 GB; el exe corre a los 60 s, sin `Fatal` ni assets faltantes.

## Permite
- Editar `Tools/unreal/*.py` solo para compatibilidad de API.

## No hacer
- No cambiar C++. No pisar los otros zips. No commitear zips ni `Tools/unreal/_diag_*`.

## Evidencia pedida
`01-mejora4-b.png`, `02-package.png`, tamaño del zip y extracto del log de `smoke`.

---

## Reporte (lo completa Windows)
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
