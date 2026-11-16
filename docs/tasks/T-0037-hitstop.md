---
id: T-0037
titulo: Hit-stop al conectar el claquetazo (game feel)
estado: ok
tipo: test
rama: t/T-0037-hitstop
creada: 2026-11-15
---

# T-0037 — Hit-stop del claquetazo

## Objetivo
Que el claquetazo "pegue": al conectar, el juego se congela un instante (90 ms si derriba, 60 ms si aturde, 40 ms si rebota) con
time dilation global, y se restaura en tiempo real con `FTSTicker`. Solo C++ en `TDCDirectorComponent.cpp`. Sin package.

## Pasos
1. `git fetch` y `git checkout t/T-0037-hitstop` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop`.
2. `.\Tools\scripts\tdc.ps1 build`. Si no compila, reportá `fallo` con `archivo:línea`.
3. `.\Tools\scripts\tdc.ps1 autorun -Task T-0037 -Wait 45` (el autorun da claquetazos cada 0.6 s y derriba al extra rojo).
4. Verificá que **no quede trabado en cámara lenta**: las 24 capturas tienen que mostrar al director avanzando como en T-0027
   (si después del primer claquetazo queda casi quieto en todas, el hit-stop no se restauró). En el log buscá `Claquetazo a`.
5. Commit de la evidencia y el Reporte. Push y `[TDC-REPORT]`.

## Criterios de aceptación
- [x] Compila.
- [x] El autorun avanza normal después de los claquetazos (sin quedarse en cámara lenta).

## Permite
- Nada.

## No hacer
- No cambiar C++, mapas ni assets. No empaquetar.

## Evidencia pedida
`docs/evidence/T-0037/autorun/` y la comparación con T-0027 en el Reporte.

---

## Reporte (lo completa Windows)
- estado: ok
- sha: 412aa9a (base probada; el commit del reporte va en el `[TDC-REPORT]`)
- resumen: compila (`tdc.ps1 build`, 20 acciones, `Result: Succeeded`, sin errores ni warnings del proyecto). Autorun de 45 s:
  24/24 capturas. El hit-stop se dispara una vez y se restaura: el director sigue avanzando normal hasta el final.
- errores: ninguno.
- evidencia: `docs/evidence/T-0037/autorun/` (TDCAutoRun_00..23, 1280x720).
- notas / preguntas:
  - Log: una sola línea `LogTDC: Claquetazo a TDCExtra_3 -> 0` (0 = `Defeated`, hit-stop de 90 ms), entre las capturas 07 y 08.
    El resto de los claquetazos del autorun no conecta con nadie (no hay hit-stop).
  - Medición de la restauración: el autorun captura cada 0,5 s de tiempo **de juego**. En tiempo real (timestamps del log) todos
    los intervalos entre capturas son de 0,50 s, salvo 07→08, que dura 0,579 s: los ~80 ms extra coinciden con 90 ms a
    dilation 0,05. Después de la 08 los intervalos vuelven a 0,50 s (08→23: 7,50 s para 15 capturas), o sea que la dilation
    volvió a 1. Si hubiera quedado trabada, cada intervalo duraría ~10 s y con `-Wait 45` no habrían salido las 24 capturas.
  - Comparación con T-0027 (revisé 07, 08, 12, 18, 21, 22 y 23 de T-0037 contra 12, 18, 22 y 23 de T-0027): mismo recorrido.
    07 muestra al director llegando al extra rojo; 08 ya sin el extra (derribado) y el director en el aire frente al cartel;
    12, 18 y 21 lo muestran cada vez más adelante sobre las azoteas; 22 es prácticamente el mismo cuadro que
    `docs/evidence/T-0027/autorun/TDCAutoRun_22.png` (misma posición, mismo fondo, mismo fx de claquetazo).
  - No cubierto por este autorun: los hit-stop de 60 ms (aturdido) y 40 ms (rebote), porque solo conecta con el extra rojo.
    Tampoco el caso de dos golpes conectados dentro de la misma ventana de hit-stop.
  - No se tocó C++, mapas ni assets. No se empaquetó.
