---
id: T-0056
titulo: Rim cálido del director, vereda de adoquín en cuadro, cono más tenue y diagnóstico del bloque gris
estado: parcial
tipo: mixto
rama: t/T-0056-rim-vereda
creada: 2026-11-16
---

# T-0056 — Rim y vereda

## Objetivo
Feedback de T-0052/T-0055:
- `MI_Toon_Director`: RimColor ámbar (1, .6, .28), RimStrength 1.4, RimPower 2. El director tiene que ser lo más cálido del plano.
- Vereda de adoquín: `FOREGROUND_Y` (210, 950), arranca en el frente de los pisos para entrar en cuadro con la cámara de juego.
- `ConeIntensity` 1.8 → 1.0 (C++): el haz tapaba el skyline.
- `list_pieces.py`: diagnóstico del bloque gris liso que aparece a la derecha de `01-escena`.

## Pasos
1. `git fetch` y `git checkout t/T-0056-rim-vereda` (verificá el sha). `git lfs pull`. `.\Tools\scripts\tdc.ps1 stop`.
2. `.\Tools\scripts\tdc.ps1 build`. Si no compila, reportá `fallo` con `archivo:línea`.
3. Locks: `git lfs lock Game/Content/Maps/L_Slice.umap` y
   `git ls-files Game/Content/Materials Game/Content/Dressing Game/Content/Characters | ForEach-Object { git lfs lock $_ }`.
