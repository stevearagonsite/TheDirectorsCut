# Convenciones

## Estructura

```
Game/                     Proyecto Unreal (TheDirectorsCut.uproject)
  Source/TheDirectorsCut/   C++ (Mac)
  Content/                  Assets (Windows)
    Maps/  Characters/  Props/  Sets/  Materials/  Blueprints/  Input/  UI/
  Config/                   .ini (Windows; la Mac propone cambios en la tarea)
Art/
  Source/                   .blend fuente
  Export/                   .fbx exportados (los importa Unreal)
Tools/
  blender/                  scripts Python de Blender
  unreal/                   scripts Python del editor
  scripts/                  tdc.ps1 y utilidades
docs/
  product/  pipeline/  tasks/  evidence/
```

## Nombres de assets (Unreal)

Prefijo + nombre en PascalCase en inglés: `BP_` Blueprint, `SM_` static mesh, `SK_` skeletal mesh, `M_` material, `MI_` instancia de material, `T_` textura, `A_` animación, `IA_`/`IMC_` input action/mapping context, `L_` mapa, `DA_` data asset, `WBP_` widget.

## C++

- Prefijo de clases `TDC` (`ATDCCharacter`, `UTDCDirectableComponent`).
- Un sistema por carpeta en `Source/TheDirectorsCut/{Public,Private}/`: `Core/`, `Character/`, `Director/`, `Stage/` (tests en `Private/Tests/`).
- Tests de automatización con prefijo `TDC.` para poder filtrarlos.

## Blender → Unreal

- Unidades: métrico, escala 1.0 = 1 m; Unreal usa cm (el exportador aplica la escala).
- Eje: frente −Y en Blender, el exportador convierte a +X de Unreal.
- Pivote en la base del objeto, transforms aplicados antes de exportar.
- Un `.blend` por asset (o por set), mismo nombre que el asset de Unreal sin prefijo: `Art/Source/Spotlight.blend` → `SM_Spotlight`.
- Plano de juego: el personaje se mueve en X (horizontal) y Z (vertical) de Unreal; Y es profundidad decorativa.
