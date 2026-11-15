---
id: T-0023
titulo: Package de mejora 3: cel shading de personajes + audio + texturas del set (sobre T-0019)
estado: parcial
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
- estado: parcial
- sha: 155afd7 (contenido, fix de `apply_toon.py` y evidencia)
- resumen: compila sin errores ni warnings. make_materials (4 texturas + M_TDCColor con patrón), import_audio (13/13),
  build_dressing y apply_toon (4/4 personajes) corrieron sin `[TDC] ERROR`. El cel shading funciona: director y extra ya no son
  siluetas negras y el extra común se lee rojo/salmón. **Pero las piezas fijas (tablones) salen casi negras**: en
  `01-mejora3-b.png` la plataforma que en T-0019 era gris claro ahora tiene el techo negro y el frente marrón muy oscuro; los
  tablones casi no se distinguen (apenas las marcas de cinta). Por la regla de Contexto ("piezas negras → parcial y no
  empaquetes") **no empaqueté**: no hay `_mejora3.zip`, ni smoke, ni build abierto (pasos 7, 8 y 10 sin hacer). Los zips
  anteriores no se tocaron.
- errores:
  - `Tools/unreal/apply_toon.py` falló en la primera corrida:
    `[TDC] ERROR: no hay ninguna textura de personaje: correr import_director/import_extras antes` (`apply_toon.py:216`).
    Causa: `eal.does_asset_exist(folder)` devuelve False para carpetas. Cambiado a `does_directory_exist` (ahora línea 214).
  - Segunda corrida: decía "MI_Toon asignado" pero no asignaba nada (SK_Director quedó byte a byte igual, con `model_001`;
    los extras con el slot en `None`, que es como ya estaban en HEAD). En UE 5.8 `for slot in slots` devuelve copias del
    struct `SkeletalMaterial`; ahora se reasigna por índice (`slots[i] = slot`). Verificado leyendo los slots después de guardar.
    Ojo: `fix_director_look.py:68-71` usa el mismo patrón, por eso el director seguía con `model_001` y no con `M_Director`.
  - Audio: en el log del juego (`-game`) no hay ninguna línea de error ni warning de `LogAudio` ni de `/Game/Audio`; solo la
    inicialización del mixer (48000 Hz, 32 voces). `TDC::LoadSfx` carga con `LOAD_NoWarn | LOAD_Quiet`, así que un sonido
    faltante tampoco quedaría en el log: lo único comprobado es que los 13 `.uasset` existen. No lo pude escuchar.
  - build_dressing: 3 warnings de Interchange ("No smoothing group information") en SM_Spotlight/SM_Crates/SM_FilmCamera,
    los mismos de siempre.
- evidencia: `docs/evidence/T-0023/01-mejora3-a.png`, `01-mejora3-b.png` (HighResShot al arrancar) y
  `autorun/TDCAutoRun_00.png` (extra común rojo, piso negro), `autorun/TDCAutoRun_15.png` (pieza amarilla intacta, plataforma
  fija negra). Faltan `02-package.png` y el tamaño del zip (no se empaquetó).
- notas / preguntas:
  - Comparado con `T-0019/01-mejora2-b.png`: personajes mejor (toon), colores de lectura intactos (amarillo visible), nada
    rosa; plataformas fijas peor (negras). El material no está roto: es la cuenta. `T_Stage_Floor.png` es casi carbón y
    BaseColor = Color × lerp(1, Pattern, 0.9), o sea alrededor de 7 veces más oscuro que el color liso de T-0019 (estimado a
    ojo, no medido), bajo una luz noir que ya era baja.
  - El director se ve casi blanco (quemado) a esta distancia: las 3 bandas no se distinguen en la captura. Puede que
    `Brightness` 1.15 + rim sea mucho para un unlit con esta exposición.
  - **Pregunta:** ¿cómo aclaramos los tablones? Opciones: (a) bajar `Strength` de 0.9 en `ATDCSetPiece::ApplyPattern`,
    (b) normalizar el patrón en M_TDCColor (multiplicarlo por una constante para que su media sea ~1), (c) aclarar el PNG.
    Con (b) o (c) alcanza con volver a correr make_materials y empaquetar; (a) es C++.
  - Sin interfaz no pude listar/cerrar procesos fuera de `tdc.ps1 stop` (no había editor abierto); si el build de T-0019
    seguía abierto, sigue abierto. `tdc.ps1 smoke` lo cierra solo cuando se use.
  - Quedaron sin commitear (a propósito): `Tools/unreal/_diag_toon.py` (mi diagnóstico de slots) y 22 capturas más en
    `docs/evidence/T-0023/autorun/`. La primera `01-mejora3-a` salió con los personajes negros porque los shaders del toon
    todavía compilaban; la repetí.
  - Locks LFS liberados al terminar.
