---
id: T-0003
titulo: Instalar Blender MCP y probar el pipeline Blender -> Unreal
estado: cancelada
tipo: blender
rama: t/T-0003-blender-mcp
creada: 2026-11-15
---


> **Cancelada (2026-11-15): el worker de Blender de VirtualKvm (blender-mcp en la PC) reemplazó la instalación de esta tarea. El prop Spotlight queda para después de la jam.**
# T-0003 — Blender MCP + primer asset del pipeline

## Objetivo
Que Claude en Windows pueda manejar Blender por MCP (lo requieren las skills `text-to-blender` y `blender-*`) y validar
el pipeline completo con un prop simple del set: un reflector de cine (`Spotlight`).

## Contexto
- Backends soportados: `.claude/skills/text-to-blender/references/mcp-backends.md` (Blender Lab oficial con Blender >= 5.1,
  o `ahujasid/blender-mcp` con Blender >= 4.0). Elegí según tu versión de Blender y anotalo.
- Convenciones Blender -> Unreal: `docs/pipeline/convenciones.md`. Export: `Tools/blender/export_fbx.py`.

## Pasos
1. Instalar y habilitar el backend MCP elegido (add-on + servidor MCP en Claude Code de Windows). Verificar con una consulta de escena.
2. Con las skills de Blender, modelar `Spotlight`: reflector de cine low-poly (base trípode, cuerpo cilíndrico, aletas/barndoors),
   ~1.8 m de alto, pivote en la base, materiales simples (metal oscuro + vidrio emisivo). Colección `Export`.
3. Guardar `Art/Source/Spotlight.blend` y exportar con
   `tdc.ps1 blender -Blend Art\Source\Spotlight.blend -Script Tools\blender\export_fbx.py -- --out Art\Export\Spotlight.fbx`.
4. Importar en Unreal a `/Game/Props/Spotlight/SM_Spotlight` (por script Python o a mano) y verificar escala contra el personaje.

## Criterios de aceptación
- [ ] MCP de Blender funcionando (anotar backend, versión y cómo se arranca).
- [ ] `Spotlight.blend` + `Spotlight.fbx` + `SM_Spotlight` en el repo (LFS).
- [ ] Escala correcta en Unreal: ~180 cm de alto, apoyado en el piso, mirando a +X.
- [ ] Capturas: `docs/evidence/T-0003/blender.png` (viewport) y `docs/evidence/T-0003/unreal.png` (junto al personaje).

## Permite
- Instalar el add-on/servidor de Blender MCP y registrarlo en la config de Claude Code de Windows (fuera del repo).
- Crear `Tools/unreal/import_assets.py` si hace falta (reportarlo).

## No hacer
- No commitear configuración local de MCP, rutas personales ni tokens.

## Evidencia pedida
Las 2 capturas + pasos exactos de arranque del MCP (para documentarlos en `docs/pipeline/`).

---

## Reporte (lo completa Windows)
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
