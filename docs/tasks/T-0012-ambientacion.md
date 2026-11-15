---
id: T-0012
titulo: Director a escala + ambientación noir (último ciclo antes del package)
estado: ok
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

## Pasos (tercer envío: C++ de locomoción + look del director + dressing noir)
Despacha la Mac (thedirectorscut-48). Si algo de C++ no compila, **pará** y reportá `fallo` con `archivo:línea` y el mensaje
exacto: gamedesigner lo arregla enseguida. No reviertas nada por tu cuenta.
1. `git pull`. `.\Tools\scripts\tdc.ps1 stop`.
2. `.\Tools\scripts\tdc.ps1 build` (trae c73baf5: locomoción walk/run/sprint, BlendSpace opcional, pose aérea, aviso de luces).
3. Locks: `git lfs lock Game/Content/Maps/L_Slice.umap` y, **si existen**, `Game/Content/Characters/Director/M_Director.uasset`
   y `Game/Content/Characters/Director/BS_Director_Locomotion.uasset`, más los `.uasset` de `Game/Content/Dressing/` que ya estén en el repo.
4. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\fix_director_look.py` (M_Director sin metallic + BS_Director_Locomotion).
5. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\build_dressing.py` (post-proceso noir, niebla lejana, telón y los props
   `Art/Export/Dressing/SM_*.fbx` que haya). Si el commandlet no alcanza, agregá `-InEditor`.
6. `.\Tools\scripts\tdc.ps1 test`.
7. Dos capturas: `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0012 -Name 03-noir-a` y después `-Name 03-noir-b`
   (la primera puede salir con shaders compilando; la que vale es la b). Describí qué se ve: director con material difuso (no
   metálico), telón, props, post-proceso oscuro pero legible, colores de lectura todavía distinguibles.
8. Commit de `Game/Content/**` (LFS) + evidencia + Reporte, `git lfs unlock` de todo lo bloqueado, push y `[TDC-REPORT]`.
9. `.\Tools\scripts\tdc.ps1 play` (dejar el juego abierto).

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

### Segundo envío (sha 0949cdf)
- estado: ok (con observaciones de arte para la Mac)
- resumen: `build_dressing.py` corrió por commandlet (sin `-InEditor`). La 1ª corrida aplicó todo y guardó `L_Slice`, pero
  salió con código 1 por `LogEditorAssetSubsystem: Error: LoadAsset failed ... /Game/Dressing/M_Backdrop` (el script hace
  `eal.load_asset` de un asset que todavía no existe). Fix de compatibilidad en `Tools/unreal/build_dressing.py`:
  `does_asset_exist` antes de `load_asset`. 2ª corrida: `Success - 0 error(s), 0 warning(s)`. Assets nuevos:
  `Game/Content/Dressing/T_Backdrop_Noir.uasset` y `M_Backdrop.uasset` (unlit, Emissive ← textura × 0.85, verificado).
  En el mapa: `DR_Backdrop` en (3750, -3000, 1500) con `M_Backdrop`, y `DR_ForegroundStage` en Y=700. **Props: ninguno**:
  `Art/Export/Dressing/` solo tiene `T_Backdrop_Noir.png`, no hay `SM_*.fbx` (el script los omite).
- errores: ninguno en la corrida final.
- evidencia: `docs/evidence/T-0012/02-ambientacion.png`, extractos agregados a `docs/evidence/T-0012/log-extractos.txt`.
- notas / preguntas: qué se ve en `02-ambientacion.png`: detrás del nivel, el telón pintado (skyline de rascacielos con
  ventanas encendidas y niebla, en azules) ocupando casi todo el cuadro; el director a escala sobre la plataforma blanca,
  el cilindro coral y la esfera; abajo, la franja oscura del escenario en primer plano. Observaciones:
  1. **No se lee como noche**: el conjunto queda claro y lavado (telón celeste, plataformas blancas). Sospecho de la
     exposición automática compensando la luz baja; para noche haría falta fijar exposición (PostProcess con
     `AutoExposure` manual/bias negativo) o bajar más el multiplicador del telón.
  2. Se ven los bordes del telón: el izquierdo (franja de cielo claro a la izquierda) y el inferior (banda clara entre el
     telón y la franja del escenario, a la altura del piso). Habría que agrandarlo/bajarlo o tapar con el escenario.
  3. `tdc.ps1 shot` captura apenas arranca: con materiales nuevos la 1ª toma salió con el material por defecto (shaders
     compilando). Repetí la toma. Sugerencia: un warm-up/delay antes de `HighResShot`.
  4. Quedaron dos archivos temporales sin trackear en la PC (`Tools/unreal/_diag_dressing.py`, `_diag_out.txt`); no se
     commitean y el permiso de esta sesión no me dejó borrarlos.
  Locks LFS liberados y el juego quedó abierto (`tdc.ps1 play`).

