---
id: T-0023
titulo: Package de mejora 3: cel shading de personajes + audio + texturas del set (sobre T-0019)
estado: enviada
tipo: mixto
rama: t/T-0023-toon
creada: 2026-11-15
---

# T-0023 — Cel shading + audio + texturas + package de mejora 3

## Objetivo
Pedido de Steve: integrar todo el arte nuevo. Incluye T-0021 (texturas), T-0022 (**audio**) y T-0023 (**cel shading** de personajes y texturas de los extras, para que lo 3D no se vea tan oscuro) (el juego estaba mudo: el audio es criterio del jurado). Texturas tileables en las piezas según su mecánica (tablones = fija, gelatina =
luz/oscuridad, película = en toma), sin perder el color de lectura. Genera `Game\Saved\stevearagonsite_TheDirectorsCut_mejora3.zip`.
**No toques** los zips de T-0015, T-0017 ni T-0019.
**Audio:** el agente no puede escuchar; en el Reporte copiá del log del juego las líneas de audio (errores de carga de `/Game/Audio`, si hay).

## Contexto
- La rama ya trae el resultado de T-0019 (cards, extras, VFX, parallax): solo cambia M_TDCColor y las piezas.
- `make_materials.py` importa `Art/Export/Textures/*` a `/Game/Materials/Textures` y suma a M_TDCColor un patrón world-aligned
  (parámetros Pattern, PatternStrength, TileSize). C++: `TDC::ApplyReadPattern` y `ATDCSetPiece::ApplyPattern`.
- Audio: 13 WAV en `Art/Export/Audio` (CC0/ElevenLabs, ver `creditos.md`), `Tools/unreal/import_audio.py` → `/Game/Audio`.
  C++ `TDC::PlaySfx*`: claquetazo, rebote, luces, rebobinado, extras, puente, salto/aterrizaje, "¡Acción!" y lluvia.
- PostProcess con adaptación de exposición rápida (el build arrancaba oscuro).
- Cel shading: `Tools/unreal/apply_toon.py` crea `M_TDCToon` (unlit, 3 bandas, sombra azulada, rim frío) y `MI_Toon_*` para el
  director y los 3 extras (con `Art/Export/Extras/<Tipo>/texture_*.png`). build_dressing: sky light 0.55 y exposure bias 0.
- `PuppetScale` por defecto = 1 (los extras ya miden ~180 cm).
- Si el build falla por audio, reportá `fallo` con `archivo:línea` y no empaquetes.
- Si M_TDCColor no compila o las piezas salen negras/rosas (material roto), reportá `parcial` y **no empaquetes**.

## Pasos
1. `git fetch` y `git checkout t/T-0023-toon` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop` (y cerrá el build de T-0019).
2. `.\Tools\scripts\tdc.ps1 build`.
3. Locks: `git lfs lock Game/Content/Maps/L_Slice.umap` y
   `git ls-files Game/Content/Materials Game/Content/Dressing Game/Content/VFX Game/Content/Characters/Extras Game/Content/Audio Game/Content/Characters | ForEach-Object { git lfs lock $_ }`.
4. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\make_materials.py` (texturas + M_TDCColor con patrón, sin `[TDC] ERROR`).
4b. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\import_audio.py` (13 sonidos en `/Game/Audio`, loops marcados).
5. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\build_dressing.py` (exposición rápida, sky light 0.55). **No** corras
   build_slice: el mapa de main ya tiene los extras a escala y el patrón de las piezas se aplica en BeginPlay.
5b. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\apply_toon.py` (si el commandlet no alcanza, `-InEditor`).
6. `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0023 -Name 01-mejora3-a` y `-Name 01-mejora3-b`.
   Compará con `docs/evidence/T-0019/01-mejora2-b.png`: ¿el director y el extra se ven con cel shading (bandas claras, sombra azulada, nada de silueta negra) y el extra común se lee **rojo**? ¿se ven tablones en las plataformas fijas, sin estirarse? ¿los colores de
   lectura (amarillo/violeta/celeste) se siguen distinguiendo? ¿nada negro ni rosa?
7. `.\Tools\scripts\tdc.ps1 package -Zip stevearagonsite_TheDirectorsCut_mejora3`
8. `.\Tools\scripts\tdc.ps1 smoke -Task T-0023 -Name 02-package -Wait 60 -Zip stevearagonsite_TheDirectorsCut_mejora3`
9. Commit de `Game/Content/**` (LFS), evidencia y Reporte (con tamaño del zip). `git lfs unlock`. Push y `[TDC-REPORT]`.
10. Dejá abierto el build: `.\Tools\scripts\tdc.ps1 smoke -Task T-0023 -Name 03-video -Keep -Zip stevearagonsite_TheDirectorsCut_mejora3`.

## Criterios de aceptación
- [ ] Compila; make_materials, build_slice y build_dressing sin errores.
- [ ] `01-mejora3-b.png`: texturas visibles y colores de lectura intactos; igual o mejor que T-0019.
- [ ] `_mejora3.zip` de menos de 1 GB; el exe corre a los 60 s, sin `Fatal` ni assets faltantes.

## Permite
- Editar `Tools/unreal/*.py` solo para compatibilidad de API.

## No hacer
- No cambiar C++. No pisar los otros zips. No commitear zips ni `Tools/unreal/_diag_*`.

## Evidencia pedida
`01-mejora3-b.png`, `02-package.png`, tamaño del zip y extracto del log de `smoke`.

---

## Reporte (lo completa Windows)
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
