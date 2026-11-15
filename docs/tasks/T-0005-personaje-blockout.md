---
id: T-0005
titulo: Personaje principal (el director): blockout y proporciones en Blender
estado: cancelada
tipo: blender
rama: t/T-0005-personaje-blockout
creada: 2026-11-15
---


> **Cancelada (2026-11-15): el protagonista salió de Hyper3D Rodin (D-014, T-0007). El blockout v3 quedó como referencia de proporciones en la PC, fuera del repo.**
# T-0005 — Personaje principal: blockout y proporciones

## Objetivo
Primer paso del protagonista en Blender: un **blockout** que fije silueta, proporciones y escala, para que Steve lo apruebe
antes de pasar a modelado final, materiales y rig (tareas siguientes). Depende de T-0003 (Blender MCP funcionando).

## Contexto
- Concepto a seguir: **concepto 1**, `docs/product/arte/referencias/personaje-director-v1.png`. Leé `docs/product/arte/direccion-de-arte.md`.
  **No** usar el personaje del concepto 2 ni el del video (bufanda roja, boina, barba): es otro.
- Decisión: D-011 en `docs/product/decisiones.md`.
- **Referencias ortográficas para montar como planos en el viewport:** `docs/product/arte/model-sheet/director-front-v1.png`,
  `director-side-v1.png`, `director-back-v1.png` y `clapperboard-v1.png` (generadas con Gemini; leé la sección "Hoja de modelo"
  de `direccion-de-arte.md`). Escalá cada plano para que la figura mida 180 cm. Mandan sobre el concepto en los detalles
  (por ejemplo, la gabardina llega a la rodilla). Requiere que el PR #4 esté en `main` y esta rama rebaseada.
- Convenciones Blender → Unreal (unidades, eje, pivote, nombres): `docs/pipeline/convenciones.md`.
- Skills: `text-to-blender` → `blender-modeling`, `blender-cameras` y `blender-rendering` para las capturas.
- En el juego, el personaje ocupa una cápsula de 40 cm de radio y 180 cm de alto (`ATDCCharacter`, `InitCapsuleSize(40, 90)`).

## Especificación
- **Altura total con gorra: 180 cm** (± 2). En pose de juego (idle), el cuerpo sin la bufanda entra en un cilindro de 80 cm de diámetro; la A-pose de bind (brazos a 40–45°) queda exenta.
- **Proporción estilizada:** cabeza grande, unas 3.5 cabezas de alto; manos y botas algo grandes para que se lean de lejos.
- **Piezas, como objetos separados con nombre:**
  - `Head` (cabeza)
  - `Cap` (gorra newsboy)
  - `Body`
  - `Coat` (gabardina gris corta, abierta, con los faldones como piezas propias para animarlos después)
  - `Vest` (chaleco)
  - `Scarf` (bufanda, con la cola separada)
  - `Legs`
  - `Boots`
  - `Hands`
  - `Clapperboard` (claqueta suelta, a escala de la mano, en la mano derecha; pivote en el punto de agarre, porque en Unreal va a un socket de la mano)
- **Pose:** A-pose de bind con los brazos a 40–45° (como la referencia), mirando a −Y de Blender, pivote en el piso entre los pies, transforms aplicados.
- **Blockout:** primitivas y formas simples. Sin detalle, sin topología final y sin materiales salvo colores planos para separar piezas.
- **Colores de blockout:** grises, oliva y tonos piel. **No** usar amarillo, violeta ni cian saturados (reservados para mecánicas).
- Todo dentro de una colección `Export`.

## Correcciones del adelanto
Revisión de gamedesigner sobre el blockout v1 (adelanto hecho fuera del repo, 2026-11-15). Aplican a esta tarea:
1. Brazos de la A-pose a 40–45°.
2. `Cap` apoyada sobre la cabeza, sin hueco: tapa la parte de arriba del pelo, visera hacia adelante.
3. Cabeza un 8–10 % más grande, hasta unas **3.5 cabezas** de alto. Los 180 cm se mantienen acortando piernas o torso.
4. Cara más redondeada, con menos mandíbula cuadrada. Ojos y cejas como planos oscuros simples, para que se lea la mirada.
5. `Clapperboard`: palo superior con bisagra como pieza separada, con franjas blancas y negras (alcanza un material de 2 colores). Pivote en el punto de agarre.
6. `Scarf`: rodea el cuello y cae en dos colas por la espalda, como en `director-back-v1.png`.

## Pasos
1. Confirmar que el MCP de Blender de T-0003 responde (consulta de escena).
2. Armar el blockout siguiendo la especificación, con el concepto 1 como imagen de referencia en el viewport.
3. Medir las dimensiones y anotarlas en el reporte: altura total, ancho de hombros, alto de la cabeza.
4. Guardar `Art/Source/Director.blend`.
5. Exportar con `.\Tools\scripts\tdc.ps1 blender -Blend Art\Source\Director.blend -Script Tools\blender\export_fbx.py -- --out Art\Export\Director.fbx`.
6. Renderizar las capturas pedidas (EEVEE alcanza) con una cámara ortográfica, junto a un cubo de referencia de 180 cm.
7. Commit (`.blend` y `.fbx` van por LFS) + evidencia, push, `[TDC-REPORT]`.

## Criterios de aceptación
- [ ] Altura total 180 ± 2 cm, medida en Blender y anotada.
- [ ] Las 10 piezas existen como objetos separados con los nombres pedidos, dentro de `Export`.
- [ ] Pivote en el piso, A-pose, mirando a −Y, transforms aplicados.
- [ ] `Director.fbx` exporta sin errores.
- [ ] Capturas de frente, perfil y 3/4 más una comparación lado a lado con el concepto 1.

## Permite
Vacío.

## No hacer
- No riggear, no animar, no texturizar ni hacer el modelado final: van en tareas siguientes.
- No importar a Unreal todavía.
- No cambiar el diseño del concepto: si algo no cierra (por ejemplo, la proporción con 180 cm), preguntarlo por `[TDC-ASK]`.

## Evidencia pedida
En `docs/evidence/T-0005/`, a 1280×720:
- `frente.png`
- `perfil.png`
- `tres-cuartos.png`
- `comparacion-concepto.png`, el blockout al lado del concepto 1

---

## Reporte (lo completa Windows)
- estado:
- sha:
- resumen:
- errores:
- evidencia:
- notas / preguntas:
