---
id: T-0019
titulo: Package de mejora 2: extras animados + cards 2D (sobre T-0017)
estado: parcial
tipo: mixto
rama: t/T-0019-extras
creada: 2026-11-15
---

# T-0019 — Extras animados + cards 2D + package de mejora 2

## Objetivo
Pedido de Steve: aprovechar el 2.5D combinando recursos 2D (cards de primer plano y plano medio, vapor y lluvia animados) y
extras animados en lugar del greybox. Genera `Game\Saved\stevearagonsite_TheDirectorsCut_mejora2.zip`.
**No toques** los zips `stevearagonsite_TheDirectorsCut.zip` (T-0015, entrega segura) ni `..._mejora.zip` (T-0017).

## Contexto
- Contiene todo T-0017 (parallax, VFX, claquetazo de 300 ms, luz dinámica) + T-0020 (cards desde
  `Art/Export/Dressing/Cards/cards.json`, 16 cards) + extras de Rodin (`ATDCExtra` con Puppet skeletal,
  `Tools/unreal/import_extras.py`, `Art/Export/Extras/*.fbx`).
- Los flipbooks de las cards usan `M_TDCFlipbook`: **import_vfx corre antes que build_dressing**.
- Si el build falla, reportá `fallo` con `archivo:línea` y no empaquetes.

## Pasos
1. `git fetch` y `git checkout t/T-0019-extras` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop` (y cerrá el build de T-0017 si quedó abierto).
2. `.\Tools\scripts\tdc.ps1 build`.
3. Locks: `git lfs lock Game/Content/Maps/L_Slice.umap` y todos los `.uasset` ya trackeados de Dressing, VFX y Extras:
   `git ls-files Game/Content/Dressing Game/Content/VFX Game/Content/Characters/Extras | ForEach-Object { git lfs lock $_ }`.
4. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\import_vfx.py`
5. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\import_extras.py` (tiene que importar Comun, Resistente y Sombra con Idle y Walk).
6. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\build_dressing.py` (parallax + cards + luz dinámica, sin `[TDC] ERROR`).
7. `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0019 -Name 01-mejora2-a` y después `-Name 01-mejora2-b`.
   Describí: ¿se ven cards de primer plano y plano medio (farolas, marquesina, afiches), lluvia o vapor? ¿el extra rojo es
   un personaje animado de **tamaño humano** (ni gigante ni diminuto) y mira hacia donde camina? ¿el director y los colores de
   lectura se siguen leyendo? ¿no aparece "LIGHTING NEEDS TO BE REBUILT"?
7b. **Si los extras no miden ~180 cm** (como el director): corregilos sin recompilar y repetí el shot:
   `$env:TDC_PUPPET_SCALE = "1"` si se ven gigantes, o `"10000"` si se ven diminutos, y después
   `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\set_extra_scale.py`. Anotá el valor que quedó.
8. `.\Tools\scripts\tdc.ps1 package -Zip stevearagonsite_TheDirectorsCut_mejora2`
9. `.\Tools\scripts\tdc.ps1 smoke -Task T-0019 -Name 02-package -Wait 60 -Zip stevearagonsite_TheDirectorsCut_mejora2`
10. Commit de `Game/Content/**` (LFS), evidencia y Reporte (con el tamaño del zip). `git lfs unlock`. Push y `[TDC-REPORT]`.
11. Dejá abierto el build: `.\Tools\scripts\tdc.ps1 smoke -Task T-0019 -Name 03-video -Keep -Zip stevearagonsite_TheDirectorsCut_mejora2`.

## Criterios de aceptación
- [ ] Compila; los 3 imports y build_dressing sin errores.
- [ ] `01-mejora2-b.png`: cards 2D visibles, extra animado a escala humana, sin aviso de luces.
- [ ] `_mejora2.zip` de menos de 1 GB; el exe corre a los 60 s, sin `Fatal` ni assets faltantes.
- [ ] Los zips de T-0015 y T-0017 quedaron intactos.

## Permite
- Editar `Tools/unreal/*.py` solo para compatibilidad de API.

## No hacer
- No cambiar C++. No pisar los otros zips. No commitear zips ni `Tools/unreal/_diag_*`.

## Evidencia pedida
`01-mejora2-b.png`, `02-package.png`, tamaño del zip y extracto del log de `smoke`.

---

