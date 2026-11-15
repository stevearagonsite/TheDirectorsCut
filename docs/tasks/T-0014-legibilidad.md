---
id: T-0014
titulo: Legibilidad (HUD de controles, aviso del puente, menos oscuro) + locomoción que anima + autorun que la valida
estado: parcial
tipo: mixto
rama: t/T-0014-legibilidad
creada: 2026-11-15
---

# T-0014 — Legibilidad + locomoción

## Objetivo
Último ciclo de contenido antes del package final (freeze 20:00). Lo pidió Steve después de jugar: entender los controles y el
puente, menos oscuridad, y que el director **anime** al correr y saltar. Contiene T-0013 (props y telón).

## Contexto
- 59a228a: HUD de controles permanente arriba a la izquierda, aviso al caer el puente ("mantené Q"), exposure −0.3.
- 9055ead: el blendspace de locomoción queda **apagado** por defecto (el que armó Python congelaba la pose); vuelven los clips.
- 9d185d7: el autorun da un claquetazo cada 0.6 s (saca al extra rojo del camino), acerca la cámara (brazo de 900) y saca
  24 capturas cada 0.5 s. `build_dressing.py` oscurece Pit_Floor (era la franja lila) y chequea los locks de M_Backdrop y T_Backdrop_Noir.

## Pasos
1. `git fetch` y `git checkout t/T-0014-legibilidad` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop`.
2. `.\Tools\scripts\tdc.ps1 build`. Si no compila, **pará** y reportá `fallo` con `archivo:línea` (no reviertas).
3. Locks: `git lfs lock` de `Game/Content/Maps/L_Slice.umap`, `Game/Content/Dressing/M_Backdrop.uasset` y
   `Game/Content/Dressing/T_Backdrop_Noir.uasset`.
4. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\build_dressing.py` (sin `[TDC] ERROR`).
5. `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0014 -Name 01-hud-a` y después `-Name 01-hud-b` (vale la b).
   Describí: ¿se ve el HUD de controles arriba a la izquierda y se lee? ¿la escena está menos oscura y los colores se distinguen?
   ¿quedó la franja lila abajo?
6. `.\Tools\scripts\tdc.ps1 autorun -Task T-0014 -Wait 45` (24 capturas en `docs/evidence/T-0014/autorun/`). Describí frame por
   frame lo importante: ¿el director pasa de quieto a **zancada de carrera**? ¿en el aire tiene **pose de salto**? ¿el claquetazo
   saca al extra rojo y el director pasa el primer tramo? ¿algo raro (patina, rotación, pies dentro del piso)?
7. Commit de `Game/Content/**` (LFS), la evidencia y el Reporte. `git lfs unlock` de lo bloqueado. Push y `[TDC-REPORT]`.
8. `.\Tools\scripts\tdc.ps1 play`.

## Criterios de aceptación
- [ ] Compila; `build_dressing.py` sin `[TDC] ERROR`.
- [ ] `01-hud-b.png`: HUD legible, escena menos oscura, sin franja lila.
- [ ] Autorun: se ve la zancada de carrera y una pose de salto distinta de la de reposo.

## Permite
- Editar `Tools/unreal/build_dressing.py` solo para compatibilidad de API.

## No hacer
- No corregir C++: reportarlo. No commitear `Tools/unreal/_diag_*`.

## Evidencia pedida
`01-hud-b.png`, `autorun/TDCAutoRun_*.png` y la descripción por frame en el Reporte.

---

## Reporte (lo completa Windows)
- estado: parcial
- sha: (el del commit de este reporte; va en el [TDC-REPORT])
- resumen: compila sin errores ni warnings (9 acciones, 25 s). `build_dressing.py` corrió sin `[TDC] ERROR` y guardó L_Slice
  ("ambientación noir aplicada y L_Slice guardado"), pero el commandlet salió con código 1 por 3 `LogSavePackage: Error`
  (ver errores). La escena quedó claramente menos oscura y los colores se distinguen. **No se pudo validar el HUD ni la
  locomoción**: las capturas no incluyen el HUD y en las 24 del autorun el director nunca entra en cuadro.
- errores:
  - `tdc.ps1 python -Script Tools\unreal\build_dressing.py` → `python terminó con código 1`. Causa: el script reimporta los
    props y sus `.uasset` son lockables sin lock (la tarea solo pedía lockear el mapa, M_Backdrop y T_Backdrop_Noir):
    ```
    LogSavePackage: Error: Cannot remove 'Game/Content/Dressing/SM_Spotlight/SM_Spotlight.uasset' as it is read only!
    LogSavePackage: Error: Cannot remove 'Game/Content/Dressing/SM_Crates/SM_Crates.uasset' as it is read only!
    LogSavePackage: Error: Cannot remove 'Game/Content/Dressing/SM_FilmCamera/SM_FilmCamera.uasset' as it is read only!
    ```
    Los tres props quedaron como estaban en T-0013 (mismos FBX) y el mapa los referencia bien: se ven en las capturas. No
    los lockeé ni volví a correr para no generar binarios nuevos sin cambio de contenido.
  - Sin errores de compilación.
