# Entrega: 2026 Epic AI MegaJam

**Cierre: hoy, 15/11, 22:59 (ART). Subir a las 22:00 como máximo.** Antes, verificar fecha, hora y reglas en la página oficial de itch.io.

## Checklist

- [ ] Build Win64 empaquetado (`tdc.ps1 package`, T-0006), probado en la PC: abre, carga L_Slice y se juega de punta a punta.
- [ ] Zip `stevearagonsite_TheDirectorsCut.zip` de menos de 1 GB. Si pesa más, se sube con butler.
- [ ] Video de gameplay de 30–60 s, con gameplay en los primeros 5 s (ver guion abajo).
- [ ] Página de itch.io con el texto de abajo, el key art, capturas y lista de herramientas.
- [ ] Datos del equipo: nombre y email de Epic de cada miembro. Los carga Steve.
- [ ] Link al dev log: [`docs/devlog.md`](../devlog.md), en T-0006. Es para el premio Glass Box.

## Key art para itch.io

En [`docs/jam/arte/`](arte/):

| Archivo | Uso en itch.io | Tamaño |
|---|---|---|
| `cover_630x500.png` | Cover image | 630×500 |
| `banner_960x300.png` | Banner del encabezado (Edit theme → Banner) | 960×300 |
| `keyart_1920x1080.png` | Primera captura de la galería / portada del video | 1920×1080 |

Hechos con Gemini, sobre la hoja de modelo del director; el título lo agrega `work/title.py` (Futura Condensed ExtraBold).

## Texto para itch.io (en inglés)

**The Director's Cut**

*You're a film director trapped inside your own unfinished movie. Don't just fight the set: **direct** it.*

A 2.5D side-scroller built in Unreal Engine 5.8, where every power is a real tool of the film crew:

- **Lights!** Switch the stage spotlights on and off. Yellow set pieces only exist in the light, violet ones only in darkness.
- **Rewind the take.** Hold to rewind anything "on take" (cyan): a collapsed bridge rises back, a fallen crate returns to its shelf.
- **Clapperboard hit.** Your clapperboard knocks out red extras and stuns orange ones. Black "scripted" extras shrug it off: you have to direct the scene to get them out of the shot.

One noir film set, four beats: light, shadow, time, and a final combo.

**Controls:** A/D move · Space jump · F clapperboard hit · E toggle nearest spotlight · Q (hold) rewind · Gamepad: stick, A, B, X, LT.

**Made by** stevearagonsite, with a team of AI agents working across two machines (a Mac for design and code, a Windows PC for Unreal and Blender). See the dev log.

## Herramientas (para los premios de partners)

Unreal Engine 5.8 · Claude Code (Anthropic) · Hyper3D Rodin (personaje: imagen → 3D, auto-rig y animaciones) · Google Gemini (hoja de modelo del personaje) · Blender 5.3 + blender-mcp · VirtualKvm (mensajería entre agentes) · Git + Git LFS.

## Guion del video (30–60 s, 1920×1080)

Se graba con la Game Bar de Windows (Win+Alt+R), en el build empaquetado, y se recorta a ~45 s.

| Tiempo | Plano | Qué se ve |
|---|---|---|
| 0–5 s | Gameplay directo | El director corre por Floor_A y le da un claquetazo al extra rojo, que sale de la toma. Nada de logo antes. |
| 5–12 s | Beat 1 | Prende L1 con E: aparece la plataforma amarilla y la cruza saltando. |
| 12–22 s | Beat 2 | Apaga L2: se abre la pared amarilla y aparece la sombra de guion. El claquetazo rebota ("Rebota: es un extra de guion"). Prende L2 y la sombra sale de la toma. |
| 22–35 s | Beat 3 | Se cae el puente. Mantiene Q, el puente vuelve a su lugar y lo cruza. |
| 35–45 s | Beat 4 | Aturde al extra naranja. Apaga L3: cae la caja y aparece el escalón. Sube a la salida verde. |
| 45–50 s | Cierre | Título "The Director's Cut" sobre el último frame. |

## Riesgos y plan B

- **El personaje Rodin no entra o anima mal:** queda el greybox (cilindro). El build sale igual.
- **T-0009 no compila a tiempo:** se empaqueta sin claquetazo, y entonces se sacan del texto y del video el claquetazo y los extras.
- **Package de las 21:00 falla:** se empaqueta el último commit que compiló (main con T-0008) para llegar con algo jugable.
