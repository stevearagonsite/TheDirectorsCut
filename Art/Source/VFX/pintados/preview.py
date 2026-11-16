import sys
import numpy as np
from PIL import Image, ImageDraw, ImageFont
orig = sys.argv[1]
def cells(p, cw, ch, cols, idx):
    im = Image.open(p).convert("RGBA")
    return [im.crop(((i % cols) * cw, (i // cols) * ch, (i % cols + 1) * cw, (i // cols + 1) * ch)) for i in idx]
W, H = 1800, 1560
y = np.linspace(0, 1, H)[:, None, None]
bg = (np.array([10, 14, 32]) * (1 - y) + np.array([22, 28, 58]) * y) * np.ones((H, W, 3))
img = Image.fromarray(bg.astype(np.uint8)).convert("RGBA")
d = ImageDraw.Draw(img)
try:
    F = ImageFont.truetype("/System/Library/Fonts/Supplemental/Arial.ttf", 22); f = ImageFont.truetype("/System/Library/Fonts/Supplemental/Arial.ttf", 15)
except Exception:
    F = f = ImageFont.load_default()
def put(tile, x, y, scale, label=None, filt=Image.BILINEAR):
    t = tile.resize((int(tile.width * scale), int(tile.height * scale)), filt)
    img.alpha_composite(t, (x, y))
    if label: d.text((x, y + t.height + 3), label, fill=(160, 175, 205), font=f)
    return t.width
d.text((30, 18), "VFX pintados - The Director's Cut (frames clave sobre noche)", fill=(235, 225, 200), font=F)
# lluvia
d.text((30, 60), "anim_lluvia  8x8, 60 fr, 30 fps, loop 2 s  (celda 256x256)", fill=(235, 225, 200), font=F)
x = 30
for i, c in zip([0, 20, 40, 59], cells("anim_lluvia.png", 256, 256, 8, [0, 20, 40, 59])):
    x += put(c, x, 95, 1.2, f"frame {i}") + 16
x += 20
put(cells(orig + "/anim_lluvia.png", 256, 256, 8, [0])[0], x, 95, 1.2, "original pixelfx (frame 0)", Image.NEAREST)
# vapor
d.text((30, 450), "anim_vapor  14x7, 90 fr, 30 fps, loop 3 s  (celda 128x256)", fill=(235, 225, 200), font=F)
x = 30
for i, c in zip([0, 15, 30, 45, 60, 75, 89], cells("anim_vapor.png", 128, 256, 14, [0, 15, 30, 45, 60, 75, 89])):
    x += put(c, x, 485, 1.5, f"frame {i}") + 14
x += 30
put(cells(orig + "/anim_vapor.png", 128, 256, 14, [0])[0], x, 485, 1.5, "original (frame 0)", Image.NEAREST)
# claquetazo
d.text((30, 905), "claquetazo_impact  3x4, 10 fr, 30 fps, one-shot  (celda 256x160, aspecto 1.6 = original)", fill=(235, 225, 200), font=F)
x, yy = 30, 940
for k, c in enumerate(cells("claquetazo_impact.png", 256, 160, 3, range(9))):
    if k == 5: x, yy = 30, 1215
    x += put(c, x, yy, 1.25, f"frame {k}") + 14
o = cells(orig + "/claquetazo_impact.png", 128, 80, 3, [1, 3])
x2 = 30 + 4 * (320 + 14) + 30
put(o[0], x2, 1215, 2.5, "original fr 1", Image.NEAREST)
img.convert("RGB").save("preview.png", optimize=True)
