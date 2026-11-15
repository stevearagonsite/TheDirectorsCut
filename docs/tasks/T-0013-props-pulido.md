---
id: T-0013
titulo: Props de Rodin y telón sin línea clara (último pulido antes del package final)
estado: enviada
tipo: editor
rama: t/T-0013-props-pulido
creada: 2026-11-15
---

# T-0013 — Props y pulido del telón

## Objetivo
Sumar los props de Rodin (cajas y cámara de cine) repartidos por toda la sala y sacar la línea clara de la base del telón.
Es el último cambio de contenido antes del package final (freeze 20:00). No hay C++ nuevo.

## Contexto
- `Tools/unreal/build_dressing.py` (fb69d55): importa cada prop en `/Game/Dressing/<SM_x>/` (así no se pisan `model`,
  `texture_*`), reparte 6 props en X 700, 1900, 3300, 4600, 5700 y 7000, y M_Backdrop oscurece el tercio inferior.
- FBX nuevos: `Art/Export/Dressing/SM_Crates.fbx`, `Art/Export/Dressing/SM_FilmCamera.fbx`.
- Los assets genéricos que quedaron sueltos en `/Game/Dressing/` (T-0012) no se tocan ni se borran ahora.

## Pasos
1. `git fetch` y `git checkout t/T-0013-props-pulido` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop`.
2. Locks: `git lfs lock Game/Content/Maps/L_Slice.umap` y `git lfs lock Game/Content/Dressing/M_Backdrop.uasset`.
3. `.\Tools\scripts\tdc.ps1 python -Script Tools\unreal\build_dressing.py`. Tiene que importar `SM_Crates`, `SM_FilmCamera` y
   `SM_Spotlight` en sus subcarpetas, sin `[TDC] ERROR`. Si el commandlet no alcanza, agregá `-InEditor`.
4. Dos capturas: `.\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0013 -Name 01-props-a` y después `-Name 01-props-b`
   (la b es la que vale). Describí: ¿hay al menos un prop en cuadro? ¿desapareció la línea clara entre el telón y el escenario?
   ¿los colores de lectura y el director se siguen distinguiendo?
5. Commit de `Game/Content/Dressing/**` y `Game/Content/Maps/L_Slice.umap` (LFS), la evidencia y el Reporte. `git lfs unlock`
   de lo bloqueado. Push y `[TDC-REPORT]`.
6. `.\Tools\scripts\tdc.ps1 play`.

## Criterios de aceptación
- [ ] `build_dressing.py` termina sin `[TDC] ERROR` y los 3 props quedan en `/Game/Dressing/<SM_x>/`.
- [ ] `01-props-b.png` muestra al menos un prop, sin línea clara en la base del telón.

## Permite
- Editar `Tools/unreal/build_dressing.py` solo para compatibilidad de API, sin cambiar la ubicación de los props.

## No hacer
- No borrar los assets viejos de `/Game/Dressing/`. No commitear `Tools/unreal/_diag_*`.

## Evidencia pedida
`docs/evidence/T-0013/01-props-b.png` + extracto del log de `build_dressing.py` en el Reporte.

---

## Reporte (lo completa Windows)
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
