---
id: T-0037
titulo: Hit-stop al conectar el claquetazo (game feel)
estado: enviada
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
- [ ] Compila.
- [ ] El autorun avanza normal después de los claquetazos (sin quedarse en cámara lenta).

## Permite
- Nada.

## No hacer
- No cambiar C++, mapas ni assets. No empaquetar.

## Evidencia pedida
`docs/evidence/T-0037/autorun/` y la comparación con T-0027 en el Reporte.

---

## Reporte (lo completa Windows)
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
