"""preview.png: frames clave sobre noche + original pixelfx (Nearest) al lado.
Uso (desde vfx-pintados-2/): python3 src/preview.py <tdc-preview dir> [salida]"""
import json, os, sys
import numpy as np
from PIL import Image, ImageDraw, ImageFont

ROOT = sys.argv[1]
OUTP = sys.argv[2] if len(sys.argv) > 2 else "preview.png"
FX = [  # nombre, original, frames clave
    ("extra_defeat", "vfx/extra_defeat/extra_defeat.png", [0, 3, 6, 12, 19, 27, 34], 13),
    ("extra_stun", "vfx/extra_stun/extra_stun.png", [0, 12, 24, 36, 48, 59], 0),
    ("scripted_bounce", "vfx/scripted_bounce/scripted_bounce.png", [0, 2, 4, 8, 12, 15, 18], 7),
    ("rewind", "vfx/rewind/rewind.png", [0, 9, 18, 27, 36, 53], 0),
    ("light_toggle", "vfx/light_toggle/light_toggle.png", [0, 2, 5, 9, 14, 22, 30], 6),
    ("light_piece_appear", "vfx2/light_piece_appear.png", [0, 2, 4, 6, 8], 4),
    ("light_piece_vanish", "vfx2/light_piece_vanish.png", [0, 2, 4, 6, 8], 5),
    ("dark_piece_appear", "vfx2/dark_piece_appear.png", [0, 2, 4, 6, 8], 4),
    ("dark_piece_vanish", "vfx2/dark_piece_vanish.png", [0, 2, 4, 6, 8], 5),
]
ROWH = 196
W = 1900
H = 60 + len(FX) * (ROWH + 46)
y = np.linspace(0, 1, H)[:, None, None]
bg = (np.array([10, 14, 32]) * (1 - y) + np.array([22, 28, 58]) * y) * np.ones((H, W, 3))
img = Image.fromarray(bg.astype(np.uint8)).convert("RGBA")
d = ImageDraw.Draw(img)
try:
    F = ImageFont.truetype("/System/Library/Fonts/Supplemental/Arial.ttf", 19)
    f = ImageFont.truetype("/System/Library/Fonts/Supplemental/Arial.ttf", 13)
except Exception:
    F = f = ImageFont.load_default()
d.text((24, 16), "VFX pintados (2da tanda) - frames clave sobre noche; a la derecha el original pixelfx", fill=(235, 225, 200), font=F)


def cell(path, js, i):
    im = Image.open(path).convert("RGBA")
    px = js["meta"]["pixelfx"]
    cw, ch = px["frameSize"]
    c = px["columns"]
    return im.crop(((i % c) * cw, (i // c) * ch, (i % c + 1) * cw, (i // c + 1) * ch))


yy = 60
for name, orig, keys, okey in FX:
    js = json.load(open(name + ".json"))
    px = js["meta"]["pixelfx"]
    cw, ch = px["frameSize"]
    d.text((24, yy), f"{name}   {px['columns']}x{js['meta']['size']['h'] // ch}, {px['frames']} fr, {px['fps']} fps, "
                     f"{'loop' if px['loop'] else 'one-shot'}   celda {cw}x{ch}", fill=(235, 225, 200), font=F)
    s = min(ROWH / ch, 250 / cw)
    x = 24
    for k in keys:
        t = cell(name + ".png", js, k)
        t = t.resize((int(cw * s), int(ch * s)), Image.BILINEAR)
        img.alpha_composite(t, (x, yy + 26))
        d.text((x, yy + 26 + t.height - 14), f"fr {k}", fill=(150, 165, 195), font=f)
        x += t.width + 8
    ojs = {"meta": {"pixelfx": {"frameSize": [cw // 2, ch // 2], "columns": px["columns"]}}}
    o = cell(os.path.join(ROOT, orig), ojs, okey).resize((int(cw * s), int(ch * s)), Image.NEAREST)
    x = max(x + 24, W - o.width - 24)
    d.line([(x - 12, yy + 26), (x - 12, yy + 26 + o.height)], fill=(80, 90, 120), width=1)
    img.alpha_composite(o, (x, yy + 26))
    d.text((x, yy + 26 + o.height - 14), f"original fr {okey}", fill=(150, 165, 195), font=f)
    yy += ROWH + 46
img.convert("RGB").save(OUTP, optimize=True)
