"""VFX pintados para The Director's Cut: lluvia, vapor y claquetazo (reemplazo directo de los sheets pixelfx).
Uso: python3 vfx_pintados.py <out_dir>   (numpy + PIL, sin scipy)
"""
import json, math, os, sys
import numpy as np
from PIL import Image

OUT = sys.argv[1] if len(sys.argv) > 1 else "."


def smoothstep(e0, e1, x):
    t = np.clip((x - e0) / (e1 - e0), 0.0, 1.0)
    return t * t * (3 - 2 * t)


class Canvas:
    """Acumulador premultiplicado (operador over) en float."""
    def __init__(self, h, w):
        self.a = np.zeros((h, w), np.float32)
        self.c = np.zeros((h, w, 3), np.float32)

    def over(self, y0, x0, alpha, color):
        h, w = alpha.shape
        H, W = self.a.shape
        ya, xa = max(0, y0), max(0, x0)
        yb, xb = min(H, y0 + h), min(W, x0 + w)
        if ya >= yb or xa >= xb:
            return
        s = alpha[ya - y0:yb - y0, xa - x0:xb - x0]
        col = np.asarray(color, np.float32)
        if col.ndim == 1:
            col = col[None, None, :]
        else:
            col = col[ya - y0:yb - y0, xa - x0:xb - x0]
        A = self.a[ya:yb, xa:xb]
        self.c[ya:yb, xa:xb] = s[..., None] * col + (1 - s[..., None]) * self.c[ya:yb, xa:xb]
        self.a[ya:yb, xa:xb] = s + (1 - s) * A

    def rgba(self, ss=1, fill=(200, 210, 225)):
        a, c = self.a, self.c
        if ss > 1:
            H, W = a.shape
            a = a.reshape(H // ss, ss, W // ss, ss).mean((1, 3))
            c = c.reshape(H // ss, ss, W // ss, ss, 3).mean((1, 3))
        rgb = np.where(a[..., None] > 1e-4, c / np.maximum(a[..., None], 1e-4), np.asarray(fill, np.float32) / 255)
        out = np.dstack([np.clip(rgb, 0, 1) * 255, np.clip(a, 0, 1) * 255])
        return np.round(out).astype(np.uint8)


def stroke(cv, p0, p1, w0, w1, a_fn, color, wmid=None, tex=None):
    """Trazo afinado de p0 a p1 (px). Ancho lineal w0->w1 (o perfil con pico wmid). a_fn(t)-> alpha a lo largo."""
    (x0, y0), (x1, y1) = p0, p1
    wmax = max(w0, w1, wmid or 0) * 0.5 + 2
    bx0, by0 = int(math.floor(min(x0, x1) - wmax)), int(math.floor(min(y0, y1) - wmax))
    bx1, by1 = int(math.ceil(max(x0, x1) + wmax)), int(math.ceil(max(y0, y1) + wmax))
    H, W = cv.a.shape
    if bx1 < 0 or by1 < 0 or bx0 >= W or by0 >= H:
        return
    yy, xx = np.mgrid[by0:by1, bx0:bx1].astype(np.float32)
    xx += 0.5; yy += 0.5
    dx, dy = x1 - x0, y1 - y0
    L2 = dx * dx + dy * dy + 1e-6
    t = np.clip(((xx - x0) * dx + (yy - y0) * dy) / L2, 0, 1)
    d = np.hypot(xx - (x0 + t * dx), yy - (y0 + t * dy))
    if wmid is None:
        w = w0 + (w1 - w0) * t
    else:
        w = np.where(t < 0.5, w0 + (wmid - w0) * t * 2, wmid + (w1 - wmid) * (t - 0.5) * 2)
    half = np.maximum(w * 0.5, 0.05)
    cover = np.clip(half + 0.5 - d, 0, 1) if True else None
    cover = cover * np.minimum(1.0, (w + 0.2))  # trazos sub-pixel: menos cobertura en vez de más ancho
    al = cover * a_fn(t)
    if tex is not None:
        al = al * tex(t, d / np.maximum(half, 0.3))
    cv.over(by0, bx0, al.astype(np.float32), color)


def blur(img, sigma, wrap=True):
    """Gaussian separable (FFT si wrap)."""
    if sigma <= 0:
        return img
    H, W = img.shape[:2]
    fy = np.fft.fftfreq(H)[:, None]
    fx = np.fft.fftfreq(W)[None, :]
    g = np.exp(-2 * (math.pi ** 2) * sigma * sigma * (fx * fx + fy * fy))
    if img.ndim == 3:
        return np.stack([np.real(np.fft.ifft2(np.fft.fft2(img[..., k]) * g)) for k in range(img.shape[2])], -1).astype(np.float32)
    return np.real(np.fft.ifft2(np.fft.fft2(img) * g)).astype(np.float32)


def periodic_noise(h, w, beta, seed, lo=2.0):
    """Ruido fBm periódico (tilea en X e Y) normalizado a [0,1]."""
    r = np.random.default_rng(seed)
    wn = r.standard_normal((h, w))
    fy = np.fft.fftfreq(h)[:, None] * h
    fx = np.fft.fftfreq(w)[None, :] * w
    f = np.sqrt(fx * fx + fy * fy)
    amp = np.where(f < 1e-6, 0, 1 / np.maximum(f, lo) ** beta)
    n = np.real(np.fft.ifft2(np.fft.fft2(wn) * amp))
    n = (n - n.min()) / (n.max() - n.min())
    return n.astype(np.float32)


def sample_wrap(img, yy, xx):
    H, W = img.shape
    x0 = np.floor(xx).astype(int); y0 = np.floor(yy).astype(int)
    fx = xx - x0; fy = yy - y0
    x0 %= W; y0 %= H
    x1 = (x0 + 1) % W; y1 = (y0 + 1) % H
    return (img[y0, x0] * (1 - fx) * (1 - fy) + img[y0, x1] * fx * (1 - fy) +
            img[y1, x0] * (1 - fx) * fy + img[y1, x1] * fx * fy)


def sheet(frames, cols, rows):
    h, w = frames[0].shape[:2]
    S = np.zeros((rows * h, cols * w, 4), np.uint8)
    for i, f in enumerate(frames):
        r, c = divmod(i, cols)
        S[r * h:(r + 1) * h, c * w:(c + 1) * w] = f
    return S


def bleed(frame, color):
    """Color de los píxeles 100% transparentes = color del efecto (sin halos oscuros al filtrar)."""
    f = frame.copy()
    m = f[..., 3] == 0
    f[m, :3] = color
    return f


def loop_report(name, frames, extra):
    a = [f[..., 3].astype(np.float32) / 255 for f in frames]
    cons = np.mean([np.abs(a[i + 1] - a[i]).mean() for i in range(len(a) - 1)])
    seam = np.abs(a[0] - a[-1]).mean()
    wrap = np.abs(a[0] - extra[..., 3].astype(np.float32) / 255).max()
    return f"{name}: dif. media entre frames consecutivos {cons:.4f} | último->primero {seam:.4f} (ratio {seam / cons:.2f}) | frame N vs frame 0 (máx) {wrap:.4f}"


# ---------------------------------------------------------------- lluvia
def rain(n_frames=60, size=256, ss=4):
    r = np.random.default_rng(7)
    M = 64                       # margen fuera del tile (> largo máx. de la estría)
    Hv = size + M                # columna virtual
    slant = -0.17                # x por unidad de y (~10°, cae hacia la izquierda como en el key art)
    layers = [  # (cantidad, vueltas por loop, largo, ancho, alpha, color)
        (150, 2, (12, 20), 0.45, (0.16, 0.24), (118, 136, 166)),
        (85, 3, (22, 34), 0.70, (0.24, 0.34), (150, 166, 194)),
        (26, 4, (36, 54), 1.00, (0.30, 0.42), (184, 198, 220)),
    ]
    drops = []
    for n, k, (l0, l1), w, (a0, a1), col in layers:
        for _ in range(n):
            drops.append(dict(x=r.uniform(0, size), y=r.uniform(0, Hv), v=k * Hv / n_frames, L=r.uniform(l0, l1),
                              w=w * r.uniform(0.85, 1.15), a=r.uniform(a0, a1), col=np.array(col) / 255.0))
    S = size * ss
    yfade = None

    def frame(f):
        cv = Canvas(S, S)
        for d in drops:
            tail = (d["y"] + d["v"] * f) % Hv - M
            head = tail + d["L"]
            xt = d["x"] + slant * tail
            xh = d["x"] + slant * head
            for off in (-size, 0, size):
                p0 = ((xt + off) * ss, tail * ss)
                p1 = ((xh + off) * ss, head * ss)
                if max(p0[0], p1[0]) < -4 or min(p0[0], p1[0]) > S + 4:
                    continue
                a = d["a"]
                stroke(cv, p0, p1, d["w"] * ss * 0.6, d["w"] * ss, lambda t, a=a: a * (t ** 1.6) * smoothstep(1.0, 0.9, t) + a * 0.0,
                       d["col"])
        out = cv.rgba(ss, fill=(150, 166, 194)).astype(np.float32)
        yy = np.arange(size)[:, None] + 0.5
        fade = smoothstep(0, 22, yy) * smoothstep(size, size - 22, yy)
        out[..., 3] *= fade
        return np.round(out).astype(np.uint8)

    frames = [frame(f) for f in range(n_frames)]
    return frames, frame(n_frames)


# ---------------------------------------------------------------- vapor
def vapor(n_frames=90, W=128, H=256):
    period = 3
    n_cycle = n_frames // period         # 30 bocanadas por loop
    r = np.random.default_rng(11)
    P = [dict(life=r.uniform(70, 96), dx=r.uniform(-10, 10), sway=r.uniform(4, 11), ph=r.uniform(0, 6.28),
              rise=r.uniform(205, 240), r0=r.uniform(3.5, 5.5), r1=r.uniform(22, 32), a=r.uniform(0.09, 0.15),
              subs=[(r.uniform(-0.8, 0.8), r.uniform(-0.6, 0.6), r.uniform(0.45, 0.8), r.uniform(0.5, 1.0)) for _ in range(4)])
         for _ in range(n_cycle)]
    n1 = periodic_noise(H, W, 1.6, 3, lo=2.0)
    n2 = periodic_noise(H, W, 1.9, 5, lo=1.5)
    n3 = periodic_noise(H, W, 1.3, 8, lo=3.0)
    yy, xx = np.mgrid[0:H, 0:W].astype(np.float32) + 0.5
    base_y, base_x = 250.0, 64.0
    shadow = np.array([176, 190, 214], np.float32) / 255
    light = np.array([236, 243, 252], np.float32) / 255

    def frame(f):
        dens = np.zeros((H, W), np.float32)
        for j in range(-40, n_cycle + 1):
            p = P[j % n_cycle]
            age = f - j * period
            if age < 0 or age >= p["life"]:
                continue
            u = age / p["life"]
            cy = base_y - p["rise"] * (u ** 0.78)
            sway = p["sway"] * u * math.sin(p["ph"] + u * 4.0)
            cx = base_x + p["dx"] * u * u + sway
            rad = p["r0"] + (p["r1"] - p["r0"]) * (u ** 0.7)
            alpha = p["a"] * smoothstep(0.0, 0.08, u) * (1 - smoothstep(0.35, 1.0, u)) ** 1.3
            for sx, sy, sr, sa in p["subs"]:
                rr = rad * sr
                g = np.exp(-(((xx - cx - sx * rad) ** 2) / (2 * (rr * 0.9) ** 2) + ((yy - cy - sy * rad) ** 2) / (2 * rr ** 2)))
                dens += alpha * sa * g
        # volutas: domain warp con ruido periódico que sube; una vuelta exacta por loop
        s = f / n_frames
        wx = (sample_wrap(n2, yy + s * H, xx) - 0.5) * 26
        wy = (sample_wrap(n2, yy + s * H + 97, xx + 41) - 0.5) * 18
        ero = sample_wrap(n1, yy + wy + s * H * 2, xx + wx)
        fine = sample_wrap(n3, yy + wy * 0.5 + s * H * 3, xx + wx * 0.5)
        height = np.clip((base_y - yy) / 230, 0, 1)
        e = 0.6 + 0.4 * height                   # más erosión (disipación) arriba
        mod = (1 - e) + e * smoothstep(0.38, 0.8, ero * 0.75 + fine * 0.25) * 1.8
        d = dens * mod
        a = 1 - np.exp(-1.5 * d)
        a *= smoothstep(0, 10, xx) * smoothstep(W, W - 10, xx) * smoothstep(0, 18, yy) * smoothstep(H, H - 4, yy)
        a = np.clip(a * 0.9, 0, 0.7)
        lit = np.clip(0.5 + 0.8 * (fine - 0.5) + 0.2 * height + 0.3 * (1 - np.exp(-2 * d)), 0, 1)
        rgb = shadow[None, None] * (1 - lit[..., None]) + light[None, None] * lit[..., None]
        out = np.dstack([rgb * 255, a * 255])
        return np.round(np.clip(out, 0, 255)).astype(np.uint8)

    frames = [frame(f) for f in range(n_frames)]
    return frames, frame(n_frames)


# ---------------------------------------------------------------- claquetazo
def clap(n_frames=10, W=256, H=160, ss=4):
    cx, cy = 104.0, 88.0                # punto de impacto (2x del original 52,44)
    warm = np.array([255, 243, 222]) / 255
    hot = np.array([255, 252, 244]) / 255
    amber = np.array([255, 214, 160]) / 255
    cyan = np.array([140, 220, 245]) / 255
    r = np.random.default_rng(21)
    # rayos: ángulo (grados, 0 = derecha, y hacia abajo), largo, ancho, demora
    rays = [(0, 140, 10.0, 0.0), (-15, 100, 7.0, 0.04), (14, 96, 7.0, 0.05), (-40, 72, 6.0, 0.08), (38, 64, 6.0, 0.1),
            (-75, 70, 6.0, 0.06), (75, 54, 5.0, 0.12), (-115, 66, 5.5, 0.08), (118, 50, 4.5, 0.14),
            (-150, 64, 5.5, 0.1), (180, 74, 6.0, 0.06), (150, 52, 4.5, 0.16)]
    sparks = [(r.uniform(-160, 10) if i % 3 else r.uniform(-25, 25), r.uniform(90, 150), r.uniform(1.8, 2.8)) for i in range(9)]
    brush = periodic_noise(64, 64, 1.2, 4)

    def tex_for(i):
        row = brush[(i * 7) % 64]
        def tex(t, dn):
            idx = np.clip((t * 63).astype(int), 0, 63)
            return 0.75 + 0.25 * row[idx] * 2 * (1 - 0.4 * np.clip(dn, 0, 1))
        return tex

    yy, xx = np.mgrid[0:H * ss, 0:W * ss].astype(np.float32) / ss + 0.5 / ss

    def frame(f):
        cv = Canvas(H * ss, W * ss)
        t = f / (n_frames - 1)
        if f == n_frames - 1:
            return np.zeros((H, W, 4), np.uint8)
        # halo y anillo de choque (elipse levemente aplastada)
        dist = np.hypot(xx - cx, (yy - cy) * 1.25)
        ring_r = 16 + 70 * (1 - (1 - t) ** 2.2)
        ring_w = 2.2 + 5 * t
        ring_a = 0.38 * (1 - smoothstep(0.1, 0.75, t)) * smoothstep(0.0, 0.1, t + 0.02)
        ring = ring_a * np.exp(-((dist - ring_r) ** 2) / (2 * ring_w ** 2)) * (0.65 + 0.35 * np.cos(np.arctan2(yy - cy, xx - cx)))
        cv.over(0, 0, ring.astype(np.float32), amber)
        halo_a = [0.45, 0.5, 0.3, 0.16, 0.08, 0.04, 0.02, 0.0, 0.0][f]
        halo = halo_a * np.exp(-(dist ** 2) / (2 * (18 + 14 * t) ** 2))
        cv.over(0, 0, halo.astype(np.float32), amber)
        # arco cian (estela de la claqueta, guiño al key art)
        if f <= 5:
            ca = [0.35, 0.5, 0.38, 0.22, 0.1, 0.03][f]
            R = 46 + 6 * f
            ang = np.arctan2(yy - cy, xx - cx)
            span = smoothstep(-3.1, -2.2, ang) * smoothstep(-0.9, -1.6, ang) * smoothstep(-3.1, -1.2, ang)  # arriba-izquierda -> arriba
            arc = ca * span * np.exp(-((np.hypot(xx - cx, yy - cy) - R) ** 2) / (2 * (1.6 + 0.5 * f) ** 2))
            cv.over(0, 0, arc.astype(np.float32), cyan)
        # rayos de impacto pintados: se abren hacia afuera y se afinan
        for i, (deg, ln, wd, delay) in enumerate(rays):
            tt = np.clip((t - delay) / (0.75 - delay), 0, 1)
            if tt <= 0 or tt >= 1:
                continue
            th = math.radians(deg)
            ro = 10 + ln * (1 - (1 - tt) ** 2.4)
            ri = 8 + ln * (tt ** 1.5) * 0.92
            if ro - ri < 2:
                continue
            a = (1 - tt) ** 0.9 * 0.95
            wv = wd * (1 - 0.6 * tt)
            p0 = ((cx + math.cos(th) * ri) * ss, (cy + math.sin(th) * ri) * ss)
            p1 = ((cx + math.cos(th) * ro) * ss, (cy + math.sin(th) * ro) * ss)
            stroke(cv, p0, p1, 1.0 * ss, 0.6 * ss, lambda q, a=a: 0.3 * a * smoothstep(0, 0.3, q) * (1 - 0.5 * q),
                   amber, wmid=wv * 2.6 * ss)
            stroke(cv, p0, p1, 0.3 * ss, 0.2 * ss, lambda q, a=a: a * smoothstep(0, 0.25, q) * (1 - 0.35 * q),
                   warm, wmid=wv * ss, tex=tex_for(i))
        # chispas suaves con gravedad
        for i, (deg, sp, sz) in enumerate(sparks):
            if f < 1 or f > 7:
                continue
            tt = (f - 1) / 7
            th = math.radians(deg)
            px = cx + math.cos(th) * sp * tt
            py = cy + math.sin(th) * sp * tt * 0.8 + 50 * tt * tt
            a = 0.9 * (1 - tt) ** 1.2
            vx, vy = math.cos(th) * sp, math.sin(th) * sp * 0.8 + 100 * tt
            n = math.hypot(vx, vy) + 1e-6
            tail = 12 * (1 - tt) + 3
            stroke(cv, ((px - vx / n * tail) * ss, (py - vy / n * tail) * ss), (px * ss, py * ss), 0.2 * ss, sz * ss,
                   lambda q, a=a: a * q, amber if i % 2 else warm)
        # destello central (blanco cálido) encima de todo
        core_r = [12, 20, 18, 13, 9, 6, 4, 2, 1][f]
        core_a = [1.0, 1.0, 0.85, 0.6, 0.4, 0.22, 0.1, 0.04, 0.0][f]
        core = core_a * np.exp(-(np.hypot(xx - cx, (yy - cy) * 1.1) ** 2) / (2 * core_r ** 2)) ** 0.8
        cv.over(0, 0, np.clip(core * 1.2, 0, 1).astype(np.float32), hot)
        out = cv.rgba(ss, fill=(255, 236, 205)).astype(np.float32)
        # suavizado de "pincel"
        a = blur(out[..., 3] / 255, 0.6)
        out[..., 3] = np.clip(a, 0, 1) * 255
        return np.round(np.clip(out, 0, 255)).astype(np.uint8)

    return [frame(f) for f in range(n_frames)]


def clap_json(W, H, cols, rows, n):
    frames = []
    for i in range(n):
        r_, c_ = divmod(i, cols)
        frames.append({"filename": f"claquetazo-impact {i}", "frame": {"x": c_ * W, "y": r_ * H, "w": W, "h": H},
                       "rotated": False, "trimmed": False, "spriteSourceSize": {"x": 0, "y": 0, "w": W, "h": H},
                       "sourceSize": {"w": W, "h": H}, "duration": 33})
    return {"frames": frames, "meta": {
        "app": "vfx-pintados (numpy+PIL)", "version": "1", "image": "claquetazo_impact.png", "format": "RGBA8888",
        "size": {"w": cols * W, "h": rows * H}, "scale": "1",
        "frameTags": [{"name": "claquetazo-impact", "from": 0, "to": n - 1, "direction": "forward"}],
        "pixelfx": {"id": "tdc/claquetazo-impact", "name": "Claquetazo impact (pintado)", "fps": 30, "loop": False,
                    "frames": n, "frameSize": [W, H], "pivot": [104, 128], "columns": cols}}}


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    report = []
    rf, rx = rain()
    Image.fromarray(sheet([bleed(f, (150, 166, 194)) for f in rf], 8, 8)).save(os.path.join(OUT, "anim_lluvia.png"), optimize=True)
    report.append(loop_report("lluvia", rf, rx))
    vf, vx = vapor()
    Image.fromarray(sheet(vf, 14, 7)).save(os.path.join(OUT, "anim_vapor.png"), optimize=True)
    report.append(loop_report("vapor", vf, vx))
    cf = clap()
    Image.fromarray(sheet([bleed(f, (255, 236, 205)) for f in cf], 3, 4)).save(os.path.join(OUT, "claquetazo_impact.png"), optimize=True)
    with open(os.path.join(OUT, "claquetazo_impact.json"), "w") as fh:
        json.dump(clap_json(256, 160, 3, 4, 10), fh, indent=1)
    print("\n".join(report))
    with open(os.path.join(OUT, "src", "loop_check.txt"), "w") as fh:
        fh.write("\n".join(report) + "\n")