- evidencia: `docs/evidence/T-0014/01-hud-a.png`, `01-hud-b.png`, `autorun/TDCAutoRun_00..23.png`
- notas / preguntas:
  - **01-hud-b.png (y la a, idénticas)**:
    - HUD de controles: **no aparece en la captura**. No puedo afirmar que no exista en el juego: `ShowControls()` usa
      `GEngine->AddOnScreenDebugMessage` y `tdc.ps1 shot` captura con `HighResShot`, que no incluye los mensajes en
      pantalla. Lo mismo el autorun (`RequestScreenshot(..., bShowUI=false, ...)`). Intenté una captura con UI
      (`-ExecCmds="Shot showui"`) pero el modo de permisos del worker no me dejó lanzar el editor fuera de `tdc.ps1`.
      Queda **sin verificar**; hace falta que `shot` capture con UI (p. ej. un switch `-ShowUI` que use `Shot showui`) o
      que Steve lo mire en la ventana de `play`.
    - Menos oscuro: **sí**. El telón (skyline azul con bruma) se lee completo, el director (gris oscuro con gorra), el
      extra rojo/salmón, la plataforma gris clara y los props (reflector con trípode, segundo trípode) se distinguen bien.
    - Franja lila: **en esta toma abierta no**. Abajo hay una franja negra (escenario de primer plano) y, debajo, una
      franja gris pardo. Pero ver el autorun: ahí sí aparece una superficie lila.
  - **Autorun (24 capturas, 2 por segundo, cámara con brazo de 900)** — el director **no se ve en ninguna**:
    - 00–01: extra rojo (cilindro) parado sobre la primera plataforma, centrado en cuadro; telón detrás; abajo a la
      derecha asoma una superficie **celeste-lila clara**. Sin director.
    - 02: el extra rojo se corrió a la izquierda del cuadro (la cámara avanzó); aparece la bola negra arriba a la derecha.
    - 03–04: solo telón y la bola negra; la cámara subió (¿salto?). En 04 franja lila abajo.
    - 05: primer plano muy desenfocado de un prop (cajas, naranja) que tapa medio cuadro; mitad inferior lila.
    - 06: borde derecho de la primera plataforma, sin extra, bola negra arriba a la derecha.
    - 07: otra vez el extra rojo, ahora a la izquierda del cuadro, sobre la plataforma (vista parecida a 00–01).
    - 08–09: repiten 03–04 (telón + bola negra; franja lila en 09). El log marca `Claquetazo a TDCExtra_3 -> 0` entre 07
      y 08: desde acá el extra rojo no vuelve a aparecer.
    - 10, 12, 15, 17, 20, 22: telón, bola negra arriba a la izquierda, esquina de la segunda plataforma (beige) abajo a
      la derecha, franja lila ancha abajo.
    - 11, 16, 21: desenfocado; la plataforma beige arriba a la derecha con una **sombra negra con forma del director**
      (cabeza y torso) sobre su cara frontal; mitad inferior del cuadro lila.
    - 13, 18, 23: prop oscuro desenfocado tapando el tercio izquierdo; plataforma beige a la derecha con la misma sombra
      del director; franja lila. En 23 la bola de arriba está encendida (blanca): el log marca `Luces: TDCStageLight_0 -> ON`.
    - 14, 19: telón, franja lila abajo, prop naranja desenfocado abajo a la derecha.
  - **Respuestas del paso 6**:
    - ¿Pasa de quieto a zancada de carrera? **No se puede saber**: el director está fuera de cuadro en las 24.
    - ¿Pose de salto en el aire? **No se puede saber**, mismo motivo. Lo único suyo que se ve es la sombra proyectada
      sobre la plataforma beige (11, 13, 16, 18, 21, 23), y no alcanza para leer la pose.
    - ¿El claquetazo saca al extra rojo? **Sí**, a los ~5 s (un solo `Claquetazo a TDCExtra_3` en el log); antes de eso
      la vista vuelve al inicio una vez (07 ≈ 00), compatible con que el extra lo tocó y lo reinició. ¿Pasa el primer
      tramo? Llega hasta el hueco entre la primera plataforma y la beige, pero desde 10 la vista **se repite cada 5
      capturas (2.5 s, el período del salto) sin avanzar**: o cae al pozo y reaparece, o salta en el lugar. No llegó más lejos.
    - Raro: (1) el encuadre: con brazo de 900 la cámara deja al director afuera (parece estar a la izquierda/abajo del
      cuadro; la cámara mira adelantada) y pasa por detrás de los props de primer plano, que tapan y desenfocan 05, 11,
      13, 14, 16, 18, 19, 21, 23; (2) la **superficie lila** bajo las plataformas se ve en 16 de 24 capturas — hipótesis
      mía, sin verificar: es Pit_Floor y el `color_override` que le pone `build_dressing.py` no llega al juego.
  - **Criterios**: 1) compila ✔, sin `[TDC] ERROR` ✔ (pero exit 1 por los props read-only). 2) menos oscura ✔, HUD sin
    verificar, lila ✘ en el autorun. 3) ✘ no se ve al director.
  - **Sugerencias para la Mac**: en el autorun no acercar tanto (o sacar el desfase de la cámara) y capturar con
    `bShowUI=true`; agregar los `SM_*/SM_*.uasset` a los locks de la tarea o que `import_prop` no reimporte si el asset ya
    existe; revisar el color de Pit_Floor en runtime.
  - Locks soltados al terminar. Quedó el juego abierto con `tdc.ps1 play` (paso 8).
