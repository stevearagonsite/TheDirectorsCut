---
id: T-0044
titulo: La sombra de guion no queda derrotada al cargar; extras a escala desde el primer frame
estado: enviada
tipo: mixto
rama: t/T-0044-extras-arranque
creada: 2026-11-15
---

# T-0044 — Extras al arrancar

## Objetivo
En el build empaquetado de mejora 6, uno de los 3 extras (`TDCExtra_4`, la sombra de guion X2) no logueaba `-> escala del puppet`.
Causa: según el orden de BeginPlay en el build cocinado, la sombra se evaluaba antes de que L2 se registrara y quedaba **derrotada al
cargar**. Ahora la luz solo puede sacar al extra de la toma desde el tick siguiente al BeginPlay, y la escala se aplica en el mismo frame
de carga (las capturas de `shot`/`smoke` ya ven los extras a tamaño humano). Genera `..._mejora7.zip`.

## Pasos
1. `git fetch` y `git checkout t/T-0044-extras-arranque` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop`.
2. `.\Tools\scripts\tdc.ps1 build`. Si no compila, reportá `fallo` con `archivo:línea`.
3. `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0044 -Name 01-primer-frame`: X1 (rojo, al lado del reflector) tiene que verse
   **a tamaño humano ya en esta captura**.
4. `.\Tools\scripts\tdc.ps1 autorun -Task T-0044 -Wait 45` (elegí 2 capturas con X1 caminando).
5. `.\Tools\scripts\tdc.ps1 package -Zip stevearagonsite_TheDirectorsCut_mejora7` y
   `.\Tools\scripts\tdc.ps1 smoke -Task T-0044 -Name 02-package -Wait 60 -Zip stevearagonsite_TheDirectorsCut_mejora7`.
   En el log del **juego empaquetado** (`Game\Saved\PackageTest\...\TheDirectorsCut.log`) buscá `-> escala del puppet`: tienen que
   aparecer los **3** extras (incluido `TDCExtra_4` / la sombra). Pegá las líneas.
6. Commit de evidencia y Reporte (con tamaño del zip). Push y `[TDC-REPORT]`.
7. Dejá abierto el build: `.\Tools\scripts\tdc.ps1 smoke -Task T-0044 -Name 03-abierto -Keep -Zip stevearagonsite_TheDirectorsCut_mejora7`.

## Criterios de aceptación
- [ ] Compila.
- [ ] `01-primer-frame.png`: X1 a tamaño humano.
- [ ] En el log del empaquetado, 3 líneas `-> escala del puppet` (la sombra incluida).

## Permite
- Nada.

## No hacer
- No cambiar C++, mapas ni assets. No tocar los otros zips.

---

## Reporte (lo completa Windows)
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
