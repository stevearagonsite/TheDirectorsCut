---
id: T-0036
titulo: Integración del arte post-jam (reflector colgante, utilería, VFX de piezas, techo de plataformas)
estado: enviada
tipo: mixto
rama: t/T-0036-arte-integracion
creada: 2026-11-15
---

# T-0036 — Integración del arte post-jam

## Objetivo
Que se vea en el juego el arte que ya está en main:
1. **Reflector colgante** (`Art/Export/Dressing/StageLightHanging/`): `build_dressing.py` lo importa como
   `/Game/Dressing/Props/SM_StageLight_Hanging` con material mate oscurecido (×0.35). `ATDCStageLight` lo carga si existe
   (componente `Housing`, sin sombra) y achica el bulbo emisivo a 0.18 para que quede en la lente. Si no existe, queda la esfera.
2. **Utilería** (`Art/Export/Dressing/SetProps/`): silla de director (×2), boom (madera oscurecida ×0.45) y latas (.glb, ×2)
   en la franja de primer plano, sin colisión, lejos de los beats.
3. **VFX de piezas**: `TDC::Fx::{Light,Dark}Piece{Appear,Vanish}`; `ATDCSetPiece` los dispara al prender/apagar luces
   (no en el BeginPlay). `import_vfx.py` ya importa todos los PNG de `Art/Export/VFX`.
4. **Techo negro** de las plataformas fijas: `make_materials.py` baja el patrón en techos (`TopPatternScale` 0.35) y los aclara
   (`TopBoost` 1.8 × PatternStrength).

## Contexto
- Si el C++ no compila, reportá `fallo` con `archivo:línea` y no empaquetes.
- Ajustables en el editor sin recompilar (en cada `ATDCStageLight` de L_Slice): `HousingOffset` (cm, default Z −20),
  `HousingRotation` (default yaw 180: el FBX de Rodin mira a −Y) y `HangingBulbScale`. Si la lente queda de espaldas o el caño
  no "cuelga", corregilo ahí y anotá los valores en el reporte.
- No toca el menú ni el HUD (T-0035).

## Pasos
1. `git fetch` y `git checkout t/T-0036-arte-integracion` (verificá el sha). Cerrá cualquier build abierto (`tdc.ps1 stop`).
2. `.\Tools\scripts\tdc.ps1 build`.
3. Locks: `git lfs lock Game/Content/Maps/L_Slice.umap` y
   `git ls-files Game/Content/Materials Game/Content/Dressing Game/Content/VFX | ForEach-Object { git lfs lock $_ }`.
4. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\import_vfx.py` (tiene que listar los 4 `T_*_piece_*` nuevos).
5. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\make_materials.py`.
6. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\build_dressing.py` (tiene que decir "reflector colgante importado" y
   "utilería de set colocada: 5"; sin `[TDC] ERROR`).
7. `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0036 -Name 01-arte-a` y `-Name 01-arte-b`.
   Describí: ¿los reflectores son el foco colgante (no la esfera) con la lente brillando cuando están prendidos? ¿la utilería se
   ve en primer plano sin tapar al director ni a los extras? ¿el techo de las plataformas fijas ya no es negro?
8. Si entra, `tdc.ps1 autorun -Task T-0036 -Wait 30` para ver un VFX de pieza (el autorun no aprieta E: puede no salir).
9. `.\Tools\scripts\tdc.ps1 package -Zip stevearagonsite_TheDirectorsCut_dev` y
   `.\Tools\scripts\tdc.ps1 smoke -Task T-0036 -Name 02-package -Wait 60 -Zip stevearagonsite_TheDirectorsCut_dev`.

## Criterios
- [ ] Compila sin errores ni warnings nuevos.
- [ ] `01-arte-b.png`: reflector colgante, utilería visible y techo de plataformas legible.
- [ ] `_dev.zip` corre a los 60 s sin `Fatal`.

## Evidencia
`docs/evidence/T-0036/`: capturas, extracto del log de build_dressing y del smoke.

---

## Reporte (lo completa Windows)
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
