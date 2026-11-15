---
id: T-0013
titulo: Props de Rodin y telón sin línea clara (último pulido antes del package final)
estado: parcial
tipo: mixto
rama: t/T-0013-props-pulido
creada: 2026-11-15
---

# T-0013 — Props y pulido del telón

## Objetivo
Sumar los props de Rodin (cajas y cámara de cine) repartidos por toda la sala y sacar la línea clara de la base del telón.
Es el último cambio de contenido antes del package final (freeze 20:00). Trae un C++ chico (09bfb9e): modo demo
`-TDCAutoRun` para validar la locomoción sin jugador (idle → corre → salta, 12 capturas).

## Contexto
- `Tools/unreal/build_dressing.py` (fb69d55): importa cada prop en `/Game/Dressing/<SM_x>/` (así no se pisan `model`,
  `texture_*`), reparte 6 props en X 700, 1900, 3300, 4600, 5700 y 7000, y M_Backdrop oscurece el tercio inferior.
- FBX nuevos: `Art/Export/Dressing/SM_Crates.fbx`, `Art/Export/Dressing/SM_FilmCamera.fbx`.
- Los assets genéricos que quedaron sueltos en `/Game/Dressing/` (T-0012) no se tocan ni se borran ahora.

## Pasos
1. `git fetch` y `git checkout t/T-0013-props-pulido` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop`.
2. `.\Tools\scripts\tdc.ps1 build`. Si no compila, **pará** y reportá `fallo` con `archivo:línea` (no reviertas).
3. Locks: `git lfs lock Game/Content/Maps/L_Slice.umap` y `git lfs lock Game/Content/Dressing/M_Backdrop.uasset`.
4. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\build_dressing.py`. Tiene que importar `SM_Crates`, `SM_FilmCamera` y
   `SM_Spotlight` en sus subcarpetas, sin `[TDC] ERROR`. Si el commandlet no alcanza, agregá `-InEditor`.
5. Dos capturas: `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0013 -Name 01-props-a` y después `-Name 01-props-b`
   (la b es la que vale). Describí: ¿hay al menos un prop en cuadro? ¿desapareció la línea clara entre el telón y el escenario?
   ¿los colores de lectura y el director se siguen distinguiendo?
6. Locomoción: `.\Tools\scripts\tdc.ps1 autorun -Task T-0013` (abre el juego con `-TDCAutoRun`, espera 40 s, lo cierra y copia
   `TDCAutoRun_00..11.png` a `docs/evidence/T-0013/autorun/`). Mirá las capturas y describí: ¿el director pasa de quieto a
   correr sin patinar? ¿en el aire tiene pose de salto? ¿algo raro (escala, rotación, pies dentro del piso)?
7. Commit de `Game/Content/Dressing/**` y `Game/Content/Maps/L_Slice.umap` (LFS), la evidencia y el Reporte. `git lfs unlock`
   de lo bloqueado. Push y `[TDC-REPORT]`.
8. `.\Tools\scripts\tdc.ps1 play`.

## Criterios de aceptación
- [ ] `build_dressing.py` termina sin `[TDC] ERROR` y los 3 props quedan en `/Game/Dressing/<SM_x>/`.
- [ ] `01-props-b.png` muestra al menos un prop, sin línea clara en la base del telón.
- [ ] Compila; `docs/evidence/T-0013/autorun/` tiene las capturas de la locomoción.

## Permite
- Editar `Tools/unreal/build_dressing.py` solo para compatibilidad de API, sin cambiar la ubicación de los props.
- Editar `Tools/scripts/tdc.ps1` solo para que `autorun` encuentre las capturas.

## No hacer
- No borrar los assets viejos de `/Game/Dressing/`. No commitear `Tools/unreal/_diag_*`.

## Evidencia pedida
`docs/evidence/T-0013/01-props-b.png` + extracto del log de `build_dressing.py` en el Reporte.

---

