---
id: T-0009
titulo: Claquetazo y extras en la sala
estado: enviada
tipo: mixto
rama: t/T-0009-claquetazo
creada: 2026-11-15
---

# T-0009 — Claquetazo y extras

## Objetivo
Sumar el ataque básico (**claquetazo**, D-002) y los enemigos greybox (**extras**, D-003) al juego y verlos funcionar en `L_Slice`.
Epic AI MegaJam: cierre 2026-11-15 22:59 ART. Si no compila antes de las 21:00 ART, el package sale sin esta tarea.
Depende de T-0007: esta rama está encima de ella.

## Contexto
- Claquetazo: tecla F, clic izquierdo o gamepad B. `UTDCDirectorComponent::Clap()` hace un overlap de esfera hacia donde mira
  el director: alcance 90, radio 75, cooldown 0.45 s.
- `ATDCExtra` patrulla en X. Si toca al director, lo manda al último piso seguro. Hay tres tipos:
  - Común (rojo): cae de un claquetazo.
  - Resistente (naranja): el claquetazo lo aturde 1.5 s y lo empuja.
  - De guion (negro con franja): inmune al claquetazo. Si depende de la luz, prender o apagar el reflector lo saca para siempre.
- `build_slice.py` suma 3 extras: X1 común (beat 1), X2 sombra de guion (beat 2, junto a L2) y X3 resistente (beat 4).
- Opcionales que el C++ carga por ruta si existen: `SM_Clapperboard` (prop, enganchado al hueso `RightHand`) y `AS_Director_Clap`
  (animación). `import_director.py` importa la claqueta si está `Art/Export/Director/SM_Clapperboard.fbx`.

## Pasos
0. **Esta tarea incluye T-0007 y T-0010** (cámara 2200/FOV 60, cielo, AndroidFileServer off; la rama está encima): hacé también sus pasos de import (el paso 4 de acá los cubre) y
   completá el Reporte de `docs/tasks/T-0007-personaje-skeletal.md` con lo que corresponda al director.
   Si el juego de T-0008 sigue abierto (Live Coding bloquea el build), cerralo primero: `Stop-Process -Name UnrealEditor`.
   Borrá `docs/tasks/T-0008-reporte.tmp` si existe (no se commitea).
1. La rama ya está rebaseada sobre `main` con T-0008 (`.uproject` 5.8 y `L_Slice`).
2. `.\Tools\scripts\tdc.ps1 build`. Si hay errores de C++: **pará** y reportá `estado: fallo` con `archivo:línea` y el mensaje.
3. `.\Tools\scripts\tdc.ps1 test`. Incluye `TDC.Combat.ResolveClap`; anotá el resultado.
4. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\import_director.py`, para sumar la claqueta si está el FBX.
5. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\build_slice.py`, para regenerar `L_Slice` con los extras
   (si el commandlet no alcanza, con `-InEditor`).
6. Captura: `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0009 -Name 01-extras`.
7. Commit de `L_Slice.umap`, `Game/Content/Characters/Director/**` (si cambió), la evidencia y el Reporte. Push y `[TDC-REPORT]`.
8. Dejá el juego abierto en la PC: `.\Tools\scripts\tdc.ps1 play`.

## Criterios de aceptación
- [ ] Compila sin errores y `TDC.Combat.ResolveClap` pasa.
- [ ] `L_Slice` regenerado con X1, X2 y X3.
- [ ] La captura muestra la sala con los extras.

## Permite
- Editar `Tools/unreal/build_slice.py` e `import_director.py`, solo para compatibilidad de API.

## No hacer
- No corregir C++: reportalo.
- No cambiar el layout de la sala.
- En el reporte no uses rutas `C:\Users\...`; usá `%USERPROFILE%`.

## Evidencia pedida
`docs/evidence/T-0009/01-extras.png`, el resultado de los tests y del import.

---

## Reporte (lo completa Windows)
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
