# The Director's Cut

Metroidvania 2.5D en Unreal Engine 5.8 cuyo pilar es la **Dirección**: el jugador es un director atrapado en su película, con
poderes en tres departamentos (Edición, Escenografía, Dirección de fotografía). Visión y features: `docs/product/vision.md`;
decisiones: `docs/product/decisiones.md`.

## Dos máquinas, dos agentes

Este repo lo trabajan dos Claude Code en paralelo, comunicados por VirtualKvm (`mcp__vkvm__*`) y git.

- **Si estás en macOS** sos la **Mac** (dirección técnica): diseño, C++, scripts, docs, revisión y merge. No hay Unreal ni Blender acá.
  Para todo lo que los necesite usá la skill **`delegar-a-windows`**.
- **Si estás en Windows** sos el **estudio**: Unreal, Blender, compilación, assets, pruebas y evidencia.
  Usá la skill **`estudio-windows`** y revisá el inbox de VirtualKvm al empezar.

Skills de dominio (de terceros, ver `.claude/skills/THIRD_PARTY.md`): `ue-*` para C++/Unreal (UE 5.8, la versión del proyecto; contexto
en `.agents/ue-project-context.md`) y `text-to-blender` + `blender-*` para Blender (requieren Blender MCP en Windows).
UI: `game-ui-ux` (layout, escalado, foco con gamepad) + `ue-ui-umg-slate` (UMG, Common UI y Slate en C++).
Diseño de juego: `game-docs` mantiene `docs/product/` (índice de features en `vision.md`, notas en `features/<slug>/`); después de
cambiar diseño o código correr `python3 .claude/skills/game-docs/scripts/check_docs.py docs/product --gdd vision.md`.

Protocolo (mensajes, ramas, autorización): `docs/pipeline/protocolo-mac-windows.md`.
Comandos de Windows: `docs/pipeline/comandos-windows.md` · `Tools/scripts/tdc.ps1`. Convenciones: `docs/pipeline/convenciones.md`.

## Reglas

- **Repo público.** Nunca commitear claves, tokens, `.env`, credenciales ni rutas personales. gitleaks corre en pre-commit
  (`git config core.hooksPath .githooks` una vez por clon) y en CI. Nunca saltear el hook (`--no-verify`).
- `main` solo recibe merges de PRs (los hace la Mac). Trabajo en ramas `t/T-XXXX-slug`, una por tarea (`docs/tasks/`).
- Binarios (`.uasset`, `.umap`, `.blend`) por Git LFS y solo los edita Windows, con `git lfs lock`.
- C++ en `Game/Source/` lo escribe la Mac; Windows lo compila y reporta errores.
- Idioma: docs, tareas y mensajes en español; código, nombres de clases y assets en inglés.