### Tercer envío (sha 6e934fa)
- estado: ok (con observaciones de arte)
- resumen: `tdc.ps1 build` OK (TDCCharacter, TDCCharacterAnimInstance, TDCExtra, TDCGameMode, TDCStageTriggers; sin
  errores). `fix_director_look.py` por commandlet: `M_Director asignado a 1 slot(s) (sin metallic)` y
  `BS_Director_Locomotion con 4 muestras` (Idle 0, Walk 160, Run 380, Sprint 600). `build_dressing.py` por commandlet (sin
  `-InEditor`): `Success - 0 error(s), 1 warning(s)`, importó `SM_Spotlight` y lo ubicó dos veces (DR_SM_Spotlight_0 y _3);
  `SM_FilmCamera` y `SM_Crates` omitidos (no hay FBX); post-proceso `DR_PostProcess` y niebla lejana aplicados. Tests ok=4
  fallidos=0.
- fixes de compatibilidad en `Tools/unreal/fix_director_look.py`:
  1. `does_asset_exist` antes de `load_asset(M_Director)` (mismo problema que en build_dressing: el primer run logueaba
     `LoadAsset failed` y el commandlet salía con 1).
  2. La 1ª corrida creó M_Director y el BlendSpace pero **no guardó `SK_Director`** (en commandlet `set_editor_property` no
     marca el paquete dirty y `save_asset` lo salteaba). Agregué `sk.modify()` + `save_asset(..., only_if_is_dirty=False)`;
     2ª corrida: `SK_Director.uasset` modificado con M_Director en su slot.
- errores: ninguno. Warning: `LogInterchangeEngine: ... SM_Spotlight.fbx ... No smoothing group information`.
- assets: nuevos `Characters/Director/M_Director`, `Characters/Director/BS_Director_Locomotion`, `Dressing/SM_Spotlight` y
  los que el import FBX de Rodin creó al lado con nombres genéricos: `Dressing/model` (material), `Dressing/texture_diffuse`,
  `texture_metallic`, `texture_normal`, `texture_roughness`. Modificados: `SK_Director`, `L_Slice`, `M_Backdrop`,
  `T_Backdrop_Noir` (reimport).
- evidencia: `docs/evidence/T-0012/03-noir-a.png`, `docs/evidence/T-0012/03-noir-b.png` (las dos salieron iguales: los
  shaders ya estaban compilados por los commandlets), extractos en `docs/evidence/T-0012/log-extractos.txt`.
- notas / qué se ve en `03-noir-b.png`:
  1. **Mucho más noir que 02**: el telón ocupa el fondo en azules oscuros y bruma pintada (en este encuadre se ve la parte de
     niebla/pinceladas, no el skyline), viñeta y grano visibles, la franja del escenario abajo casi negra. Oscuro pero legible.
  2. **Director**: silueta oscura (gorra, sobretodo) en el tercio izquierdo sobre la plataforma; se ve mate, sin el brillo
     metalizado de antes, aunque a esta luz casi no se distingue la textura (queda muy en sombra).
  3. **Colores de lectura**: el cilindro coral se distingue bien; la plataforma gris cálida y la esfera oscura (sobre el
     telón se lee apenas por contraste).
  4. **Props**: del SM_Spotlight de la izquierda solo asoman las patas del trípode, delante de la plataforma, en el borde
     inferior izquierdo; el cabezal queda fuera de cuadro (X=-150, más allá del borde izquierdo de la cámara). El de X=6900
     no entra en esta toma.
  5. Sigue una **línea celeste clara** horizontal entre el telón y la franja del escenario, a la altura de la base de la
     plataforma (parece el borde superior de la franja `DR_ForegroundStage` iluminado o el piso de fondo).
  6. Los assets genéricos del import de Rodin (`model`, `texture_*`) en `/Game/Dressing` van a chocar con el próximo prop
     de Rodin que se importe ahí; sugiero importar cada prop en su subcarpeta o renombrar.
  Locks LFS liberados y el juego quedó abierto (`tdc.ps1 play`). Siguen sin trackear `Tools/unreal/_diag_*` (no se commitean).