4. `apply_toon.py` → `build_dressing.py`.
5. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\list_pieces.py` y **pegá las primeras ~15 líneas** en el Reporte.
6. `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0056 -Name 01-escena` (cámara de juego). Describí: ¿el director tiene borde
   anaranjado y es lo más cálido del plano? ¿se ve la **vereda de adoquín** abajo? ¿sigue el bloque gris a la derecha?
7. `.\Tools\scripts\tdc.ps1 playtest -Task T-0056` → RESULT PASS. En `*_luces`: ¿el cono ya no tapa el skyline?
8. Commit de `Game/Content/**` (LFS), evidencia y Reporte. `git lfs unlock`. Push y `[TDC-REPORT]`.
9. `.\Tools\scripts\tdc.ps1 play`.

## Criterios de aceptación
- [ ] Compila; scripts sin `[TDC] ERROR`; salida de `list_pieces.py` en el Reporte.
- [ ] Rim anaranjado visible; vereda en cuadro; playtest RESULT PASS.

## Permite
- Editar `Tools/unreal/*.py` solo para compatibilidad de API.

## No hacer
- No cambiar C++. No empaquetar. No commitear `Tools/unreal/_diag_*`.

---

## Reporte (lo completa Windows)
- estado: parcial
- sha: el commit que agrega este reporte (ver `[TDC-REPORT]`)
- resumen: compila (4 acciones, `Result: Succeeded`). `apply_toon.py` (4/4 personajes) y `build_dressing.py` sin `[TDC] ERROR`
  (13 warnings, todos conocidos: smoothing groups de FBX y `Allow CPU Access` de `SM_Ledge_Mid`). Playtest `RESULT PASS 20/20`,
  meta en 16.7 s. Lo visual quedó a medias: el rim se nota pero no es naranja, y la vereda entra en cuadro pero no se lee como adoquín.
- errores: ninguno.
- evidencia: `docs/evidence/T-0056/01-escena.png` y `docs/evidence/T-0056/playtest/` (11 capturas + `playtest.log`).
- notas / preguntas:
  - **Rim del director**: ahora el director se ve cálido (el abrigo pasó de gris, en T-0055, a crema/beige con un lavado ámbar suave
    en todo el cuerpo), pero **no hay un borde anaranjado nítido**: con RimPower 2 el rim se reparte como tinte, no como contorno.
    **No es lo más cálido del plano**: el extra de traje rojo es bastante más saturado, y con L1 prendido el cono amarillo domina
    (`TDCPlaytest_00_inicio.png`, `TDCPlaytest_02_luces.png`).
  - **Vereda**: `DR_ForegroundStage` quedó en Y 210–950 y **entra en cuadro** como franja abajo (~80 px en `01-escena`, ~40 px en el
    playtest). Pero se ve **gris muy claro, casi blanco y liso**: el adoquín apenas se adivina (unas líneas tenues en
    `TDCPlaytest_03_luces.png`). No apareció el warning `DR_ForegroundStage sin patrón`, así que el `pattern_override` se asignó;
    parece lavado por el `color_override` (0.50, 0.52, 0.60) y/o las luces de relleno, no ausente.
  - **Bloque gris a la derecha de `01-escena`**: en esta captura **ya no lo veo**; a la derecha quedan la card de pared de ladrillo, el
    vapor y el borde de un cartel. Lo único gris liso del plano es la franja de la vereda (ver punto anterior). Interpretación mía, sin
    confirmar: el bloque era `DR_ForegroundStage` (11000×740, único actor grande con `color_override` gris en ese rango, junto con
    `Pit_Floor`, que está en Z −850 y fuera de cuadro) y al correrlo a Y 210 pasó de bloque a franja. Aparte, en
    `TDCPlaytest_03_luces.png` hay un panel gris translúcido arriba a la derecha (X≈1050–1240 px); no sé qué actor es, cae fuera del
    rango X 1300–2600 de `list_pieces.py`.
  - **Cono**: con ConeIntensity 1.0 es claramente más translúcido que en T-0055: los edificios y el skyline **se leen a través del haz**
    (`TDCPlaytest_02_luces.png` de T-0056 vs T-0055). Cerca del reflector sigue bastante denso.
  - `list_pieces.py` (X 1300–2600, 19 actores, ordenados por área X×Z), primeras 15 líneas:
    ```
    DR_Parallax_L0 (StaticMeshActor) centro=(4000,-4500,500) tamaño=(17000,2,5000)
    DR_Parallax_L1 (StaticMeshActor) centro=(4000,-3000,300) tamaño=(15000,2,3600)
    DR_Parallax_L2 (StaticMeshActor) centro=(4000,-1600,200) tamaño=(13000,2,2600)
    DR_Card_16_Card_anim_lluvia (StaticMeshActor) centro=(500,300,950) tamaño=(2500,2,2500)
    DR_Card_17_Card_anim_lluvia (StaticMeshActor) centro=(2700,300,950) tamaño=(2500,2,2500)
    DR_ForegroundStage (TDCSetPiece) centro=(3750,580,-90) tamaño=(11000,740,100) presence=ALWAYS record_time=False color_override=(0.50,0.52,0.60,1.0) kit_trim=False
    Pit_Respawn (TDCRespawnVolume) centro=(3500,0,-700) tamaño=(5000,600,200)
    L2 (TDCStageLight) centro=(3050,-163,196) tamaño=(1333,93,448)
    DR_Card_08_Card_mid_pared_ladrillo (StaticMeshActor) centro=(2300,-530,200) tamaño=(1138,2,480)
    Pit_Floor (TDCSetPiece) centro=(3500,0,-850) tamaño=(5000,600,100) presence=ALWAYS record_time=False color_override=(0.55,0.57,0.65,1.0) kit_trim=False
    L1 (TDCStageLight) centro=(1600,-163,196) tamaño=(1111,93,448)
    DR_Card_01_Card_fg_escalera_incendio (StaticMeshActor) centro=(2700,600,575) tamaño=(335,2,650)
    DR_Card_14_Card_anim_vapor (StaticMeshActor) centro=(2000,-200,160) tamaño=(210,2,420)
    Floor_B (TDCSetPiece) centro=(2600,0,-50) tamaño=(800,400,100) presence=ALWAYS record_time=False color_override=(0,0,0,0) kit_trim=True
    DR_Card_09_Card_mid_cartel_pelicula (StaticMeshActor) centro=(2300,-500,150) tamaño=(167,2,260)
    ```
    Resto: `DR_SM_Crates_1`, `P1_LightPlatform` (ONLY_IN_LIGHT), `DR_SetProp_1_SM_FilmCanStack`, `DR_PlayKey_1`.
    (Abrevié la repr de los enums y de `LinearColor`; los valores son los del log.)
  - El stash "T-0053 cancelada" quedó sin tocar. No se cambió C++ ni `Tools/`.
