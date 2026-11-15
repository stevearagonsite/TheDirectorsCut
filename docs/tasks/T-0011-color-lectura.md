---
id: T-0011
titulo: Build integrado para la jam (director + claquetazo + cámara/cielo + colores de lectura)
estado: parcial
tipo: mixto
rama: t/T-0011-color-lectura
creada: 2026-11-15
---

# T-0011 — Build integrado: T-0007 + T-0009 + T-0010 + colores de lectura

## Objetivo
Un único ciclo de PC antes del package de la Epic AI MegaJam (21:00 ART). La rama contiene en su historia:
- **T-0007**: el director de Rodin con animaciones (`import_director.py`, `Art/Export/Director/*.fbx`).
- **T-0009**: claquetazo y extras. `build_slice.py` suma X1, X2 y X3; además vienen la claqueta y la animación del golpe.
- **T-0010**: cámara con brazo de 2200 y FOV 60; SkyAtmosphere, SkyLight y niebla; AndroidFileServer apagado.
- **T-0011**: los colores de lectura (luz, oscuridad, toma y extras) salían grises en 5.8. Ahora usan `/Game/Materials/M_TDCColor`,
  que crea `make_materials.py`, y el bulbo del reflector se ve emisivo cuando está prendido.

## Pasos
0. Si sigue abierto el juego de T-0008, cerralo antes de compilar (Steve ya lo vio; Live Coding bloquea el build):
   `.\Tools\scripts\tdc.ps1 stop` (cierra solo el editor/juego de este proyecto y su LiveCodingConsole). Si existe `docs/tasks/T-0008-reporte.tmp`, borralo; no se commitea.
1. `.\Tools\scripts\tdc.ps1 build`. Si hay errores de C++, **pará** y reportá `estado: fallo` con `archivo:línea` y el mensaje
   de los primeros errores reales.
2. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\make_materials.py`: crea `M_TDCColor`.
3. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\import_director.py`: tiene que loguear la altura (~180 cm), las
   animaciones `-> AS_Director_*` (Idle, Run, Jump y Clap) y `SM_Clapperboard importado`.
4. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\build_slice.py`. Si el commandlet no alcanza, agregá `-InEditor`.
5. `.\Tools\scripts\tdc.ps1 test`: anotá el resultado, que incluye `TDC.Combat.ResolveClap`.
6. Capturas: `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0011 -Name 01-sala`. Tienen que verse el director
   (no la cápsula), los colores (amarillo, violeta, celeste y verde), el cielo y más de un piso en cuadro.
7. Commit de `Game/Content/**` (mapas, materiales y `Characters/Director`, todo por LFS), la evidencia y el Reporte de esta tarea.
   En `T-0007-personaje-skeletal.md` y `T-0009-claquetazo.md` poné `estado:` y una línea de reporte que remita a este archivo.
   Push y `[TDC-REPORT T-0011]`.
8. Dejá el juego abierto: `.\Tools\scripts\tdc.ps1 play`.

## Criterios de aceptación
- [ ] Compila sin errores y los tests pasan (o el fallo queda anotado).
- [ ] `M_TDCColor`, `SK_Director`, `AS_Director_Idle`, `AS_Director_Run`, `AS_Director_Jump` y `SM_Clapperboard` existen.
- [ ] La captura muestra al director, los colores de lectura y el cielo.
- [ ] El juego quedó abierto en la PC.

## Permite
- Cerrar el editor/juego de este proyecto con `tdc.ps1 stop` (no el sandbox TDCPreview).
- Editar `Tools/unreal/*.py` y `Tools/scripts/tdc.ps1`, solo para compatibilidad.

## No hacer
- No corregir C++: reportalo.
- No commitear `Saved/`, `Intermediate/`, `Binaries/`, `.tmp` ni cambios de `DefaultEngine.ini` que agregue el motor (por ejemplo,
  `SecurityToken`).
- En el reporte no uses rutas `C:\Users\...`; usá `%USERPROFILE%`.

## Evidencia pedida
`docs/evidence/T-0011/01-sala.png`, más los extractos del import (altura y animaciones) y de los tests.

---

## Reporte (lo completa Windows)
- estado: parcial
- sha: 69acf21 (validado) + commit de este reporte
- resumen:
  - Paso 0: `tdc.ps1 stop` → "no había editor/juego de este proyecto abierto". No existía `T-0008-reporte.tmp`.
    En el clon había un `Game/Content/Maps/L_Slice.umap` sin trackear, idéntico (mismo blob LFS) al de la rama; se pisó con el checkout.
  - Paso 1: build OK (`Result: Succeeded`, 14 acciones, sin errores).
  - Paso 2: `make_materials.py` OK → `/Game/Materials/M_TDCColor`.
  - Paso 3: `import_director.py` OK: `altura importada: 180.0 cm`; `Idle -> AS_Director_Idle`, `Jogging -> AS_Director_Run`,
    `Jumping -> AS_Director_Jump`, `Two_Cycle_Sprint -> AS_Director_Sprint`, `Walking -> AS_Director_Walk`,
    `AS_Director_Clap -> AS_Director_Clap`; `SM_Clapperboard importado (alto 27.2 cm)`. Warnings FBX: sin smoothing groups;
    SM_Clapperboard con tangentes degeneradas.
  - Paso 4: `build_slice.py` loguea "construido y guardado" pero **no guardaba**: `L_Slice.umap` es LFS lockable y queda
    read-only (`Unable to save package ... because the file is read-only!`, el mensaje no falla el script). Con
    `git lfs lock Game/Content/Maps/L_Slice.umap` + `-InEditor` se guardó bien. (Sugerencia: que `build_slice.py` verifique el
    resultado de `save_current_level()` y falle.)
  - Paso 5: tests `ok=4 fallidos=0`: TDC.Combat.ResolveClap, TDC.Stage.Lighting.IsLit, TDC.Stage.Lighting.Presence, TDC.Stage.Time.History.
  - Paso 6: captura `01-sala.png`. Se ven el cielo (SkyAtmosphere + niebla), Floor_A y el extra X1 rojo. **No cumple**:
    (a) el director no se lee: en la posición del jugador (centro, sobre el piso) hay solo una marca gris de pocos píxeles,
    mucho más chica que el extra de 170 cm; no se ve ni el director a escala ni la cápsula greybox. Sospecha: las animaciones
    (Mixamo/Rodin) traen escala 0.01 en el root/armature y achican el mesh al reproducirse, aunque el ref pose mida 180 cm;
    (b) un solo piso en cuadro (Floor_B empieza en X=2200 y queda fuera); (c) de los colores solo se ve el rojo del extra:
    la plataforma amarilla P1 es ONLY_IN_LIGHT con L1 apagado, el resto está fuera de cuadro.
  - Paso 7: commit de `Game/Content/` (L_Slice, Materials/M_TDCColor, Characters/Director/*) por LFS, evidencia y reportes de
    T-0007 (parcial) y T-0009 (ok). No se commiteó `Art/Export/Director/Director_rigged_anims.fbm/` (texturas que extrae el import).
  - Paso 8: juego abierto con `tdc.ps1 play`.
- errores: ninguno de compilación ni de tests. Paso 4: `Unable to save package to .../Game/Content/Maps/L_Slice.umap because the file is read-only!` (resuelto con lock).
- evidencia: `docs/evidence/T-0011/01-sala.png`; extractos de import y tests arriba.
- notas / preguntas: 1) revisar escala del director en juego (animaciones con escala de armature; probar reimport con
  `convert_scene`/`remove_root_motion` o forzar escala 1 en el root de las AS_*), 2) encuadre: con brazo 2200 y FOV 60 desde
  PlayerStart (X=200) no entra un segundo piso; ¿mover PlayerStart o la cámara? 3) `L_Slice.umap` se lockeó para el paso 4 y se
  desbloqueó después del push.
