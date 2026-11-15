---
id: T-0017
titulo: Package de mejora: parallax de 3 capas + VFX de mecánicas (sobre el package final T-0015)
estado: ok
tipo: mixto
rama: t/T-0017-parallax
creada: 2026-11-15
---

# T-0017 — Parallax + VFX + package de mejora

## Objetivo
Pedidos de Steve: fondo con parallax en varias capas y VFX en las mecánicas (T-0018 incluida acá). Es un package **de mejora**: el zip de T-0015
(`Game\Saved\stevearagonsite_TheDirectorsCut.zip`) es la entrega segura y **no se toca**. Este genera
`Game\Saved\stevearagonsite_TheDirectorsCut_mejora.zip`; si la captura se ve peor o algo falla, se entrega T-0015.

## Contexto
- `build_dressing.py` (cc4ff4d): si existen `Art/Export/Dressing/Parallax/{L0_cielo,L1_skyline_lejano,L2_edificios_cercanos}.png`,
  arma 3 planos unlit a Y −4500, −3000 y −1600 en `/Game/Dressing/Parallax/` (L1 y L2 con alpha). Si no, queda el telón único.
- C++ nuevo de T-0018: `ATDCFlipbookFx` (quad con sprite sheet) en el claquetazo, los extras (derrota, aturdido, rebote),
  las luces y el rebobinado. Si faltan los assets de `/Game/VFX`, no hace nada.
- `Tools/unreal/import_vfx.py`: importa `Art/Export/VFX/*.png` a `/Game/VFX/T_*` y arma `/Game/VFX/M_TDCFlipbook`.
- Si el build falla por T-0018, reportá `fallo` con `archivo:línea` y no empaquetes: la Mac reenvía sin VFX.

