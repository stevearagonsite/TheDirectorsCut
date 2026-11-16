"""Compone T_LightCone.png (aditivo, en lineal) bajo el reflector interactivo de la captura T-0033.
Uso: python3 preview.py <dir_cono> <captura> <salida>"""
import sys
import numpy as np
from PIL import Image, ImageDraw

D, BG, OUT = sys.argv[1], sys.argv[2], sys.argv[3]
V_LENS, V_FLOOR, HW_FLOOR = 0.015, 0.94, 0.90
INTENSITY = 1.8


def s2l(x): return np.where(x <= 0.04045, x / 12.92, ((x + 0.055) / 1.055) ** 2.4)
def l2s(x): x = np.clip(x, 0, 1); return np.where(x <= 0.0031308, x * 12.92, 1.055 * x ** (1 / 2.4) - 0.055)


def resize_premult(img, size):
    """PIL premultiplica RGBA al resamplear: escalar canal por canal para no tocar el premultiplicado."""
    return np.dstack([np.asarray(Image.fromarray(img[..., k].astype(np.float32), "F").resize(size, Image.LANCZOS))
                      for k in range(4)]).clip(0, 255) / 255


def add_cone(bg_lin, cone, lens_xy, floor_y, radius_px, intensity):
    """Quad: ancho = 2R/0.90; alto tal que la lente cae en V_LENS y el piso en V_FLOOR. Blend aditivo."""
    qw = int(round(2 * radius_px / HW_FLOOR))
    qh = int(round((floor_y - lens_xy[1]) / (V_FLOOR - V_LENS)))
    q = resize_premult(cone, (qw, qh))
    emis = s2l(q[..., :3]) * intensity                 # Emissive = Tex.rgb (sRGB decodificado = lineal premult)
    x0 = int(round(lens_xy[0] - qw / 2)); y0 = int(round(lens_xy[1] - V_LENS * qh))
    H, W = bg_lin.shape[:2]
    ya, xa, yb, xb = max(0, y0), max(0, x0), min(H, y0 + qh), min(W, x0 + qw)
    out = bg_lin.copy()
    out[ya:yb, xa:xb] += emis[ya - y0:yb - y0, xa - x0:xb - x0]
    return out


def to8(lin): return Image.fromarray((l2s(lin) * 255).round().astype(np.uint8))


bg = s2l(np.asarray(Image.open(BG).convert("RGB")).astype(np.float32) / 255)
cone = np.asarray(Image.open(f"{D}/T_LightCone.png"))
# T-0033: esfera interactiva en (972, 298); plano de juego (pie del director / plataforma) en y≈470.
# Escala: director 80 px ≈ 180 cm -> 0.44 px/cm -> R = 500 cm ≈ 222 px.
LENS, FLOOR, R = (972, 300), 470, 222
comp = to8(add_cone(bg, cone, LENS, FLOOR, R, INTENSITY))
dark = to8(add_cone(bg * 0.35, cone, LENS, FLOOR, R, INTENSITY))   # simula punto 6 (SkyLight/Sun bajos)
before = to8(bg)

th, tw = 720, 360
t = resize_premult(cone, (tw, th))
panel = to8(np.full((th, tw, 3), s2l(0.08), np.float32) * (1 - t[..., 3:]) + s2l(t[..., :3]))
alpha = Image.fromarray((t[..., 3] / 0.45 * 255).clip(0, 255).astype(np.uint8)).convert("RGB")

box = (722, 250, 1222, 520)                                         # 500x270 -> zoom x2
z = lambda im: im.crop(box).resize((1000, 540), Image.LANCZOS)
Wt = 1280 + 2 * tw + 16
out = Image.new("RGB", (Wt, 720 + 8 + 540), (12, 12, 16))
out.paste(comp, (0, 0)); out.paste(panel, (1288, 0)); out.paste(alpha, (1288 + tw + 8, 0))
out.paste(z(comp), (0, 728)); out.paste(z(dark), (1008, 728))
d = ImageDraw.Draw(out)
for xy, s in [((10, 700), f"T-0033 + T_LightCone aditivo x{INTENSITY}, R=500cm (~222px), lente en la esfera"),
              ((1294, 6), "T_LightCone (over premult)"), ((1294 + tw + 8, 6), "alpha / 0.45"),
              ((10, 734), "zoom x2: escena actual"), ((1018, 734), "zoom x2: fondo x0.35 (luz del punto 6)")]:
    d.text(xy, s, fill=(235, 235, 235))
out.save(OUT)
print(out.size)