## Reporte (lo completa Windows)
- estado: parcial
- sha: el commit que contiene este reporte (va en el `[TDC-REPORT]`); partí de e93d867.
- resumen: Compila (6 acciones, sin errores ni warnings de C++). `import_vfx` (6 `T_*` + `M_TDCFlipbook`), `import_extras`
  (Comun, Resistente y Sombra, cada uno con `SK_`, `SKEL_`, `M_`, `AS_..._Idle` y `AS_..._Walk`) y `build_dressing`
  ("parallax de 3 capas armado", "cards 2D colocadas: 16", "luz 100% dinámica") sin `[TDC] ERROR`.
  Con `PuppetScale = 100` los extras salían **gigantes** (un zapato tapaba medio cuadro): apliqué el paso 7b y quedó
  **`puppet_scale = 1`** en los 3 extras de L_Slice; ahora miden lo mismo que el director.
  Package OK: `Game\Saved\stevearagonsite_TheDirectorsCut_mejora2.zip`, **342.9 MB**. Smoke: corre a los 60 s, sin `Fatal`.
  Los zips de T-0015 y T-0017 quedaron intactos (mismo SHA256 antes y después: `0CC8F0C2…` y `66350B6D…`).
  Queda `parcial` y no `ok` porque las capturas fijas no alcanzan para confirmar todo lo que pide el paso 7 (ver abajo).
- Lo que se ve en `01-mejora2-b.png` / `02-package.png`:
  - Cards: farola en primer plano (centro), marquesina "THE DIRECTOR'S CUT" (arriba a la izquierda), un afiche (arriba a la
    derecha) y vapor (derecha, sobre la escalera de incendio). **No distingo lluvia.** La marquesina y el afiche se leen
    **flotando en el cielo**, sin edificio ni poste que los sostenga. No veo en cuadro la cortina, la escalera de incendio
    de primer plano ni el segundo cartel.
  - Extra: hay una figura de tamaño humano junto al reflector, de la altura del director. Se ve como **silueta oscura, no
    roja** (está en la zona sin luz de la plataforma). Con una captura fija **no puedo confirmar que esté animado ni que
    mire hacia donde camina**.
  - El director se lee en `01-mejora2-b.png`; en `02-package.png` (primer cuadro del exe) director y extra quedan muy
    oscuros. No aparece "LIGHTING NEEDS TO BE REBUILT". El HUD de controles no llegó a dibujarse en ese primer cuadro.
  - `01-mejora2-a.png` es de antes del paso 7b: muestra el extra gigante y cards todavía sin textura (negras).
- errores:
  - `Tools/unreal/set_extra_scale.py:33` — `TypeError: isinstance() arg 2 must be a type, a tuple of types, or a union`
    (`unreal.load_class` devuelve un `unreal.Class`, no un tipo de Python). Lo cambié por `unreal.TDCExtra`
    (compatibilidad de API, dentro de `Permite`).
  - Log del package (no fatal): `LogStreaming: Warning: LoadPackage: SkipPackage: /Game/Characters/Director/BP_Director
    - The package to load does not exist on disk or in the loader`.
  - Avisos de import sin efecto: `No smoothing group information was found` en los 3 FBX de extras y los 3 de props.
- evidencia: `docs/evidence/T-0019/` — `01-mejora2-a.png`, `01-mejora2-b.png`, `02-package.png`.
  Extracto del smoke:
  ```
  seguía corriendo a los 60 s: sí
  LogLoad: LoadMap: /Game/Maps/L_Slice?Name=Player
  LogLoad: Took 0.099041 seconds to LoadMap(/Game/Maps/L_Slice)
  LogGlobalStatus: UEngine::LoadMap Load map complete /Game/Maps/L_Slice
  ```
- notas / preguntas:
  - El **primer** smoke dio "seguía corriendo a los 60 s: NO". No fue un crash: el log muestra salida limpia a los ~44 s
    (`RequestExit(0, UGameEngine::Tick.ViewportClosed)`), con entradas de juego antes (luces alternadas, un claquetazo), o
    sea que alguien usó y cerró la ventana en la PC. Lo repetí y pasó; el reporte usa la segunda corrida.
  - El worker sin interfaz no puede definir variables de entorno desde el shell, así que `TDC_PUPPET_SCALE=1` lo pasé con
    un wrapper local sin commitear (`Tools/unreal/_diag_puppet_scale_1.py`) que setea la variable y corre
    `set_extra_scale.py`. Conviene que el default de C++ (`PuppetScale = 100.f` en `TDCExtra.h:59`) pase a 1: hoy el valor
    correcto vive solo en el `.umap`, y cualquier extra nuevo va a nacer gigante.
  - Esta rama sale de 47dee7e y **no contiene** el commit de reporte de T-0017 (cb3017b), que ya había agregado
    `Game/Content/VFX/**`, `Game/Content/Dressing/Parallax/**` y su `L_Slice.umap`. Acá se regeneraron y se commitean de
    nuevo: al mergear las dos ramas va a haber conflicto de binarios; hay que quedarse con los de T-0019.
  - Step 11: el build de mejora 2 queda abierto con `smoke -Keep` (`03-video`).
  - Preguntas: ¿la marquesina y el afiche flotando son la posición buscada en `cards.json`? ¿Querés una corrida
    `autorun` para ver el extra caminando en varios cuadros?
