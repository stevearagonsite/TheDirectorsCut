---
id: T-0052
titulo: Kit de plataformas, director legible y fachadas 2D (T-0050 + T-0051 + T-0052)
estado: ok
tipo: mixto
rama: t/T-0052-fachadas
creada: 2026-11-15
---

# T-0052 — Kit, director legible y fachadas

## Objetivo
Sobre main con T-0049 (integración):
- **T-0050:** cornisa del kit (`/Game/Dressing/Kit/SM_Ledge_Mid`) en el borde frontal de las piezas fijas (solo visual).
  `build_dressing` mide la escala y la rotación del FBX y hacia dónde mira la cornisa (`kit_front_yaw`).
- **T-0051:** rim cálido del director y cámara a 1900 (el director medía 11 % del alto de pantalla).
- **T-0052:** fachadas 2D en `cards.json`: cine art déco detrás de la marquesina (que sube a z=265) y pared de ladrillo detrás de cada afiche.
- `tdc.ps1 shot` ahora pasa `-TDCNoMenu`, así la captura sale con la cámara de juego.

## Pasos
1. `git fetch` y `git checkout t/T-0052-fachadas` (verificá el sha). `git lfs pull`. `.\Tools\scripts\tdc.ps1 stop`.
2. `.\Tools\scripts\tdc.ps1 build`. Si no compila, reportá `fallo` con `archivo:línea`.
3. Locks: `git lfs lock Game/Content/Maps/L_Slice.umap` y
   `git ls-files Game/Content/Materials Game/Content/Dressing Game/Content/Characters | ForEach-Object { git lfs lock $_ }`.
4. `make_materials.py` → `build_dressing.py` (pegá las líneas `kit SM_Ledge_Mid: …` y `cornisa: voladizo … -> KitTrimYaw …`) → `apply_toon.py`.
5. `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0052 -Name 01-escena`. Describí: ¿la cornisa mira a la cámara y queda al ras
   de la tapa? ¿se ven las fachadas (cine detrás de la marquesina, ladrillo detrás de los afiches)? ¿el director se distingue mejor (rim cálido)?
   ¿la calle de adoquín se ve abajo?
6. `.\Tools\scripts\tdc.ps1 playtest -Task T-0052` → **RESULT PASS** (no cambia ninguna colisión). Pegá el resultado.
7. Commit de `Game/Content/**` (LFS), evidencia y Reporte. `git lfs unlock`. Push y `[TDC-REPORT]`.
8. Dejá abierto el editor en juego con `.\Tools\scripts\tdc.ps1 play` para que Steve lo vea.

## Criterios de aceptación
- [ ] Compila; scripts sin `[TDC] ERROR`.
- [ ] `01-escena.png` con cámara de juego, cornisas al frente y fachadas visibles.
- [ ] Playtest RESULT PASS.

## Permite
- Editar `Tools/unreal/*.py` y `Tools/scripts/tdc.ps1` solo para compatibilidad de API.

## No hacer
- No cambiar C++. No empaquetar. No commitear `Tools/unreal/_diag_*`.

---

