---
id: T-0012
titulo: Director a escala + ambientación noir (último ciclo antes del package)
estado: enviada
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

## Pasos
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
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
