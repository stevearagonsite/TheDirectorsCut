"""Cono de luz del reflector (The Director's Cut). numpy + PIL, sin scipy.
Uso: python3 cono_luz.py <out_dir>
Salida: T_LightCone.png (1024x2048, RGB premultiplicado + A), T_LightCone_dust.png (512x512, 4x4 frames).
"""
import math, os, sys
import numpy as np
from PIL import Image

OUT = sys.argv[1] if len(sys.argv) > 1 else "."
W, H = 1024, 2048
COL_CORE = np.array([255, 228, 150], np.float32) / 255   # centro, casi blanco cálido
COL_MECH = np.array([255, 210, 90], np.float32) / 255    # amarillo de la mecánica
COL_EDGE = np.array([255, 186, 70], np.float32) / 255    # borde, más ámbar
A_PEAK = 0.45

# Geometría en UV (u: -1..1 a lo ancho, v: 0..1 de arriba a abajo)
V_LENS, V_FLOOR = 0.015, 0.94     # lente y centro de la elipse del piso
HW_LENS, HW_FLOOR = 0.045, 0.90   # medio ancho del haz en la lente y en el piso (0.90 -> 2R)
POOL_HV = 0.05                    # medio alto de la elipse en v


def s2l(x): return np.where(x <= 0.04045, x / 12.92, ((x + 0.055) / 1.055) ** 2.4)
def l2s(x): x = np.clip(x, 0, 1); return np.where(x <= 0.0031308, x * 12.92, 1.055 * x ** (1 / 2.4) - 0.055)


def ss(e0, e1, x):
    t = np.clip((x - e0) / (e1 - e0), 0.0, 1.0)
    return t * t * (3 - 2 * t)


def blur(img, sigma):
    if sigma <= 0:
        return img
    h, w = img.shape[:2]
    fy = np.fft.fftfreq(h)[:, None]; fx = np.fft.fftfreq(w)[None, :]
    g = np.exp(-2 * math.pi ** 2 * sigma * sigma * (fx * fx + fy * fy))
    return np.real(np.fft.ifft2(np.fft.fft2(img) * g)).astype(np.float32)


def fbm(h, w, beta, seed, lo=2.0):
    r = np.random.default_rng(seed)
    wn = r.standard_normal((h, w))
    fy = np.fft.fftfreq(h)[:, None] * h; fx = np.fft.fftfreq(w)[None, :] * w
    f = np.sqrt(fx * fx + fy * fy)
    amp = np.where(f < 1e-6, 0, 1 / np.maximum(f, lo) ** beta)
    n = np.real(np.fft.ifft2(np.fft.fft2(wn) * amp))
    return ((n - n.min()) / (n.max() - n.min())).astype(np.float32)


def noise1d(n, beta, seed, lo=2.0):
    r = np.random.default_rng(seed)
    wn = r.standard_normal(n)
    f = np.abs(np.fft.fftfreq(n) * n)
    amp = np.where(f < 1e-6, 0, 1 / np.maximum(f, lo) ** beta)
    x = np.real(np.fft.ifft(np.fft.fft(wn) * amp))
    return ((x - x.min()) / (x.max() - x.min())).astype(np.float32)


