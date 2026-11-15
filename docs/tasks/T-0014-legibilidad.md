---
id: T-0014
titulo: Legibilidad (HUD de controles, aviso del puente, menos oscuro) + locomoción que anima + autorun que la valida
estado: enviada
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
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
