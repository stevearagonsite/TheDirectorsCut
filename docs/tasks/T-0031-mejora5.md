---
id: T-0031
titulo: Package de mejora 5: toma rebobinada visible + extras con movimiento (sobre mejora 4)
estado: enviada
tipo: mixto
rama: t/T-0031-toma-extras
creada: 2026-11-15
---

# T-0031 — Toma visible + extras vivos + package de mejora 5

## Objetivo
Dos cosas rotas que reportó Steve jugando mejora 4:
1. "Los personajes enemigos no tienen animaciones": el Puppet anima siempre (sin optimización por visibilidad) y suma rebote y
   balanceo procedural al caminar, temblor al estar aturdido y respiración quieto. Además `check_anims.py` diagnostica los clips.
2. "La plataforma que vuelve en el tiempo no muestra que quedó afectada": al rebobinar, las piezas en toma que se movieron brillan
   cian con pulso y llevan el flipbook de rebobinado; al soltar queda un resplandor que parpadea y se apaga en ~2.5 s.
Genera `Game\Saved\stevearagonsite_TheDirectorsCut_mejora5.zip`. **No toques** los otros zips.

## Contexto
- La rama sale de `main` con mejora 4 (#31): no hay assets que rehacer, solo C++.
- Si el build falla, reportá `fallo` con `archivo:línea` y no empaquetes.

## Pasos
1. `git fetch` y `git checkout t/T-0031-toma-extras` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop` (cerrá también el build abierto).
2. `.\Tools\scripts\tdc.ps1 build`.
3. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\check_anims.py` y **pegá la salida completa en el Reporte** (por clip:
   si tiene movimiento real y si el esqueleto coincide con el SK del extra).
4. `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0031 -Name 01-mejora5`.
5. `.\Tools\scripts\tdc.ps1 package -Zip stevearagonsite_TheDirectorsCut_mejora5`
6. `.\Tools\scripts\tdc.ps1 smoke -Task T-0031 -Name 02-package -Wait 60 -Zip stevearagonsite_TheDirectorsCut_mejora5`
7. Commit de la evidencia y el Reporte (con el tamaño del zip). Push y `[TDC-REPORT]`.
8. Dejá abierto el build: `.\Tools\scripts\tdc.ps1 smoke -Task T-0031 -Name 03-video -Keep -Zip stevearagonsite_TheDirectorsCut_mejora5`.

## Criterios de aceptación
- [ ] Compila; salida de `check_anims.py` en el Reporte.
- [ ] `_mejora5.zip` de menos de 1 GB; el exe corre a los 60 s, sin `Fatal` ni assets faltantes.

## Permite
- Editar `Tools/unreal/check_anims.py` solo para compatibilidad de API.

## No hacer
- No cambiar C++, mapas ni assets. No pisar los otros zips. No commitear zips ni `Tools/unreal/_diag_*`.

## Evidencia pedida
Salida de `check_anims.py`, `01-mejora5.png`, `02-package.png`, tamaño del zip y extracto del log de `smoke`.

---

## Reporte (lo completa Windows)
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