## Reporte (lo completa Windows)
- estado: parcial
- sha: (el del commit de este reporte; va en el `[TDC-REPORT]`)
- resumen: compila (9 acciones, sin errores ni warnings de C++). `build_dressing.py` terminó sin `[TDC] ERROR` e importó
  `SM_Spotlight`, `SM_Crates` y `SM_FilmCamera` en `/Game/Dressing/<SM_x>/` (cada uno con su `model` y `texture_*`).
  `01-props-b.png`: el foco con trípode queda en cuadro delante de la primera plataforma, la base del telón ya no tiene la
  línea clara (se funde a oscuro contra el escenario), el cilindro rojo se lee bien y el director se distingue aunque
  queda oscuro. `autorun` generó las 12 capturas, pero **no validan la locomoción**: ver notas.
- errores:
  - Primera corrida de `build_dressing.py`: salió con código 1 por
    `LogSavePackage: Error: Cannot remove '.../Game/Content/Dressing/T_Backdrop_Noir.uasset' as it is read only!`.
    El script reimporta la textura del telón y la tarea solo pedía lockear el mapa y `M_Backdrop`. Lockeé también
    `Game/Content/Dressing/T_Backdrop_Noir.uasset` y la segunda corrida terminó con código 0.
  - Extracto de la segunda corrida:
    ```
    LogInterchangeEngine: Warning: [Art/Export/Dressing/SM_Spotlight.fbx : 'SM_Spotlight', StaticMesh] No smoothing group information was found for this mesh 'model' in the FBX file.
    LogInterchangeEngine: Warning: [Art/Export/Dressing/SM_Crates.fbx : 'SM_Crates', StaticMesh] No smoothing group information was found for this mesh 'model' in the FBX file.
    LogInterchangeEngine: Warning: [Art/Export/Dressing/SM_FilmCamera.fbx : 'SM_FilmCamera', StaticMesh] No smoothing group information was found for this mesh 'model' in the FBX file.
    LogPython: [TDC] ambientación noir aplicada y L_Slice guardado.
    ```
- evidencia: `docs/evidence/T-0013/01-props-a.png`, `01-props-b.png` y `autorun/TDCAutoRun_00..11.png` (1 por segundo,
  confirmado en el log).
- notas / preguntas:
  - **Locomoción (autorun), lo que se ve en las capturas:**
    - 00 quieto en el arranque; 01 avanzó a la derecha; 02 parado en el borde derecho de la primera plataforma;
      03 a 06 otra vez cerca del arranque; 07 de nuevo en el borde; 08 muy por debajo del nivel, sobre un plano lila;
      09 no se lo ve; 10 y 11 dentro del hueco entre plataformas, por debajo del nivel del piso.
    - El demo no cruza el primer hueco: cae y reaparece, así que solo recorre la primera plataforma.
    - En las 12 capturas el director se ve en la misma pose erguida, con brazos abajo y piernas juntas. No vi zancada
      de carrera ni pose de salto en ningún frame, incluidos 01 (ya en movimiento) y 11 (en el aire dentro del hueco).
      Mide ~80 px de alto en 1280x720, así que no puedo descartar diferencias chicas de pose; no pude ampliar los recortes.
    - No puedo afirmar si patina: con 1 frame/s y sin pose de carrera visible, no hay cómo comparar pies contra piso.
    - Escala y rotación se ven bien (mira a la derecha, pies sobre la plataforma, sin hundirse) en 00 a 07.
  - Cuando la cámara baja (02, 07, 08, 10, 11) aparece una franja lila clara en el borde inferior del cuadro; en 08 ocupa
    media pantalla. No es la base del telón y no aparece en el encuadre normal (`01-props-b`), pero se ve al caer al hueco.
  - Los props de cajas se ven en 02, 07, 09, 10 y 11, en el hueco entre plataformas, con color madera legible.
    `SM_FilmCamera` no entró en cuadro en ninguna captura (está más adelante, en X 3300).
  - En todas las capturas hay una esfera oscura flotando delante del telón; no sé si es intencional.
  - `T_Backdrop_Noir.uasset` quedó modificado por la reimportación y va en el commit (entra en `Game/Content/Dressing/**`).
  - Sugerencia: lockear `T_Backdrop_Noir.uasset` en los pasos de la tarea, o que `build_dressing.py` llame a
    `require_writable` para esa textura.
