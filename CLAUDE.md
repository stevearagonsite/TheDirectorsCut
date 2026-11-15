# The Director's Cut

Metroidvania 2.5D en Unreal Engine 5 cuyo pilar es la **Dirección**: el jugador es un director de cine que prende/apaga luces,
rebobina el tiempo, corta escenas y cambia la escenografía para avanzar. Visión: `docs/product/vision.md`.

## Dos máquinas, dos agentes

Este repo lo trabajan dos Claude Code en paralelo, comunicados por VirtualKvm (`mcp__vkvm__*`) y git.

- **Si estás en macOS** sos la **Mac** (dirección técnica): diseño, C++, scripts, docs, revisión y merge. No hay Unreal ni Blender acá.
  Para todo lo que los necesite usá la skill **`delegar-a-windows`**.
- **Si estás en Windows** sos el **estudio**: Unreal, Blender, compilación, assets, pruebas y evidencia.
  Usá la skill **`estudio-windows`** y revisá el inbox de VirtualKvm al empezar.

Protocolo (mensajes, ramas, autorización): `docs/pipeline/protocolo-mac-windows.md`.
Comandos de Windows: `docs/pipeline/comandos-windows.md` · `Tools/scripts/tdc.ps1`. Convenciones: `docs/pipeline/convenciones.md`.

## Reglas

- **Repo público.** Nunca commitear claves, tokens, `.env`, credenciales ni rutas personales. gitleaks corre en pre-commit
  (`git config core.hooksPath .githooks` una vez por clon) y en CI. Nunca saltear el hook (`--no-verify`).
- `main` solo recibe merges de PRs (los hace la Mac). Trabajo en ramas `t/T-XXXX-slug`, una por tarea (`docs/tasks/`).
- Binarios (`.uasset`, `.umap`, `.blend`) por Git LFS y solo los edita Windows, con `git lfs lock`.
- C++ en `Game/Source/` lo escribe la Mac; Windows lo compila y reporta errores.
- Idioma: docs, tareas y mensajes en español; código, nombres de clases y assets en inglés.
