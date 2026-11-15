# The Director's Cut

Metroidvania 2.5D sobre un set de filmación: sos el director y avanzás **dirigiendo** la escena —
luces, tiempo, cortes y escenografía. Unreal Engine 5 + Blender.

- Visión de producto: [docs/product/vision.md](docs/product/vision.md)
- Cómo trabajamos (Mac ⇄ Windows): [docs/pipeline/protocolo-mac-windows.md](docs/pipeline/protocolo-mac-windows.md)

## Clonar

Requiere Git LFS.

```sh
git lfs install
git clone git@github.com:stevearagonsite/TheDirectorsCut.git
cd TheDirectorsCut
git config core.hooksPath .githooks   # escaneo de secretos con gitleaks antes de cada commit
```

El proyecto de Unreal está en `Game/TheDirectorsCut.uproject`.
