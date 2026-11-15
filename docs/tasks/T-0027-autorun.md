---
id: T-0027
titulo: Validar la locomoción del director (autorun con la cámara centrada) sobre mejora 4
estado: ok
tipo: test
rama: t/T-0029-autorun-camara
creada: 2026-11-15
---

# T-0027 — Locomoción del director en capturas

## Objetivo
Objetivo abierto de Steve: "ver el personaje con los movimientos básicos corriendo en Unreal PC Windows". Confirmar con capturas que el
director **anima** (quieto → corriendo → en el aire) con el contenido de mejora 4. No se empaqueta nada.

## Contexto
- La rama trae el resultado de mejora 4 (T-0026) + una línea de C++: en `-TDCAutoRun` la cámara se centra en el director
  (`TargetOffset = 0`, brazo de 900). El juego normal no cambia.

## Pasos
1. `git fetch` y `git checkout t/T-0029-autorun-camara` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop` (cerrá el build de mejora 4 si está abierto).
2. `.\Tools\scripts\tdc.ps1 build`.
3. `.\Tools\scripts\tdc.ps1 autorun -Task T-0027 -Wait 45` (24 capturas en `docs/evidence/T-0027/autorun/`).
4. Elegí 4 capturas y describilas en el Reporte: **quieto** (antes de 1 s), **corriendo** (zancada: piernas separadas), **en el aire**
   (pose de salto) y **claquetazo** (si alguna lo agarra). Para cada una: número de captura, qué pose tiene el director, si está a
   escala, si mira hacia donde avanza, si patina o atraviesa el piso.
5. Commit solo de la evidencia (las 4 elegidas + las demás) y el Reporte. Push y `[TDC-REPORT]`.
6. Volvé a dejar abierto el build de mejora 4: `.\Tools\scripts\tdc.ps1 smoke -Task T-0027 -Name 99-video -Keep -Zip stevearagonsite_TheDirectorsCut_mejora4`
   (si `_mejora4.zip` no existe, `_mejora2`). No commitees esa captura.

## Criterios de aceptación
- [ ] Compila.
- [ ] Hay capturas con el director en cuadro: quieto, corriendo y en el aire, con poses distintas.

## Permite
- Nada.

## No hacer
- No cambiar C++, mapas ni assets. No empaquetar.

## Evidencia pedida
`docs/evidence/T-0027/autorun/` + la descripción de 4 capturas en el Reporte.

---

## Reporte (lo completa Windows)
- estado: ok
- sha: 65b8865 (probado; el commit del reporte va en el `[TDC-REPORT]`)
- resumen: compila (`tdc.ps1 build`: Result: Succeeded, 4 acciones, solo `TDCCharacter.cpp`, 58 s). `tdc.ps1 autorun -Task T-0027 -Wait 45`
  generó las 24 capturas (1280x720, una cada 0,5 s). La cámara ahora deja al director centrado: aparece en 18 de 24 capturas y se
  leen tres poses distintas (quieto, corriendo, en el aire). El recorrido no pasa del primer hueco: cae, reaparece al inicio y repite
  el ciclo unas 5 veces (ver notas).
- errores: ninguno de compilación. El log del juego no tiene `Error:`, `Ensure` ni `Fatal`.
- evidencia: `docs/evidence/T-0027/autorun/TDCAutoRun_00..23.png`. Las 4 elegidas:
  - **Quieto — `TDCAutoRun_00.png`** (0,5 s): de pie, erguido, brazos a los costados, pies juntos. A escala (llega más o menos al
    pecho del extra rojo que entra por la derecha; proporción cabezona coherente con el set). Mira a la derecha, que es hacia donde
    va a avanzar. Pies apoyados sobre la plataforma, con sombra de contacto; no atraviesa el piso.
  - **Corriendo — `TDCAutoRun_07.png`** (4,0 s, segundo ciclo): torso inclinado hacia adelante, piernas separadas en zancada (una
    atrás extendida, otra adelante), brazo delantero flexionado. A escala respecto del extra rojo que tiene al lado. Mira a la derecha
    = dirección de avance. Pie de apoyo sobre la plataforma, no la atraviesa. Lo mismo, con otra fase de la zancada, en `02`, `06`,
    `12`, `16` y `21` (`06` y `21` salen con motion blur). Si patina no se puede afirmar con fotos fijas: lo que sí se ve es que la
    fase de la zancada cambia de una captura a otra.
  - **En el aire — `TDCAutoRun_03.png`** (2,0 s): pose de salto, distinta de las otras dos: torso erguido, un brazo adelante, piernas
    flexionadas hacia atrás; el piso queda en el borde inferior del cuadro y el extra rojo abajo. A escala, mira a la derecha. Misma
    pose en `08` y `13`; en `19` va en el aire con las piernas más recogidas.
  - **Claquetazo — `TDCAutoRun_01.png`** (1,0 s): se ve el destello blanco del claquetazo a la altura del pecho, delante del director.
    El director **no tiene una pose propia de claquetazo**: está en la misma pose de quieto que en `00`. A escala, mira a la derecha,
    pies sobre la plataforma. El destello también sale en `04` (sobre la cabeza), `10`, `16` y `22`.
- notas / preguntas:
  - **El autorun cae siempre en el mismo lugar y vuelve al inicio.** Secuencia que se repite: corre (`02`, `06-07`, `12`, `16`, `21`) →
    salta (`03`, `08`, `13`, `19`) → queda más allá del borde derecho de la primera plataforma, erguido en pose de quieto y sin piso
    visible debajo (`04`, `09`, `10`, `14`, `17`, `22`) → cae fuera de cuadro (`05`, `11`, `15`, `18`, `20`; en `23` solo se le ve la
    gorra en el borde inferior) → reaparece al inicio. En `14`, `17` y `22` se ve un trapecio amarillo muy tenue justo donde está
    parado: parece una pieza de luz (T-0026) que lo sostiene un momento y desaparece. Es una interpretación mía a partir de las fotos,
    no lo verifiqué en el editor; el claquetazo automático cada 0,6 s podría ser lo que la apaga.
  - En esas capturas "paradas" después del salto el director está en pose de quieto aunque el autorun sigue empujando hacia la
    derecha. Con fotos fijas no puedo distinguir si está apoyado en esa pieza o si cae con la pose de quieto; conviene mirarlo en
    movimiento.
  - 6 de 24 capturas no sirven para ver al director (cae más rápido de lo que lo sigue la cámara).
  - Paso 6: el build de mejora 4 se reabre después del push; el resultado va en el `[TDC-REPORT]` (esa captura no se commitea).