def cone():
    yy, xx = np.mgrid[0:H, 0:W].astype(np.float32)
    u = (xx + 0.5) / W * 2 - 1
    v = (yy + 0.5) / H
    t = np.clip((v - V_LENS) / (V_FLOOR - V_LENS), 0, 1)          # 0 lente -> 1 piso
    hw = HW_LENS + (HW_FLOOR - HW_LENS) * t
    s = u / hw                                                     # coordenada angular (-1..1 = borde del haz)
    sa = np.abs(s)

    # --- perfil lateral: núcleo gaussiano + meseta con borde difuso ---
    lateral = 0.45 * np.exp(-(s / 0.40) ** 2) + 0.55 * (1 - ss(0.62, 1.04, sa))
    # --- perfil longitudinal: fuerte en la lente, se diluye al abrirse ---
    along = 0.68 + 0.32 * (1 - t) ** 1.6
    fade_in = ss(V_LENS - 0.006, V_LENS + 0.035, v)
    fade_out = 1 - ss(V_FLOOR - 0.012, V_FLOOR + 0.022, v)        # el haz termina en el piso
    beam = lateral * along * fade_in * fade_out

    # --- estrías radiales (god rays pintados): ruido 1D en s, que varía lento con t ---
    n1 = noise1d(512, 1.1, 11); n2 = noise1d(512, 1.4, 12)
    idx = (s * 0.5 + 0.5) * 511
    i0 = np.clip(idx, 0, 511)
    streak = np.interp(i0.ravel(), np.arange(512), n1).reshape(H, W) * (1 - t) + \
             np.interp(i0.ravel(), np.arange(512), n2).reshape(H, W) * t
    streak = 0.80 + 0.26 * streak
    # --- humo suave: fBm de baja frecuencia ---
    smoke = fbm(H // 4, W // 4, 2.2, 21)
    smoke = np.array(Image.fromarray(smoke).resize((W, H), Image.BICUBIC))
    smoke = 0.78 + 0.32 * smoke
    beam = beam * streak * smoke

    # --- elipse del piso ---
    e = np.sqrt((u / (HW_FLOOR * 1.02)) ** 2 + ((v - V_FLOOR) / POOL_HV) ** 2)
    pool = 0.55 * np.exp(-(e / 0.45) ** 2) + 0.45 * (1 - ss(0.55, 1.0, e))
    pool *= 0.88 + 0.12 * smoke

    # --- brillo de la lente ---
    le = np.sqrt((u / 0.07) ** 2 + ((v - V_LENS - 0.008) / 0.012) ** 2)
    lens = np.exp(-(le / 0.7) ** 2)

    a = 0.40 * beam + A_PEAK * pool + 0.42 * lens
    a = np.minimum(a, A_PEAK)

    # --- polvo: motas blandas solo donde hay haz ---
    rng = np.random.default_rng(7)
    dust = np.zeros((H, W), np.float32)
    n = 380
    for _ in range(n):
        tt = rng.uniform(0.04, 0.97); sv = rng.normal(0, 0.45)
        if abs(sv) > 1: continue
        vy = V_LENS + tt * (V_FLOOR - V_LENS)
        hwv = HW_LENS + (HW_FLOOR - HW_LENS) * tt
        cx = (sv * hwv * 0.5 + 0.5) * W; cy = vy * H
        r = rng.choice([1.2, 1.8, 2.6, 4.0, 7.0], p=[0.35, 0.3, 0.2, 0.1, 0.05])
        amp = rng.uniform(0.25, 1.0) * (0.5 if r > 5 else 1.0)
        R = int(r * 3 + 2)
        x0, y0 = int(cx) - R, int(cy) - R
        gy, gx = np.mgrid[y0:y0 + 2 * R + 1, x0:x0 + 2 * R + 1].astype(np.float32)
        g = amp * np.exp(-((gx + 0.5 - cx) ** 2 + (gy + 0.5 - cy) ** 2) / (2 * r * r))
        ya, xa = max(0, y0), max(0, x0); yb, xb = min(H, y0 + 2 * R + 1), min(W, x0 + 2 * R + 1)
        dust[ya:yb, xa:xb] = np.maximum(dust[ya:yb, xa:xb], g[ya - y0:yb - y0, xa - x0:xb - x0])
    lit = np.clip(beam / 0.6, 0, 1)
    a = np.minimum(a + 0.10 * dust * lit, A_PEAK)

    # --- color: centro casi blanco, borde ámbar ---
    k_core = np.clip(np.maximum(np.exp(-(s / 0.3) ** 2) * (1 - t * 0.6), np.maximum(pool * 0.8, lens)), 0, 1)
    k_edge = ss(0.5, 1.0, sa) * (1 - pool)
    col = COL_MECH[None, None] * (1 - k_core[..., None]) + COL_CORE[None, None] * k_core[..., None]
    col = col * (1 - k_edge[..., None]) + COL_EDGE[None, None] * k_edge[..., None]

    # borde de la textura = 0 (clamp sin franjas)
    win = ss(0, 12, xx) * ss(0, 12, W - 1 - xx) * ss(0, 6, yy) * ss(0, 12, H - 1 - yy)
    a = a * win
    a = np.where(a < 0.5 / 255, 0, a)
    # premultiplicado en LINEAL y codificado sRGB: UE (sRGB=on) decodifica RGB -> lin(col)*a exacto; A queda lineal
    rgb = l2s(s2l(col) * a[..., None])
    out = np.dstack([rgb, a]) * 255
    return np.round(np.clip(out, 0, 255)).astype(np.uint8)


def dust_sheet(F=16, S=128, cols=4):
    """Motas que suben en loop; cada frame tilea en X e Y. Premultiplicado."""
    rng = np.random.default_rng(3)
    N = 70
    x0 = rng.uniform(0, S, N); y0 = rng.uniform(0, S, N)
    laps = rng.choice([1, 1, 1, 2], N)              # vueltas verticales por loop (lento)
    sway = rng.uniform(1.0, 4.0, N); ph = rng.uniform(0, 2 * math.pi, N)
    r = rng.choice([0.9, 1.3, 1.8, 2.6], N, p=[0.35, 0.35, 0.2, 0.1])
    amp = rng.uniform(0.35, 1.0, N); tw = rng.integers(1, 3, N); tph = rng.uniform(0, 2 * math.pi, N)
    frames = []
    yy, xx = np.mgrid[0:S, 0:S].astype(np.float32) + 0.5
    for f in range(F):
        p = f / F
        a = np.zeros((S, S), np.float32)
        for i in range(N):
            cx = (x0[i] + sway[i] * math.sin(2 * math.pi * p + ph[i])) % S
            cy = (y0[i] - laps[i] * S * p) % S
            dx = (xx - cx + S / 2) % S - S / 2; dy = (yy - cy + S / 2) % S - S / 2
            k = amp[i] * (0.6 + 0.4 * math.sin(2 * math.pi * tw[i] * p + tph[i]))
            a = np.maximum(a, k * np.exp(-(dx * dx + dy * dy) / (2 * r[i] ** 2)))
        a = np.clip(a * 0.9, 0, 0.9)
        rgb = l2s(s2l(COL_CORE)[None, None] * a[..., None])
        frames.append(np.round(np.dstack([rgb, a]) * 255).astype(np.uint8))
    sheet = np.zeros((S * (F // cols), S * cols, 4), np.uint8)
    for i, fr in enumerate(frames):
        rr, cc = divmod(i, cols)
        sheet[rr * S:(rr + 1) * S, cc * S:(cc + 1) * S] = fr
    return sheet


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    c = cone()
    Image.fromarray(c, "RGBA").save(os.path.join(OUT, "T_LightCone.png"))
    a = c[..., 3] / 255.0
    print("cone alpha max %.3f  borde max %.4f  cobertura>0 %.1f%%" % (
        a.max(), max(a[0].max(), a[-1].max(), a[:, 0].max(), a[:, -1].max()), (a > 0).mean() * 100))
    d = dust_sheet()
    Image.fromarray(d, "RGBA").save(os.path.join(OUT, "T_LightCone_dust.png"))
    print("dust", d.shape)