## Pasos
1. `git fetch` y `git checkout t/T-0017-parallax` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop` (y cerrá el build de T-0015 si quedó abierto).
2. `.\Tools\scripts\tdc.ps1 build`.
3. Locks: `git lfs lock Game/Content/Maps/L_Slice.umap` y **todos** los `.uasset` ya trackeados de Dressing:
   `git ls-files Game/Content/Dressing | ForEach-Object { git lfs lock $_ }` (los props se reimportan y fallan sin lock).
4. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\build_dressing.py`. Tiene que armar las 3 capas sin `[TDC] ERROR`.
4b. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\import_vfx.py` (los `/Game/VFX/*` son nuevos; si ya existieran, lockealos).
   Tiene que crear los 6 `T_*` y `M_TDCFlipbook` sin errores.
5. `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0017 -Name 01-parallax-a` y después `-Name 01-parallax-b`.
   Compará con `docs/evidence/T-0015/01-final.png`: ¿se ven las 3 capas, sin halos, sin huecos ni bordes del plano en cuadro?
   ¿el director y los colores de lectura se siguen leyendo? ¿Ya **no** aparece "LIGHTING NEEDS TO BE REBUILT" (build_dressing
   ahora fuerza luz 100% dinámica)? Si se ve peor, reportá `parcial` y **no empaquetes**.
6. `.\Tools\scripts\tdc.ps1 package -Zip stevearagonsite_TheDirectorsCut_mejora`.
7. `.\Tools\scripts\tdc.ps1 smoke -Task T-0017 -Name 02-package -Wait 60 -Zip stevearagonsite_TheDirectorsCut_mejora`
   (mismos chequeos que T-0015: corre a los 60 s, log sin `Fatal` ni assets faltantes).
8. Commit de `Game/Content/**` (LFS, incluye `/Game/VFX`), evidencia y Reporte (con el tamaño del zip). `git lfs unlock` de lo bloqueado. Push y `[TDC-REPORT]`.
9. Dejá abierto el build de mejora: `.\Tools\scripts\tdc.ps1 smoke -Task T-0017 -Name 03-video -Keep -Zip stevearagonsite_TheDirectorsCut_mejora`.

## Criterios de aceptación
- [ ] Compila con T-0018; `import_vfx.py` crea los 6 sheets y `M_TDCFlipbook`.
- [ ] `01-parallax-b.png` muestra las 3 capas y se ve igual o mejor que T-0015.
- [ ] `..._mejora.zip` de menos de 1 GB; el exe corre a los 60 s, sin `Fatal` ni assets faltantes.
- [ ] En `02-package.png` no aparece "LIGHTING NEEDS TO BE REBUILT".
- [ ] El zip de T-0015 quedó intacto.

## Permite
- Editar `Tools/unreal/build_dressing.py` solo para compatibilidad de API, sin cambiar profundidades ni tamaños.

## No hacer
- No pisar ni borrar `Game\Saved\stevearagonsite_TheDirectorsCut.zip`. No cambiar C++. No commitear zips ni `Tools/unreal/_diag_*`.

## Evidencia pedida
`01-parallax-b.png`, `02-package.png`, tamaño del zip y extracto del log de `smoke`.

---

## Reporte (lo completa Windows)
- estado: ok
- sha: el commit que contiene este reporte (va en el `[TDC-REPORT]`); partí de 47dee7e.
- resumen: compila con T-0018 (9 acciones, `Result: Succeeded`, sin errores ni warnings de `Game/Source`).
  `build_dressing.py` armó el parallax de 3 capas y dejó la luz 100% dinámica; `import_vfx.py` creó los 6 `T_*` y
  `M_TDCFlipbook`. Package de mejora generado: `Game\Saved\stevearagonsite_TheDirectorsCut_mejora.zip`, **329.6 MB**
  (345.643.337 bytes). El exe corre a los 60 s, sin `Fatal` ni assets faltantes, y ya no aparece
  "LIGHTING NEEDS TO BE REBUILT". El zip de T-0015 quedó intacto (343.931.514 bytes, misma fecha y mismo sha256
  `0cc8f0c2…aece9c` antes y después). El build de mejora quedó abierto (`smoke -Name 03-video -Keep`).
- errores: ninguno. Extractos:
  - `build_dressing.py`: `[TDC] parallax de 3 capas armado` · `[TDC] luz 100% dinámica (force_no_precomputed_lighting,
    luces Movable).` · `[TDC] ambientación noir aplicada y L_Slice guardado.` · `Success - 0 error(s), 3 warning(s)`
    (los 3 son el aviso de smoothing groups de los FBX de props, ya conocido).
  - `import_vfx.py`: `[TDC] 6 sheets importadas: T_claquetazo_impact, T_extra_defeat, T_extra_stun, T_light_toggle,
    T_rewind, T_scripted_bounce` · `[TDC] M_TDCFlipbook listo` · `Success - 0 error(s), 0 warning(s)`.
  - `package`: `BUILD SUCCESSFUL` · `AutomationTool exiting with ExitCode=0 (Success)`.
  - `smoke` (corrida válida): `seguía corriendo a los 60 s: sí` · `LoadMap: /Game/Maps/L_Slice?Name=Player` ·
    `Load map complete /Game/Maps/L_Slice`. Sin `Fatal`, `Error:` ni `Couldn't find`. Único warning de carga:
    `LoadPackage: SkipPackage: /Game/Characters/Director/BP_Director` (la carga silenciosa de `TDCGameMode`; ese asset no existe en el repo).
- evidencia: `docs/evidence/T-0017/`
  - `01-parallax-b.png` (editor `-game`): se ven las 3 capas (cielo con luna, skyline lejano claro, edificios cercanos),
    sin halos, sin huecos ni bordes de plano en cuadro; director, cilindro rojo, reflector y plataforma se leen bien.
    Sin "LIGHTING NEEDS TO BE REBUILT". Lo veo mejor que `T-0015/01-final.png`.
  - `01-parallax-a.png`: la primera corrida salió con la pared gris sin el parallax (captura antes de que carguen las
    texturas nuevas); se deja como referencia, la buena es la `b`.
  - `02-package.png` (zip de mejora): mismas 3 capas, sin el aviso de lighting.
  - `03-video.png`: el build que quedó abierto.
  - `autorun/TDCAutoRun_01|04|13.png` (editor `-game -TDCAutoRun`): el parallax en movimiento; las capas se desplazan
    distinto entre sí y no aparecen bordes.
- notas / preguntas:
  - **`smoke` se corrió 3 veces.** (1) La primera captura salió solo con el degradado azul (el caso que ya anotaba
    T-0015 para el frame 0); el juego corría bien a los 60 s. (2) En la segunda la captura salió bien pero el juego
    **no** seguía a los 60 s: no fue crash, el log dice `Window ... being destroyed` →
    `RequestExit(0, UGameEngine::Tick.ViewportClosed)`, o sea se cerró la ventana (había input en la máquina: en los
    logs se ven reflectores prendiéndose y apagándose). Sin carpeta `Crashes`. (3) La tercera es la válida.
  - **Sin verificar:** la captura de `smoke` es del frame 0 y en este build sale **sin el HUD de controles** y más
    oscura que en el editor (el director queda casi en silueta contra los edificios, el cielo más nocturno). En T-0015
    la misma captura sí mostraba el HUD. No pude sacar una captura posterior del build empaquetado (corriendo sin
    interfaz no pude capturar la pantalla ni lanzar otra instancia), así que **no hay evidencia de que el HUD se vea en
    el package de mejora ni de cómo se lee el director pasado el primer frame**. Conviene que Steve lo mire en el build
    que quedó abierto antes de elegir este zip sobre el de T-0015.
  - Los VFX (`ATDCFlipbookFx`) tampoco quedaron capturados: los assets están y cocinan sin errores, pero no hay
    captura de un claquetazo o rebobinado con el flipbook en pantalla.
  - Locks: se bloquearon `L_Slice.umap` y los 26 `.uasset` de `Game/Content/Dressing`; se liberan después del push.
    Cambiaron `L_Slice.umap` y los 3 `SM_*` de props (reimport); nuevos: `Dressing/Parallax/*` (6) y `VFX/*` (7).
  - No toqué C++ ni `build_dressing.py`. No se commitean zips ni `Tools/unreal/_diag_*`.
