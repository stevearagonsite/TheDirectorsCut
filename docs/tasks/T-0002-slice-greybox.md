---
id: T-0002
titulo: Construir y probar la sala greybox del vertical slice
estado: nueva
tipo: editor
rama: t/T-0002-slice-greybox
creada: 2026-11-15
---

# T-0002 — Construir y probar la sala greybox

## Objetivo
Generar `/Game/Maps/L_Slice` con `Tools/unreal/build_slice.py`, jugarla de punta a punta y reportar cómo se siente dirigir.
Depende de T-0001 (que compile).

## Contexto
- El docstring de `Tools/unreal/build_slice.py` describe los 4 beats (luces, sombra, tiempo, combo) y las medidas.
- Controles: A/D mover · Espacio saltar · E prende/apaga el reflector más cercano · Q (mantener) rebobina. Gamepad: stick, A, X, gatillo izq.
- Colores: gris fija · amarillo solo con luz · violeta solo a oscuras · celeste en toma (rebobinable) · verde salida.

## Pasos
1. Abrí el editor y corré el script: Output Log > Python → `py "<repo>/Tools/unreal/build_slice.py"` (o `-ExecutePythonScript`). Tiene que terminar con `[TDC] L_Slice construido y guardado.`
2. Play (PIE) y jugá los 4 beats. Probá también fallar: caer a un pozo (debe reaparecer en el último piso), rebobinar el puente a mitad de caída, rebobinar la caja.
3. Si el script falla por API de Python de tu versión de UE, arreglalo en `Tools/unreal/build_slice.py` (permitido) sin cambiar el layout, y explicalo.
4. Capturas 1280x720 (PIE o `tdc.ps1 shot`) en `docs/evidence/T-0002/`:
   `01-inicio.png`, `02-beat1-luz-on.png`, `03-beat2-luz-off.png`, `04-beat3-puente-caido.png`, `05-beat3-rebobinado.png`, `06-beat4-final.png`.
5. Commit del mapa (`Game/Content/Maps/L_Slice.umap`) + evidencia.

## Criterios de aceptación
- [ ] El mapa se genera sin errores y abre como mapa de inicio.
- [ ] Se puede completar la sala hasta el marcador verde usando los poderes.
- [ ] Reportar por beat: ¿funciona? ¿se entiende sin explicación? ¿algo se siente mal (cámara, salto, alcance de luces, velocidad de rebobinado)?
- [ ] Bugs con pasos para reproducir.

## Permite
- Editar `Tools/unreal/build_slice.py` solo para compatibilidad de API.
- Ajustar intensidad de luces/ambiente en el mapa si no se ve nada (anotar valores para pasarlos al código).

## No hacer
- No cambiar medidas del layout ni C++: proponerlo en el reporte.

## Evidencia pedida
Las 6 capturas + notas de feel por beat.

---

## Reporte (lo completa Windows)
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