## Reporte (lo completa Windows)
- estado: ok
- sha: 97046cf (contenido + evidencia; el commit siguiente solo agrega este Reporte)
- resumen:
  - **Build OK** (`Result: Succeeded`, 4 acciones, sin errores ni `warning C`).
  - **Locks**: `L_Slice.umap` + 120 assets de `Materials`, `Dressing` y `Characters` (121), soltados al terminar.
  - **Scripts sin `[TDC] ERROR`** (commandlet, sin `-InEditor`): `make_materials.py` → `build_dressing.py` → `apply_toon.py`.
    Líneas pedidas de `build_dressing.py`:
    ```
    [TDC] kit SM_Ledge_Mid: escala 1.0, rotación (0.0, 0.0, 0.0), medidas (268, 121, 80) (error 0.00)
    [TDC] cornisa: voladizo +Y 10 cm, -Y 12 cm -> KitTrimYaw 180
    [TDC] cornisas del kit en 5 piezas fijas
    [TDC] kit SM_Catwalk_Mid: escala 1.0, rotación (0.0, 0.0, 0.0), medidas (483, 103, 160) (error 0.01)
    [TDC] cards 2D colocadas: 20
    ```
    `apply_toon.py`: `toon aplicado a 4/4 personajes`.
  - **Captura `01-escena.png`** con cámara de juego (`-TDCNoMenu` funciona: sale la vista lateral con el HUD de controles).
  - **Playtest: `[TDC-PLAYTEST] RESULT PASS 20/20`**, meta en 16.7 s (log completo en `docs/evidence/T-0052/playtest/playtest.log`).
  - Assets nuevos: `Game/Content/Dressing/Kit/` (SM/M/T de `SM_Ledge_Mid` y `SM_Catwalk_Mid`) y las cards
    `mid_fachada_cine_70` y `mid_pared_ladrillo_65` en `Dressing/Parallax/`.
  - Paso 8: juego abierto con `tdc.ps1 play`.
- errores: ninguno. Warnings no bloqueantes de `build_dressing.py`:
  - Interchange `No smoothing group information was found for this mesh 'model' in the FBX file` en los FBX de Dressing
    (Spotlight, Crates, FilmCamera, StageLight_Hanging, Ledge_Mid, Catwalk_Mid, DirectorChair, BoomMic), igual que en T-0049.
  - `Calling GetSectionFromStaticMesh on /Game/Dressing/Kit/SM_Ledge_Mid but 'Allow CPU Access' is not enabled. This is required
    for converting StaticMesh to ProceduralMeshComponent in cooked builds.` Sale justo antes de la línea `cornisa: voladizo …`
    (medición en editor); en esta tarea no se empaqueta.
- evidencia: `docs/evidence/T-0052/01-escena.png`, `docs/evidence/T-0052/playtest/` (11 capturas + `playtest.log`).
- notas / preguntas (lo que se ve en `01-escena.png`):
  - **Cornisa: sí.** Mira a la cámara, corre por todo el borde frontal de la tarima de inicio y queda al ras de la tapa
    (la tapa clara termina justo donde arranca la moldura, sin escalón ni hueco). Se ven las juntas entre módulos.
  - **Fachadas: sí.** El cine art déco queda detrás de la marquesina "THE DIRECTOR'S CUT" (marquesina arriba de las puertas, sin
    tapar a los personajes). El ladrillo se ve detrás de los afiches en `playtest/TDCPlaytest_02_luces.png` y `_09_luces.png`
    (pared con caños, afiches rotos, ventana con balcón y farol). En `01-escena.png` solo entra el borde de un afiche a la derecha.
  - **Director: mejora parcial.** Con la cámara a 1900 mide ~95 px de 720 (≈13 % del alto) y se separa del fondo azul por
    valor (abrigo gris claro contra edificios oscuros). El rim cálido, en cambio, casi no se nota en esta toma: el borde se lee
    claro/neutro, no anaranjado. El extra rojo se lee más que el director.
  - **Calle de adoquín: no la distingo.** Abajo de todo queda una franja clara de ~8 px en el borde inferior del cuadro; no se
    reconoce textura de adoquín. Si tiene que verse, hay que bajar el encuadre o subir la calle.
  - Observación: a la derecha de `01-escena.png` hay un bloque gris oscuro grande y liso (≈400×350 px) delante del skyline, con un
    reflector apenas visible encima; parece una pieza a oscuras sin textura y es lo que más desentona de la toma.
  - Observación: en las capturas del playtest (`_02`, `_09`) el director sale cortado por el borde inferior del cuadro. Son tomas
    justo después de un teleport del bot, así que puede ser el retardo de la cámara; no lo verifiqué.
  - Sin cambios en `Tools/` ni en C++. Las dos primeras corridas de `shot` fallaron por mi invocación (Git Bash convirtió
    `/Game/Maps/L_Slice` en una ruta de Windows); desde PowerShell salió a la primera. No es un problema del script.
