---
id: T-0004
titulo: Instalar Earshot (sonidos + voz) en Windows
estado: nueva
tipo: setup
rama: t/T-0004-earshot-windows
creada: 2026-11-15
---

# T-0004 — Earshot en Windows

## Objetivo
Que Steve escuche lo que hace el estudio Windows (compilaciones, Blender, fallos, permisos) sin mirar esa pantalla.
En la Mac ya está instalado (solo sonidos: la voz de Earshot es solo Windows).

## Contexto
Plugin `earshot` 1.1.0 del directorio de Anthropic (https://github.com/olegthesnowman/earshot, MIT), revisado en la Mac:
sin red, lee el final del transcript solo para decir el mensaje de cierre, logs apagados por defecto.

## Pasos
1. `claude plugin install earshot@anthropic-plugin-directory --scope user`
2. Verificar que la versión instalada sea `1.1.0-fa158b530993` (carpeta en `~/.claude/plugins/cache/anthropic-plugin-directory/earshot/`).
3. Voz: funciona con SAPI de Windows sin nada extra. Si Steve usa NVDA: `pip install accessible_output2`.
4. En `config.json` del plugin: agregar a `focus_process_names` el proceso de la terminal que se use (ej. `windowsterminal.exe`, `wezterm-gui.exe`).
   Dejar `log_spoken` en `false`.
5. Reiniciar Claude Code y probar: `python scripts/play.py stop --test` desde la carpeta del plugin.

## Criterios de aceptación
- [ ] Suena al terminar un turno y al fallar una herramienta.
- [ ] Lee el mensaje de cierre en voz alta con Claude Code enfocado.

## Permite
- Instalar el plugin a nivel usuario y `accessible_output2` (pip). Nada se commitea al repo.

## No hacer
- No activar `log_spoken` (graba textualmente todo lo dicho).

---

## Reporte (lo completa Windows)
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
