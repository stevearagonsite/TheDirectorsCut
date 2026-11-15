---
id: T-0007
titulo: Personaje del director (Rodin) corriendo en el juego
estado: parcial
tipo: mixto
rama: t/T-0007-personaje-skeletal
creada: 2026-11-15
---

# T-0007 — Personaje del director corriendo en el juego

## Objetivo
Reemplazar la cápsula greybox por el director (modelo de Rodin con auto-rig y animaciones in-place) y verlo correr, saltar y
quedarse quieto en el juego. Epic AI MegaJam: cierre 2026-11-15 22:59 ART. Depende de T-0008 (C++ compila y `L_Slice` existe).

## Contexto
- FBX: `Art/Export/Director/Director_rigged_anims.fbx` (LFS). Trae mesh, skeleton con nombres Mixamo y las animaciones Idle,
  Walking, Jogging, Two Cycle Sprint y Jumping, ya in-place (la traslación horizontal del Hips quedó fija en Blender).
- `Tools/unreal/import_director.py` lo importa a `/Game/Characters/Director/` con nombres fijos: `SK_Director`,
  `AS_Director_Idle`, `AS_Director_Run`, `AS_Director_Jump` (más Walk y Sprint).
- El C++ de esta rama (`ATDCCharacter`) carga esos assets por ruta y, si existen, oculta el greybox y reproduce Idle/Run/Jump
  según la velocidad y si está en el aire. No hay AnimBP ni Blueprint que armar. Si no existen, queda el greybox.
- Rotación y altura del mesh: UPROPERTY en `ATDCCharacter` (por defecto Yaw −90 y Z −90).

## Pasos
1. `git merge origin/main` si la rama quedó atrás. Tiene que incluir T-0008: `.uproject` en 5.8 y `L_Slice`.
2. `.\Tools\scripts\tdc.ps1 build`. Si hay errores de C++: **pará** y reportá `estado: fallo` con `archivo:línea` y el mensaje.
3. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\import_director.py`. Tiene que loguear
   `[TDC] Director importado en /Game/Characters/Director`, la altura (~180 cm) y una línea `anim ... ->` por cada animación.
   Si dice `FALTAN animaciones`, anotalo. Si falla por API de Python de UE 5.8, arreglá el script (permitido) y explicalo.
4. Captura: `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0007 -Name 01-director`. Tiene que verse el director
   (no la cápsula) parado, mirando hacia el costado del recorrido y con los pies en el piso. Si está acostado, de espaldas o
   flotando, anotá qué ves (eso se corrige con las UPROPERTY de rotación y altura).
5. Commit de `Game/Content/Characters/Director/**` (LFS), la evidencia y el Reporte. Push y `[TDC-REPORT]`.
6. **Dejá el juego abierto en la PC** para que Steve lo pruebe:
   `Start-Process "$UE\Engine\Binaries\Win64\UnrealEditor.exe" -ArgumentList '"<repo>\Game\TheDirectorsCut.uproject" /Game/Maps/L_Slice -game -windowed -ResX=1600 -ResY=900'`.

## Criterios de aceptación
- [ ] Compila sin errores.
- [ ] `SK_Director` mide unos 180 cm y existen `AS_Director_Idle`, `AS_Director_Run` y `AS_Director_Jump`.
- [ ] La captura muestra al director en lugar de la cápsula, con los pies en el piso.
- [ ] El juego quedó abierto en la PC.

## Permite
- Editar `Tools/unreal/import_director.py` y `Tools/scripts/tdc.ps1`, solo para compatibilidad.

## No hacer
- No corregir C++ en `Game/Source/`: reportalo.
- No editar `L_Slice`.
- No commitear `Saved/`, `Intermediate/` ni `Binaries/`.
- En el reporte no uses rutas `C:\Users\...`; usá `%USERPROFILE%`.

## Evidencia pedida
`docs/evidence/T-0007/01-director.png` y el log del import: altura y mapeo de animaciones.

---

## Reporte (lo completa Windows)
- estado: parcial
- sha: ver T-0011
- resumen: se ejecutó dentro de T-0011 (ver `docs/tasks/T-0011-color-lectura.md`, Reporte): import OK (180 cm, AS_Director_*,
  SM_Clapperboard), pero en la captura el director se ve diminuto.
- errores:
- evidencia: docs/evidence/T-0011/
- notas / preguntas:
