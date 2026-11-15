---
id: T-0026
titulo: Package de mejora 4: toon sin quemar + piezas "de luz" (sobre mejora 3)
estado: ok
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
- estado: ok
- sha: (el del commit de este reporte; ver `[TDC-REPORT]`)
- resumen: Compila (12 acciones, sin errores ni warnings de C++). `make_materials.py` y `apply_toon.py` sin `[TDC] ERROR`
  (M_TDCColor + M_TDCLightPiece creados; toon 4/4 personajes). Hubo que corregir un bug de API en los dos scripts
  (ver notas): con eso los personajes ya no salen quemados ni en silueta. Package `Game\Saved\stevearagonsite_TheDirectorsCut_mejora4.zip`
  de **346.1 MB**; el exe seguía corriendo a los 60 s, sin `Fatal` ni assets faltantes. Los otros zips no se tocaron.
  Build abierto con `smoke -Keep` (paso 12).
- errores: ninguno de compilación ni de scripts. Único warning de carga en el log del package:
  `LogStreaming: Warning: LoadPackage: SkipPackage: /Game/Characters/Director/BP_Director - The package to load does not exist on disk or in the loader`
  (no comprobé si ya salía en packages anteriores).
- evidencia: `docs/evidence/T-0026/`
  - `01-mejora4-a.png`, `01-mejora4-b.png`: editor `-game`, después del fix.
  - `02-package.png`: primer arranque del zip. **Sale un degradé azul borroso** (la captura es del frame 0 del primer arranque);
    no es representativa.
  - `03-video.png`: segundo arranque del mismo zip (paso 12): escena correcta, igual a `01-mejora4-b.png`. Usar esta como evidencia del package.
  - `autorun/TDCAutoRun_02.png`, `_12.png`, `_20.png`: frames de `tdc.ps1 autorun` en el editor (extra común rojo con bandas; pieza amarilla fantasma).
  - Extracto del log de `smoke` (60 s):
    ```
    seguía corriendo a los 60 s: sí
    LogLoad: LoadMap: /Game/Maps/L_Slice?Name=Player
    LogLoad: Took 0.094706 seconds to LoadMap(/Game/Maps/L_Slice)
    LogGlobalStatus: UEngine::LoadMap Load map complete /Game/Maps/L_Slice
    ```
- notas / preguntas:
  - **Fix de API (Permite) en `Tools/unreal/apply_toon.py` y `Tools/unreal/make_materials.py`:** en UE 5.8 el pin de entrada de
    `MaterialExpressionEyeAdaptationInverse` se llama `LightValueInput` (nombre de la propiedad, ver
    `Engine/Source/Runtime/Engine/Public/Materials/MaterialExpressionEyeAdaptationInverse.h`), no `LightValue`.
    `connect_material_expressions` devolvía `False` sin error, el nodo quedaba sin entrada y emitía blanco (1,1,1) × inversa de exposición:
    en la primera corrida los personajes salían como **silueta negra** en el frame 0 y **silueta blanca plana** después (peor que T-0023).
    Ahora los scripts prueban `LightValueInput` y luego `LightValue`, y cortan con `[TDC] ERROR` si ninguno conecta. Se volvieron a correr los dos.
  - Comparación con `docs/evidence/T-0023/01-mejora3-b.png` y `docs/evidence/T-0019/01-mejora2-b.png`:
    - Personajes: ya no quemados ni silueta. Director gris con detalle de textura, extra común **rojo** legible, bandas de sombra visibles.
    - Plataforma fija del inicio: la cara frontal volvió a gris claro (antes casi negra); **la cara superior sigue casi negra** y
      **los tablones no se distinguen** a esta distancia (gris liso).
    - Pieza amarilla (a la derecha del inicio): **fantasma tenue** translúcido amarillo (reflector apagado). No hay en las capturas
      una pieza presente al 50 % brillando: no se encendió ningún reflector (capturas automáticas, sin input), así que ese estado quedó sin verificar.
  - `L_Slice.umap` no cambió. Assets modificados: M_TDCColor, M_TDCToon, M_TDCLightPiece (nuevo), 4 texturas de `Materials/Textures`,
    y `MI_Toon_*` + `SK_*` de los 4 personajes.
  - En el autorun el director queda fuera de cuadro desde el frame ~03; pasa igual en los frames de T-0023, no es de esta tarea.
  - `tdc.ps1 smoke` saca la captura en el frame 0; en el primer arranque de un build nuevo puede salir borrosa (`02-package.png`).
  - Locks: se tomaron los 115 del paso 3 y se soltaron al terminar. No hizo falta limpiar nada de la corrida cancelada (árbol limpio, sin locks).
