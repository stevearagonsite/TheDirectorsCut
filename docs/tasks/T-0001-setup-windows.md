---
id: T-0001
titulo: Setup del estudio Windows + primera compilación
estado: nueva
tipo: setup
rama: t/T-0001-setup-windows
creada: 2026-11-15
---

# T-0001 — Setup del estudio Windows + primera compilación

## Objetivo
Dejar la PC Windows lista para trabajar el repo y comprobar que el C++ del prototipo (escrito en la Mac, nunca compilado) compila y sus tests pasan.

## Contexto
- Leé `CLAUDE.md`, `docs/pipeline/protocolo-mac-windows.md` y la skill `.claude/skills/estudio-windows/SKILL.md`.
- El `.uproject` dice `"EngineAssociation": "5.6"` como placeholder: ajustalo a la versión instalada.

## Pasos
1. Herramientas: verificá/instalá `git`, `git-lfs`, `gitleaks` (`winget install Gitleaks.Gitleaks`), `gh` (autenticado con una cuenta con permiso de push a `stevearagonsite/TheDirectorsCut`).
2. En el clon: `git lfs install`, `git config core.hooksPath .githooks`.
3. Copiá `Tools/scripts/tdc.local.example.ps1` → `Tools/scripts/tdc.local.ps1` con las rutas reales de Unreal y Blender. `.\Tools\scripts\tdc.ps1 doctor` tiene que dar todo OK.
4. Ajustá `EngineAssociation` en `Game/TheDirectorsCut.uproject` a tu versión de UE (ej. `"5.6"`, `"5.5"`).
5. `.\Tools\scripts\tdc.ps1 projectfiles` y `.\Tools\scripts\tdc.ps1 build`.
6. Si compila: `.\Tools\scripts\tdc.ps1 test` (corre `TDC.*`).
7. Si `tdc.ps1` falla por el script en sí, arreglalo (permitido) y explicá qué cambiaste.

## Criterios de aceptación
- [ ] `tdc.ps1 doctor` todo OK (pegar salida redactada en el reporte).
- [ ] `TheDirectorsCutEditor Win64 Development` compila sin errores. Listar warnings de nuestro código si los hay.
- [ ] Tests `TDC.Stage.*` pasan (3 tests).
- [ ] El pre-commit con gitleaks corre al commitear.

## Permite
- Editar `Game/TheDirectorsCut.uproject` (solo `EngineAssociation`) y `Tools/scripts/tdc.ps1`.
- Instalar git-lfs, gitleaks y gh si faltan.

## No hacer
- No corregir errores de C++ en `Game/Source/`: reportarlos con archivo:línea y mensaje exacto. La Mac los arregla.
- No commitear `tdc.local.ps1`, `Binaries/`, `Intermediate/`, `Saved/`.

## Evidencia pedida
Extracto de build (errores/warnings) y resumen de tests en la sección Reporte.

---

## Reporte (lo completa Windows)
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
