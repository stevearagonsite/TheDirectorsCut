---
id: T-0011
titulo: Build integrado para la jam (director + claquetazo + cámara/cielo + colores de lectura)
estado: enviada
tipo: mixto
rama: t/T-0011-color-lectura
creada: 2026-11-15
---

# T-0011 — Build integrado: T-0007 + T-0009 + T-0010 + colores de lectura

## Objetivo
Un único ciclo de PC antes del package de la Epic AI MegaJam (21:00 ART). La rama contiene en su historia:
- **T-0007**: el director de Rodin con animaciones (`import_director.py`, `Art/Export/Director/*.fbx`).
- **T-0009**: claquetazo y extras. `build_slice.py` suma X1, X2 y X3; además vienen la claqueta y la animación del golpe.
- **T-0010**: cámara con brazo de 2200 y FOV 60; SkyAtmosphere, SkyLight y niebla; AndroidFileServer apagado.
- **T-0011**: los colores de lectura (luz, oscuridad, toma y extras) salían grises en 5.8. Ahora usan `/Game/Materials/M_TDCColor`,
  que crea `make_materials.py`, y el bulbo del reflector se ve emisivo cuando está prendido.

## Pasos
0. Si sigue abierto el juego de T-0008, cerralo antes de compilar (Steve ya lo vio; Live Coding bloquea el build):
   `Stop-Process -Name UnrealEditor -ErrorAction SilentlyContinue`. Si existe `docs/tasks/T-0008-reporte.tmp`, borralo; no se commitea.
1. `.\Tools\scripts\tdc.ps1 build`. Si hay errores de C++, **pará** y reportá `estado: fallo` con `archivo:línea` y el mensaje
   de los primeros errores reales.
2. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\make_materials.py`: crea `M_TDCColor`.
3. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\import_director.py`: tiene que loguear la altura (~180 cm), las
   animaciones `-> AS_Director_*` (Idle, Run, Jump y Clap) y `SM_Clapperboard importado`.
4. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\build_slice.py`. Si el commandlet no alcanza, agregá `-InEditor`.
5. `.\Tools\scripts\tdc.ps1 test`: anotá el resultado, que incluye `TDC.Combat.ResolveClap`.
6. Capturas: `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0011 -Name 01-sala`. Tienen que verse el director
   (no la cápsula), los colores (amarillo, violeta, celeste y verde), el cielo y más de un piso en cuadro.
7. Commit de `Game/Content/**` (mapas, materiales y `Characters/Director`, todo por LFS), la evidencia y el Reporte de esta tarea.
   En `T-0007-personaje-skeletal.md` y `T-0009-claquetazo.md` poné `estado:` y una línea de reporte que remita a este archivo.
   Push y `[TDC-REPORT T-0011]`.
8. Dejá el juego abierto: `.\Tools\scripts\tdc.ps1 play`.

## Criterios de aceptación
- [ ] Compila sin errores y los tests pasan (o el fallo queda anotado).
- [ ] `M_TDCColor`, `SK_Director`, `AS_Director_Idle`, `AS_Director_Run`, `AS_Director_Jump` y `SM_Clapperboard` existen.
- [ ] La captura muestra al director, los colores de lectura y el cielo.
- [ ] El juego quedó abierto en la PC.

## Permite
- Cerrar el proceso `UnrealEditor` del juego de T-0008.
- Editar `Tools/unreal/*.py` y `Tools/scripts/tdc.ps1`, solo para compatibilidad.

## No hacer
- No corregir C++: reportalo.
- No commitear `Saved/`, `Intermediate/`, `Binaries/`, `.tmp` ni cambios de `DefaultEngine.ini` que agregue el motor (por ejemplo,
  `SecurityToken`).
- En el reporte no uses rutas `C:\Users\...`; usá `%USERPROFILE%`.

## Evidencia pedida
`docs/evidence/T-0011/01-sala.png`, más los extractos del import (altura y animaciones) y de los tests.

---

## Reporte (lo completa Windows)
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
