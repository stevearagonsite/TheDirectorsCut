---
id: T-0049
titulo: Integración post-jam en una sola corrida (T-0037 a T-0048)
estado: enviada
tipo: mixto
rama: t/T-0049-integracion
creada: 2026-11-15
---

# T-0049 — Integración

## Objetivo
Reemplaza la cola de 10 tareas sueltas por una sola corrida. `main` + T-0037 (hit-stop), T-0044 (extras al arrancar), T-0046 (salto
asistido, que trae T-0040/T-0043/T-0045: menú, pausa, HUD, cámara, feel del claquetazo), T-0047 (fixes del playtest) y T-0048 (VFX pintados,
que trae T-0039/T-0041/T-0042: arte, valores de luz, calle). Genera `Game\Saved\stevearagonsite_TheDirectorsCut_dev.zip`.
Si algo falla, reportá qué paso y el error: se bisecta con las ramas sueltas.

## Pasos
1. `git fetch` y `git checkout t/T-0049-integracion` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop`.
2. `.\Tools\scripts\tdc.ps1 build`. Si no compila, **pará** y reportá `fallo` con `archivo:línea`.
3. Locks: `git lfs lock Game/Content/Maps/L_Slice.umap` y
   `git ls-files Game/Content/Materials Game/Content/Dressing Game/Content/VFX Game/Content/UI Game/Content/Characters | ForEach-Object { git lfs lock $_ }`.
4. Scripts, en este orden (cada uno sin `[TDC] ERROR`; si alguno no anda por commandlet, `-InEditor`):
   `make_materials.py` → `import_vfx.py` → `import_ui.py` → `build_dressing.py` → `apply_toon.py`.
5. `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0049 -Name 01-escena`. Describí: valores de luz (el plano de juego le gana
   al fondo), calle de adoquín abajo, marquesina a nivel calle, reflector colgante, extras a tamaño humano y cámara más alta.
6. `.\Tools\scripts\tdc.ps1 playtest -Task T-0049` → **RESULT PASS**, con las líneas `claquetazo_*`. Pegá todas las `[TDC-PLAYTEST]`.
7. `.\Tools\scripts\tdc.ps1 replay -Task T-0049 -Run best.json` → pegá el resumen.
8. `.\Tools\scripts\tdc.ps1 autorun -Task T-0049 -Wait 45`: HUD nuevo (sin texto de debug), squash en el claquetazo, sin utilería tapando.
9. `.\Tools\scripts\tdc.ps1 package -Zip stevearagonsite_TheDirectorsCut_dev` y
   `.\Tools\scripts\tdc.ps1 smoke -Task T-0049 -Name 02-menu -Wait 30 -Zip stevearagonsite_TheDirectorsCut_dev`: la captura tiene que mostrar
   el **menú inicial**. En el log del exe, las 3 líneas `-> escala del puppet` (la sombra incluida).
10. Commit de `Game/Content/**` (LFS), evidencia, `Tools/playtest/runs/best.json` y Reporte (con tamaño del zip). `git lfs unlock`. Push y `[TDC-REPORT]`.
11. Dejá abierto el build para Steve: `.\Tools\scripts\tdc.ps1 smoke -Task T-0049 -Name 03-abierto -Keep -Zip stevearagonsite_TheDirectorsCut_dev`.

## Criterios de aceptación
- [ ] Compila; los 5 scripts sin `[TDC] ERROR`.
- [ ] Playtest y replay en RESULT PASS (con `claquetazo_*`).
- [ ] El exe del zip arranca en el menú; 3 extras con escala ajustada; quedó abierto.

## Permite
- Editar `Tools/unreal/*.py` y `Tools/scripts/tdc.ps1` solo para compatibilidad de API.

## No hacer
- No cambiar C++. No tocar los zips de la jam. No commitear zips ni `Tools/unreal/_diag_*`.

## Evidencia pedida
`01-escena.png`, líneas del playtest y replay, `autorun/`, `02-menu.png`, tamaño del zip.

---

## Reporte (lo completa Windows)
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
