---
id: T-0012
titulo: Director a escala + ambientación noir (último ciclo antes del package)
estado: parcial
tipo: mixto
rama: t/T-0012-ambientacion
creada: 2026-11-15
---

# T-0012 — Director a escala + ambientación noir

## Objetivo
Último ciclo de PC antes del package de la Epic AI MegaJam (se congela a las 20:00 ART). Tiene que quedar:
- **el director a escala**: los FBX están re-exportados en cm, sin la escala 0.01 del armature, y `AnimatedMeshScale` vuelve a 1;
- **la cámara con look-ahead** (`TargetOffset` X=700), que ya está en `main`;
- **ambientación noir**: backdrop pintado, props de Rodin en silueta y luz de noche, con `build_dressing.py` de gamedesigner.

## Segundo envío (dressing): hacé SOLO esto
El primer envío dejó al director a escala (ver Reporte). Ahora `build_dressing.py` está en la rama. Pasos:
`tdc.ps1 stop` → `git lfs lock Game/Content/Maps/L_Slice.umap` (más los assets nuevos que el script vaya a pisar, si ya
existen) → `tdc.ps1 python -Script Tools\unreal\build_dressing.py` (con `-InEditor` si el commandlet no alcanza) →
captura `tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0012 -Name 02-ambientacion` → commit de `Game/Content/**` + evidencia +
un párrafo "Segundo envío" en el Reporte → `git lfs unlock` → push → `[TDC-REPORT]` → `tdc.ps1 play`.
No hace falta recompilar ni reimportar al director. Si `build_dressing.py` falla, reportá el error exacto (lo arregla la Mac).

## Pasos (primer envío, ya hechos)
0. `.\Tools\scripts\tdc.ps1 stop` (cierra el juego de este proyecto si quedó abierto).
1. Locks de LFS antes de tocar binarios (los scripts fallan si no pueden guardar):
   `git lfs lock Game/Content/Maps/L_Slice.umap`, `git lfs lock Game/Content/Materials/M_TDCColor.uasset` y
   `git lfs lock` de cada `.uasset` en `Game/Content/Characters/Director/`.
2. `.\Tools\scripts\tdc.ps1 build`. Si hay errores de C++, **pará** y reportá con `archivo:línea`.
3. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\import_director.py`. Primero borra los assets viejos del director (rig
   anterior) y reimporta. Tiene que loguear una altura de ~177 cm (no reescala: está dentro de ±10 cm), las animaciones
   `-> AS_Director_*` (Idle, Run, Jump y Clap) y `SM_Clapperboard importado`.
4. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\make_materials.py`.
5. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\build_slice.py`. Si el commandlet no alcanza, agregá `-InEditor`.
6. **Solo si existe** `Tools/unreal/build_dressing.py` en este sha: `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\build_dressing.py`.
   Si no existe, salteá este paso: la ambientación viene en un segundo envío. Si falla, anotá el error y seguí.
7. `.\Tools\scripts\tdc.ps1 test`.
8. Captura: `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0012 -Name 01-sala`. **Tiene que verse el director
   a escala humana**, de altura parecida a una plataforma de 180 cm, con los pies en el piso. Revisala y describí qué se ve:
   el director, el backdrop, los props y más de un piso.
9. Commit de `Game/Content/**` (LFS) + la evidencia + el Reporte. `git lfs unlock` de todo lo que bloqueaste. Push y `[TDC-REPORT]`.
10. Dejá el juego abierto: `.\Tools\scripts\tdc.ps1 play`.

## Criterios de aceptación
- [ ] Compila, los tests pasan y `import_director.py` loguea ~177 cm con Idle, Run, Jump y Clap.
- [ ] La captura muestra al director a escala humana.
- [ ] La ambientación está aplicada, o el fallo queda anotado.
- [ ] Locks liberados y el juego abierto en la PC.

## Permite
- `git lfs lock/unlock` de los archivos listados; `tdc.ps1 stop`.
- Editar `Tools/unreal/*.py` y `Tools/scripts/tdc.ps1`, solo para compatibilidad.

## No hacer
- No corregir C++: reportalo.
- No commitear `Saved/`, `Intermediate/`, `Binaries/`, `*.fbm/` ni cambios de `DefaultEngine.ini` que agregue el motor.
- En el reporte no uses rutas `C:\Users\...`; usá `%USERPROFILE%`.

## Evidencia pedida
`docs/evidence/T-0012/01-sala.png` y los extractos del import (altura y animaciones), de dressing y de tests.

---

## Reporte (lo completa Windows)
- estado: parcial
- sha: 853919e (base)
- resumen: compila; `import_director.py` hace import limpio y loguea **180.0 cm** (no reescala) con Idle, Run (Jogging),
  Jump, Sprint, Walk y Clap, y `SM_Clapperboard importado (alto 27.2 cm)`; `make_materials.py` y `build_slice.py` (commandlet,
  sin `-InEditor`) OK; tests ok=4 fallidos=0. **El director ya se ve a escala humana**: en `01-sala.png` mide lo mismo que el
  cilindro coral de la plataforma, con los pies apoyados en el piso y sombra. La ambientación **no** se aplicó: no existe
  `Tools/unreal/build_dressing.py` en este sha (paso 6 salteado, según la tarea).
- errores: ninguno. Warnings benignos del import: `LogLinker: ... Error opening file` de los assets que el script borra antes de
  reimportar; `FBXImport: No smoothing group information` y tangentes degeneradas en `SM_Clapperboard`.
- evidencia: `docs/evidence/T-0012/01-sala.png`, `docs/evidence/T-0012/log-extractos.txt`.
- notas / preguntas: qué se ve en la captura: el director (gris oscuro, gorra y sobretodo) a la izquierda sobre una plataforma
  blanca, un cilindro coral a la derecha de él, una esfera gris flotando y el borde de un segundo piso entrando por el borde
  derecho (más de un piso, pero el segundo apenas asoma). Cielo y fondo son el cielo azul por defecto: sin backdrop, sin props
  de Rodin y sin luz de noche (falta el segundo envío con `build_dressing.py`). El director queda en el tercio izquierdo, con
  espacio hacia adelante (look-ahead). Locks LFS liberados y el juego quedó abierto (`tdc.ps1 play`).
