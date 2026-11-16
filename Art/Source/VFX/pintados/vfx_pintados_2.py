"""VFX pintados (2da tanda) para The Director's Cut: extras, rebobinado, piezas y light_toggle.
Reemplazo directo de los sheets pixelfx: mismos nombres, grilla, frames, fps y loop; celda 2x (misma proporción).
Uso: python3 vfx_pintados_2.py <repo_vfx_json_dir> <out_dir>      (numpy + PIL; reusa helpers de vfx_pintados.py)
"""
import json, math, os, sys
import numpy as np
from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from vfx_pintados import Canvas, stroke, smoothstep, periodic_noise, sample_wrap, sheet, bleed, loop_report  # noqa: E402

SRC = sys.argv[1] if len(sys.argv) > 1 else "."
OUT = sys.argv[2] if len(sys.argv) > 2 else "."
SS = 2          # supersampling
K = 2           # escala de celda respecto del original


def C(*rgb):
    return np.array(rgb, np.float32) / 255


def ctx(W, H):
    cv = Canvas(H * SS, W * SS)
    yy, xx = np.mgrid[0:H * SS, 0:W * SS].astype(np.float32) / SS + 0.5 / SS
    return cv, yy, xx


def put(cv, alpha, col):
    cv.over(0, 0, np.clip(alpha, 0, 1).astype(np.float32), col)


def line(cv, p0, p1, w0, w1, a, col, wmid=None, tex=None, afn=None):
    stroke(cv, (p0[0] * SS, p0[1] * SS), (p1[0] * SS, p1[1] * SS), w0 * SS, w1 * SS,
           afn or (lambda t, a=a: a + 0 * t), col, wmid=None if wmid is None else wmid * SS, tex=tex)


def blob(cv, cx, cy, r, a, col, sx=1.0, sy=1.0):
    if a <= 0.002:
        return
    R = 3 * r * max(sx, sy) + 1
    x0, x1 = int(math.floor((cx - R) * SS)), int(math.ceil((cx + R) * SS))
    y0, y1 = int(math.floor((cy - R) * SS)), int(math.ceil((cy + R) * SS))
    yy, xx = np.mgrid[y0:y1, x0:x1].astype(np.float32) / SS + 0.5 / SS
    g = a * np.exp(-(((xx - cx) / sx) ** 2 + ((yy - cy) / sy) ** 2) / (2 * r * r))
    cv.over(y0, x0, g.astype(np.float32), col)


def sparkle(cv, cx, cy, L, a, col, core=None):
    """Destello de 4 puntas pintado (dos trazos afinados + núcleo suave)."""
    if a <= 0.01:
        return
    w = max(1.0, L * 0.2)
    line(cv, (cx - L, cy), (cx + L, cy), 0.2, 0.2, a, col, wmid=w)
    line(cv, (cx, cy - L), (cx, cy + L), 0.2, 0.2, a, col, wmid=w)
    blob(cv, cx, cy, max(0.8, L * 0.22), a, core if core is not None else col)


def star5(cv, cx, cy, R, a, core, edge, rot):
    """Estrella de 5 puntas con borde antialiasado y degradé radial."""
    B = R + 3
    x0, x1 = int(math.floor((cx - B) * SS)), int(math.ceil((cx + B) * SS))
    y0, y1 = int(math.floor((cy - B) * SS)), int(math.ceil((cy + B) * SS))
    yy, xx = np.mgrid[y0:y1, x0:x1].astype(np.float32) / SS + 0.5 / SS
    th = np.arctan2(yy - cy, xx - cx)
    d = np.hypot(xx - cx, yy - cy)
    rs = R * (0.46 + 0.54 * ((1 + np.cos(5 * (th - rot))) / 2) ** 1.6)
    cover = np.clip((rs - d) / 0.9 + 0.5, 0, 1) * a
    u = np.clip(d / np.maximum(rs, 0.1), 0, 1)[..., None]
    col = core[None, None] * (1 - u) + edge[None, None] * u
    cv.over(y0, x0, cover.astype(np.float32), col)


def rrect_sdf(xx, yy, x0, y0, x1, y1, r):
    cx, cy, hw, hh = (x0 + x1) / 2, (y0 + y1) / 2, (x1 - x0) / 2, (y1 - y0) / 2
    qx, qy = np.abs(xx - cx) - (hw - r), np.abs(yy - cy) - (hh - r)
    return np.hypot(np.maximum(qx, 0), np.maximum(qy, 0)) + np.minimum(np.maximum(qx, qy), 0) - r


def brush_tex(seed):
    n = periodic_noise(64, 64, 1.2, seed)
    row = n[seed % 64]

    def tex(t, dn):
        idx = np.clip((t * 63).astype(int), 0, 63)
        return np.clip(0.7 + 0.5 * row[idx] * (1 - 0.5 * np.clip(dn, 0, 1)), 0, 1)
    return tex


def finish(cv, fill):
    return cv.rgba(SS, fill=fill)


EMPTY = lambda W, H: np.zeros((H, W, 4), np.uint8)  # noqa: E731


# ------------------------------------------------------------------ extra_defeat
def extra_defeat():
    W, H, N = 112 * K, 108 * K, 46
    cx0, cy0 = 56.0 * K, 58.0 * K              # centro del iris / puff (original 56,58)
    r = np.random.default_rng(21)
    puffs = [dict(ox=r.uniform(-30, 30), oy=r.uniform(-20, 14), r0=r.uniform(7, 11), r1=r.uniform(18, 27),
                  t0=5 + r.uniform(0, 4), rise=r.uniform(6, 18), life=r.uniform(20, 32), a=r.uniform(0.55, 0.85),
                  subs=[(r.uniform(-.6, .6), r.uniform(-.5, .5), r.uniform(.5, .85)) for _ in range(3)]) for _ in range(15)]
    glints = [(cx0 + r.uniform(-70, 70), cy0 + r.uniform(-72, -8), r.uniform(4.5, 8), int(r.integers(9, 15)), r.uniform(0, 6.28))
              for _ in range(6)]
    noise = periodic_noise(H * SS, W * SS, 1.6, 4)
    fine = periodic_noise(H * SS, W * SS, 1.2, 6)
    ink, cream, warm = C(11, 10, 16), C(244, 234, 210), C(255, 250, 238)
    dark, light = C(96, 95, 104), C(220, 216, 206)

    def frame(f):
        if f == N - 1:
            return EMPTY(W, H)
        cv, yy, xx = ctx(W, H)
        d = np.hypot(xx - cx0, yy - cy0)
        # iris de cine que se cierra (0.14 s)
        if f <= 5:
            u = min(f / 4, 1.0)
            rad = 100 - 76 * u ** 1.4
            fade = [1, 1, 1, 1, 0.75, 0.25][f]
            put(cv, 0.55 * fade * smoothstep(rad + 1, rad + 9, d) * (1 - smoothstep(rad + 18, rad + 32, d)), ink)
            put(cv, 0.32 * fade * np.exp(-(d - rad) ** 2 / (2 * 5.0 ** 2)), cream)
            put(cv, 0.95 * fade * np.exp(-(d - rad) ** 2 / (2 * 1.3 ** 2)), cream)
        # humo (puf)
        dens = np.zeros_like(xx)
        for p in puffs:
            age = f - p["t0"]
            if age < 0 or age > p["life"]:
                continue
            u = age / p["life"]
            px = cx0 + p["ox"] * (0.55 + 0.45 * u ** 0.5)
            py = cy0 + p["oy"] - p["rise"] * u
            rad = p["r0"] + (p["r1"] - p["r0"]) * u ** 0.6
            a = p["a"] * smoothstep(0, 0.1, u) * (1 - smoothstep(0.4, 1.0, u))
            for sx, sy, sr in p["subs"]:
                rr = rad * sr
                dens += a * np.exp(-((xx - px - sx * rad) ** 2 + (yy - py - sy * rad) ** 2) / (2 * rr * rr))
        if dens.max() > 0.01:
            ero = sample_wrap(noise, yy * SS + f * 2.0 * SS, xx * SS)
            fn = sample_wrap(fine, yy * SS + f * 1.0 * SS, xx * SS + 17)
            e = 0.3 + 0.6 * smoothstep(10, 36, f)
            mod = (1 - e) + e * smoothstep(0.3, 0.75, ero * 0.75 + fn * 0.25) * 1.7
            dd = dens * mod
            alpha = np.clip(1 - np.exp(-2.4 * dd), 0, 0.88)
            sh = np.roll(dens, (4 * SS, 3 * SS), (0, 1))
            lit = np.clip(0.62 + 0.38 * np.tanh(2.2 * (dens - sh)) + 0.3 * (fn - 0.5), 0, 1)[..., None]
            col = dark[None, None] * (1 - lit) + light[None, None] * lit
            cv.over(0, 0, alpha.astype(np.float32), col)
        # destello + anillo + rayos cortos
        if 4 <= f <= 10:
            k = f - 4
            rr = 30 + 11 * k
            put(cv, 0.45 * (1 - k / 7) * np.exp(-(d - rr) ** 2 / (2 * (2 + 0.6 * k) ** 2)), cream)
            if k <= 3:
                for i in range(12):
                    th = i * math.pi / 6 + 0.2
                    ri, ro = 24 + 6 * k, 40 + 14 * k
                    line(cv, (cx0 + math.cos(th) * ri, cy0 + math.sin(th) * ri), (cx0 + math.cos(th) * ro, cy0 + math.sin(th) * ro),
                         0.3, 0.2, 0.8 * (1 - k / 4), cream, wmid=2.6)
            ca = [1, 1, 0.8, 0.5, 0.28, 0.12, 0.04][k]
            put(cv, ca * np.exp(-d ** 2 / (2 * (14 + 3 * k) ** 2)) ** 0.8, warm)
        # brillos crema
        for gx, gy, L, t0, ph in glints:
            k = f - t0
            if 0 <= k < 16:
                a = math.sin(math.pi * k / 16) * (0.7 + 0.3 * math.sin(ph + k * 1.4))
                sparkle(cv, gx, gy, L, 0.9 * a, cream, warm)
        return finish(cv, (200, 198, 192))

    return [frame(f) for f in range(N)]


# ------------------------------------------------------------------ extra_stun (loop)
def extra_stun():
    W, H, N = 64 * K, 64 * K, 60
    cx, cy, rx, ry = 32.0 * K, 21.0 * K, 15.5 * K, 4.2 * K
    core, edge, glow = C(255, 222, 176), C(214, 138, 78), C(226, 150, 92)
    trail_c = C(214, 160, 110)
    # destellos: período 20 o 30 frames (divide 60 -> loop exacto)
    r = np.random.default_rng(2)
    twinkles = [(per, ph, [(cx + r.uniform(-24, 24), cy + r.uniform(-20, -10)) for _ in range(N // per)])
                for per, ph in ((20, 0), (30, 9), (20, 11))]

    def frame(f):
        cv, yy, xx = ctx(W, H)
        base = 2 * math.pi * f / N
        # estela de órbita: más fuerte justo detrás de cada estrella
        ang = np.arctan2(-(yy - cy) / ry, (xx - cx) / rx)
        q = np.sqrt(((xx - cx) / rx) ** 2 + ((yy - cy) / ry) ** 2)
        rl = np.hypot(rx * np.cos(ang), ry * np.sin(ang))
        band = np.exp(-((q - 1) * rl) ** 2 / (2 * 1.1 ** 2))
        tr = np.zeros_like(xx)
        for i in range(5):
            phi = -(base + 2 * math.pi * i / 5)            # gira en sentido horario visto de frente
            dphi = np.mod(ang - phi, 2 * math.pi)          # detrás de la estrella
            tr += np.exp(-dphi / 0.55)
        depth = 0.55 + 0.45 * np.clip(-np.sin(ang), -1, 1) * 0 + 0.45 * ((yy - cy) / ry > 0)
        put(cv, 0.42 * band * np.clip(tr, 0, 1) * depth, trail_c)
        stars = []
        for i in range(5):
            phi = -(base + 2 * math.pi * i / 5)
            sx, sy = cx + rx * math.cos(phi), cy - ry * math.sin(phi)
            dz = (-math.sin(phi) + 1) / 2              # 1 = adelante (abajo)
            stars.append((dz, sx, sy, i))
        for dz, sx, sy, i in sorted(stars):
            s = 0.78 + 0.32 * dz
            a = 0.6 + 0.4 * dz
            blob(cv, sx, sy, 6.0 * s, 0.32 * a, glow)
            star5(cv, sx, sy, 6.4 * s, a, core * (0.85 + 0.15 * dz) + 0 * core, edge * (0.8 + 0.2 * dz), base * 2 + i)
        for per, ph, pos in twinkles:
            k = (f + ph) % per
            px, py = pos[((f + ph) // per) % len(pos)]
            a = math.sin(math.pi * k / per) ** 2
            sparkle(cv, px, py, 4.2, 0.85 * a, C(255, 214, 160), C(255, 240, 215))
        return finish(cv, (226, 150, 92))

    return [frame(f) for f in range(N)], frame(N)


# ------------------------------------------------------------------ scripted_bounce
def scripted_bounce():
    W, H, N = 128 * K, 112 * K, 25
    P = (64.0 * K, 66.0 * K)
    floor = 96.0 * K
    r = np.random.default_rng(11)
    sparks = []
    for i in range(22):
        th = math.radians(r.uniform(-172, -8))
        sp = r.uniform(5, 10.5)
        vx, vy = math.cos(th) * sp, math.sin(th) * sp
        x, y = P
        path = [(x, y)]
        for _ in range(N):
            for _ in range(4):
                vy += 0.95 / 4
                x += vx / 4
                y += vy / 4
                if y > floor:
                    y = floor - (y - floor) * 0.45
                    vy *= -0.45
                    vx *= 0.7
            path.append((x, y))
        sparks.append((path, r.uniform(1.4, 2.4), int(r.integers(14, 20)), i % 2))
    noise = periodic_noise(H * SS, W * SS, 1.2, 9)
    white, cream, gray, ink = C(255, 252, 246), C(238, 232, 218), C(176, 174, 172), C(16, 14, 22)
    tex = brush_tex(5)

    def frame(f):
        if f == N - 1:
            return EMPTY(W, H)
        cv, yy, xx = ctx(W, H)
        d = np.hypot(xx - P[0], yy - P[1])
        # anillo que se rompe
        if 2 <= f <= 8:
            k = f - 2
            rr = 24 + 6 * k
            th = np.arctan2(yy - P[1], xx - P[0])
            brk = smoothstep(0.3, 0.6, 0.5 + 0.5 * np.cos(6 * th + 1.3 + 0.25 * k) + 0.25 * (k / 6) * 0)
            brk = brk * smoothstep(-0.1 + 0.12 * k, 0.15 + 0.12 * k, sample_wrap(noise, th * 40 + 300, d * 0 + 7))
            put(cv, 0.8 * (1 - k / 7) * brk * np.exp(-(d - rr) ** 2 / (2 * (1.6 + 0.4 * k) ** 2)), cream)
        # X de guion estampada (tinta con borde crema), se disuelve al final
        if 2 <= f <= 16:
            k = f - 2
            sc = [1.35, 1.12, 1.0][k] if k < 3 else 1.0
            cx = Canvas(H * SS, W * SS)
            blob(cv, P[0], P[1], 22, 0.28 * (1 - smoothstep(4, 12, k)), gray)
            L = 19 * sc
            for sgn in (1, -1):
                p0 = (P[0] - L, P[1] - sgn * L)
                p1 = (P[0] + L, P[1] + sgn * L)
                line(cx, p0, p1, 13 * sc, 12 * sc, 0.97, cream, wmid=14.5 * sc, tex=tex)
            for sgn in (1, -1):
                p0 = (P[0] - L + 2.5, P[1] - sgn * (L - 2.5))
                p1 = (P[0] + L - 2.5, P[1] + sgn * (L - 2.5))
                line(cx, p0, p1, 6.5 * sc, 5.5 * sc, 0.96, ink, wmid=8 * sc, tex=tex)
            th_ = smoothstep(9, 14, k)
            mask = smoothstep(th_ - 0.06, th_ + 0.06, noise) if k >= 9 else 1.0
            if k >= 9:
                mask = mask * (1 - smoothstep(13, 14.5, k))
            a = cx.a * mask
            col = cx.c / np.maximum(cx.a[..., None], 1e-4)
            cv.over(0, 0, a.astype(np.float32), col)
        # chispas grises que rebotan
        for path, w, life, alt in sparks:
            k = f - 1
            if k < 0 or k >= life:
                continue
            (x0, y0), (x1, y1) = path[k], path[k + 1]
            a = 1.0 * (1 - k / life) ** 0.9
            tail = 1.0
            line(cv, (x1 - (x1 - x0) * tail, y1 - (y1 - y0) * tail), (x1, y1), 0.3, w * 1.4, a, cream if alt else gray)
        # destello de 4 puntas
        if f <= 4:
            a = [1, 0.85, 0.55, 0.3, 0.12][f]
            L = [56, 64, 58, 48, 36][f]
            blob(cv, P[0], P[1], 26, 0.45 * a, gray)
            line(cv, (P[0] - L, P[1]), (P[0] + L, P[1]), 0.3, 0.3, a, white, wmid=7 - f)
            line(cv, (P[0], P[1] - L * 0.72), (P[0], P[1] + L * 0.72), 0.3, 0.3, a, white, wmid=6 - f)
            for s1, s2 in ((1, 1), (1, -1), (-1, 1), (-1, -1)):
                line(cv, P, (P[0] + s1 * L * 0.42, P[1] + s2 * L * 0.42), 3.0, 0.2, 0.75 * a, white)
            put(cv, a * np.exp(-d ** 2 / (2 * (9 + 2 * f) ** 2)) ** 0.8, white)
        return finish(cv, (230, 226, 216))

    return [frame(f) for f in range(N)]


# ------------------------------------------------------------------ rewind (loop)
def rewind():
    W, H, N = 96 * K, 72 * K, 54
    cx, cy, rx, ry = 48.0 * K, 38.0 * K, 31.0 * K, 12.5 * K
    cyan, bright, deep, hole = C(90, 222, 244), C(214, 250, 255), C(22, 120, 150), C(4, 26, 36)
    r = np.random.default_rng(2)
    motes = [(r.uniform(0, 1), r.uniform(0, 6.28), r.uniform(0.9, 1.25), r.uniform(0.9, 1.6)) for _ in range(14)]
    SPAN = 2.3                       # largo de cada tira (rad)
    SEG = 70

    def pos(phi):
        return cx + rx * math.cos(phi), cy - ry * math.sin(phi)

    def frame(f):
        cv, yy, xx = ctx(W, H)
        th0 = 2 * math.pi * f / N                 # gira antihorario (rebobina)
        scroll = 3 * 0.17 * f / N                 # perforaciones corren hacia atrás: 3 pasos por loop
        # brillo de piso y halo central
        put(cv, 0.12 * np.exp(-(((xx - cx) / 34) ** 2 + ((yy - 60 * K) / 4) ** 2) / 2), cyan)
        pulse = 0.5 + 0.12 * math.sin(2 * math.pi * 2 * f / N)
        put(cv, 0.35 * pulse * np.exp(-((xx - cx) ** 2 + (yy - cy) ** 2) / (2 * 22.0 ** 2)), deep)
        # motas en espiral hacia el centro (tiempo invertido)
        for u0, a0, sr, sz in motes:
            u = (u0 + 2 * f / N) % 1.0
            rad = sr * (1 - u) + 0.08
            phi = a0 + th0 + u * 2.4
            mx, my = cx + rx * rad * math.cos(phi), cy - ry * rad * math.sin(phi)
            a = 0.85 * smoothstep(0, 0.2, u) * (1 - smoothstep(0.75, 1.0, u))
            blob(cv, mx, my, sz, a, cyan)
            blob(cv, mx, my, sz * 0.45, a, bright)

        def ribbon(back):
            for k in range(2):
                start = th0 + k * math.pi
                for j in range(SEG):
                    t0, t1 = j / SEG, (j + 1) / SEG
                    p0, p1 = start + SPAN * t0, start + SPAN * t1
                    sm = (p0 + p1) / 2
                    is_back = math.sin(sm) > 0
                    if is_back != back:
                        continue
                    dz = 0.62 if back else 1.0
                    taper = smoothstep(0, 0.35, t0)          # cola que se desvanece
                    a = taper * dz
                    w = (9.0 + 3 * (1 - math.sin(sm)) / 2) * dz ** 0.5
                    A, B = pos(p0), pos(p1)
                    line(cv, A, B, w, w, 0.38 * a, cyan)
                    # rieles
                    nx, ny = -(B[1] - A[1]), (B[0] - A[0])
                    nl = math.hypot(nx, ny) + 1e-6
                    nx, ny = nx / nl, ny / nl
                    for sgn in (1, -1):
                        o = sgn * w * 0.42
                        line(cv, (A[0] + nx * o, A[1] + ny * o), (B[0] + nx * o, B[1] + ny * o), 1.1, 1.1, 0.85 * a, bright)
                    # perforaciones (sprockets) que corren hacia atrás
                    sp = 0.17
                    for m in range(int(SPAN / sp) + 2):
                        hp = start + (scroll % sp) + m * sp
                        if p0 <= hp < p1 and hp > start + 0.1:
                            hx, hy = pos(hp)
                            for sgn in (1, -1):
                                o = sgn * w * 0.2
                                blob(cv, hx + nx * o, hy + ny * o, 0.9 * dz, 0.75 * a, hole, sx=1.3, sy=1.0)
                # punta de flecha al frente (sentido antihorario)
                tip = start + SPAN + 0.05
                is_back = math.sin(tip) > 0
                if is_back == back:
                    dz = 0.62 if back else 1.0
                    T = pos(tip)
                    tx, ty = -rx * math.sin(tip), -ry * math.cos(tip)
                    tl = math.hypot(tx, ty) + 1e-6
                    tx, ty = tx / tl, ty / tl
                    nx, ny = -ty, tx
                    hl = 11 * dz
                    for sgn in (1, -1):
                        line(cv, (T[0] - tx * hl + nx * sgn * hl * 0.8, T[1] - ty * hl + ny * sgn * hl * 0.8),
                             (T[0] + tx * 2, T[1] + ty * 2), 1.0, 3.2 * dz, 0.95 * dz, bright)
                    blob(cv, T[0], T[1], 4.5, 0.35 * dz, cyan)

        ribbon(back=True)
        # núcleo
        put(cv, 0.55 * pulse * np.exp(-((xx - cx) ** 2 + (yy - cy) ** 2) / (2 * 8.0 ** 2)), cyan)
        put(cv, 0.8 * pulse * np.exp(-((xx - cx) ** 2 + (yy - cy) ** 2) / (2 * 3.2 ** 2)), bright)
        ribbon(back=False)
        return finish(cv, (90, 222, 244))

    return [frame(f) for f in range(N)], frame(N)


# ------------------------------------------------------------------ piezas (136x60 -> 272x120, 11 frames, 25 fps)
PW, PH, PN = 136 * K, 60 * K, 11
PX0, PX1, PY0, PY1 = 12.0 * K, 124.0 * K, 35.0 * K, 45.0 * K
LY_HOT, LY_WHITE, LY_YEL, LY_AMB = C(255, 252, 236), C(255, 244, 205), C(255, 214, 96), C(232, 160, 52)
DK_INK, DK_DEEP, DK_VIO, DK_LIL = C(22, 12, 40), C(70, 36, 120), C(150, 96, 226), C(214, 182, 255)


def ease(t):
    t = min(max(t, 0.0), 1.0)
    return t * t * (3 - 2 * t)


def light_piece_appear():
    r = np.random.default_rng(3)
    dust = [(r.uniform(PX0 + 4, PX1 - 4), r.uniform(PY0, PY1), r.uniform(1.0, 2.6), r.uniform(0.8, 1.6), r.uniform(-0.4, 0.4))
            for _ in range(46)]

    def edge_at(f):
        return PX0 + (PX1 - PX0) * ease(f / 6)

    def tspawn(x):  # frame en que el filo pasa por x
        u = (x - PX0) / (PX1 - PX0)
        lo, hi = 0.0, 6.0
        for _ in range(20):
            m = (lo + hi) / 2
            if ease(m / 6) < u:
                lo = m
            else:
                hi = m
        return lo

    def frame(f):
        if f == PN - 1:
            return EMPTY(PW, PH)
        cv, yy, xx = ctx(PW, PH)
        e = edge_at(f)
        fade = [1, 1, 1, 1, 1, 1, 1, 0.9, 0.6, 0.28][f]
        sdf = rrect_sdf(xx, yy, PX0, PY0, PX1, PY1, 2.5)
        inside = np.clip(0.5 - sdf, 0, 1)
        reveal = np.clip((e - xx) / 3 + 0.5, 0, 1)
        heat = np.exp(-np.maximum(e - xx, 0) / 26) if f <= 6 else np.exp(-np.maximum(e - xx, 0) / 26) * (1 - (f - 6) / 4)
        # relleno + borde
        put(cv, fade * reveal * inside * (0.26 + 0.5 * heat), LY_YEL)
        put(cv, fade * reveal * inside * 0.7 * heat ** 2, LY_HOT)
        put(cv, fade * reveal * 0.95 * np.exp(-sdf ** 2 / (2 * 0.9 ** 2)), LY_YEL)
        put(cv, fade * reveal * 0.3 * np.exp(-np.maximum(sdf, 0) ** 2 / (2 * 5.0 ** 2)) * (1 - inside), LY_AMB)
        # polvo de luz que sube
        for x, y, sp, sz, dr in dust:
            age = f - tspawn(x)
            if age < 0:
                continue
            a = 0.85 * math.exp(-age / 3.5) * smoothstep(0, 0.6, age)
            blob(cv, x + dr * age, y - sp * age * K, sz, a, LY_WHITE)
        # filo de luz (destello vertical + halo + raya horizontal)
        if f <= 7:
            fa = 1.0 if f < 7 else 0.6
            blob(cv, e, (PY0 + PY1) / 2, 16, 0.38 * fa, LY_AMB)
            blob(cv, e, (PY0 + PY1) / 2, 2.2, 0.95 * fa, LY_HOT, sx=1.0, sy=7.5)
            line(cv, (e - 30, (PY0 + PY1) / 2), (e + 18, (PY0 + PY1) / 2), 0.2, 0.2, 0.55 * fa, LY_WHITE, wmid=2.0)
        if f == 0:
            sparkle(cv, PX0, (PY0 + PY1) / 2, 14, 0.95, LY_WHITE, LY_HOT)
        if f in (6, 7, 8):
            sparkle(cv, PX1, (PY0 + PY1) / 2, [16, 13, 7][f - 6], [1, 0.8, 0.4][f - 6], LY_WHITE, LY_HOT)
        return finish(cv, (255, 214, 96))

    return [frame(f) for f in range(PN)]


def light_piece_vanish():
    r = np.random.default_rng(4)
    mid = (PX0 + PX1) / 2
    jit = periodic_noise(PH * SS, PW * SS, 1.4, 12)

    def tbreak(x):
        return 2.0 + 4.2 * (1 - np.abs(x - mid) / (mid - PX0))

    sparks = []
    for _ in range(150):
        x = r.uniform(PX0 + 2, PX1 - 2)
        y = r.uniform(PY0 + 2, PY1 - 2)
        sparks.append((x, y, float(tbreak(x)) + r.uniform(-0.6, 0.6), r.uniform(2.0, 4.5), r.uniform(-0.8, 0.8),
                       r.uniform(0.9, 1.7), r.uniform(4, 7), r.uniform(0, 6.28)))

    def frame(f):
        if f == PN - 1:
            return EMPTY(PW, PH)
        cv, yy, xx = ctx(PW, PH)
        sdf = rrect_sdf(xx, yy, PX0, PY0, PX1, PY1, 2.5)
        inside = np.clip(0.5 - sdf, 0, 1)
        tb = tbreak(xx) + (sample_wrap(jit, yy * SS, xx * SS) - 0.5) * 2.0
        alive = 1 - smoothstep(tb - 0.6, tb + 0.4, f)
        hot = np.exp(-np.abs(f - tb) / 0.9) * alive
        if f <= 1:
            put(cv, [0.55, 0.3][f] * np.exp(-(((xx - mid) / 60) ** 2 + ((yy - (PY0 + PY1) / 2) / 22) ** 2) / 2), LY_AMB)
        body = [0.95, 0.75, 0.6, 0.55, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5][f]
        put(cv, inside * alive * body, LY_YEL)
        put(cv, inside * alive * (0.85 if f == 0 else 0.25 if f == 1 else 0) , LY_WHITE)
        put(cv, inside * np.clip(hot, 0, 1) * 0.55, LY_HOT)
        put(cv, alive * 0.9 * np.exp(-sdf ** 2 / (2 * 0.9 ** 2)), LY_YEL)
        for x, y, t0, sp, dr, sz, life, ph in sparks:
            age = f - t0
            if age < 0 or age > life:
                continue
            u = age / life
            a = 0.95 * (1 - u) ** 1.2 * (0.65 + 0.35 * math.sin(ph + age * 2.6))
            px = x + dr * age * K
            py = y - sp * K * (age - 0.12 * age * age)
            col = LY_HOT * (1 - u) + LY_YEL * u if u < 0.5 else LY_YEL * (1.5 - u) + LY_AMB * (u - 0.5)
            blob(cv, px, py, sz * (1 - 0.35 * u), a, col, sx=1.0, sy=1.25)
        return finish(cv, (255, 214, 96))

    return [frame(f) for f in range(PN)]


def dark_piece_appear():
    r = np.random.default_rng(5)
    wav = periodic_noise(PH * SS, PW * SS, 1.5, 21)
    cy = (PY0 + PY1) / 2
    clouds = [(r.uniform(-6, 10), r.uniform(-12, 8), r.uniform(5, 10), r.uniform(0.18, 0.35)) for _ in range(5)]
    smoke = [(r.uniform(PX0 + 6, PX1 - 6), r.uniform(1.5, 3.2), r.uniform(6, 10), r.uniform(0.12, 0.22)) for _ in range(12)]

    def front_at(f):
        return PX0 - 4 + (PX1 - PX0 + 10) * ease((f - 0.5) / 5.5)

    def frame(f):
        if f == PN - 1:
            return EMPTY(PW, PH)
        cv, yy, xx = ctx(PW, PH)
        fade = [1, 1, 1, 1, 1, 1, 1, 0.9, 0.62, 0.3][f]
        e = front_at(f)
        sdf = rrect_sdf(xx, yy, PX0, PY0, PX1, PY1, 2.5)
        inside = np.clip(0.5 - sdf, 0, 1)
        wv = (sample_wrap(wav, yy * SS * 1.5, xx * 0 + f * 9.0) - 0.5) * 14
        fill = np.clip((e + wv - xx) / 2.5 + 0.5, 0, 1)
        rim = np.clip((e + 22 - xx) / 3 + 0.5, 0, 1)            # el contorno corre adelante
        # humo violeta que sube detrás del frente
        for sx, sp, sz, a in smoke:
            t0 = (sx - PX0) / (PX1 - PX0) * 5.5 + 0.5
            age = f - t0
            if age < 0:
                continue
            blob(cv, sx + 2 * math.sin(age + sx), cy - 4 - sp * age * K, sz + age * 1.6, fade * a * math.exp(-age / 3.5), DK_VIO)
        put(cv, fade * inside * fill * 0.82, DK_INK)
        put(cv, fade * inside * fill * 0.35 * np.exp(-np.maximum(e + wv - xx, 0) / 30), DK_DEEP)
        put(cv, fade * rim * 0.95 * np.exp(-sdf ** 2 / (2 * 0.9 ** 2)), DK_LIL)
        put(cv, fade * rim * 0.35 * np.exp(-np.maximum(sdf, 0) ** 2 / (2 * 4.0 ** 2)) * (1 - inside), DK_VIO)
        if f <= 7:
            # menisco y nube de tinta en el frente
            men = np.exp(-((xx - (e + wv)) ** 2) / (2 * 1.6 ** 2)) * np.clip(0.5 - sdf + 2, 0, 1)
            put(cv, 0.85 * men * (1 if f < 7 else 0.5), DK_VIO)
            for ox, oy, sz, a in clouds:
                blob(cv, e + ox, cy + oy * 0.8, sz, a * (1 if f < 7 else 0.4), DK_DEEP)
        if f <= 2:
            # gota de tinta que salpica en la punta izquierda
            rr = [3.5, 7, 11][f]
            d = np.hypot(xx - PX0, yy - cy)
            put(cv, [0.95, 0.7, 0.35][f] * np.exp(-(d - rr) ** 2 / (2 * 1.2 ** 2)), DK_LIL)
            blob(cv, PX0, cy, [4.0, 3.0, 2.0][f], [0.95, 0.8, 0.4][f], DK_INK)
        return finish(cv, (150, 96, 226))

    return [frame(f) for f in range(PN)]


def dark_piece_vanish():
    r = np.random.default_rng(6)
    mid, cy = (PX0 + PX1) / 2, (PY0 + PY1) / 2
    brk = periodic_noise(PH * SS, PW * SS, 1.3, 31)
    ero = periodic_noise(PH * SS, PW * SS, 1.7, 33)
    puffs = [(r.uniform(PX0 + 4, PX1 - 4), r.uniform(PY0, PY1), r.uniform(2.2, 2.6), r.uniform(1.0, 3.2), r.uniform(5, 9),
              r.uniform(0.35, 0.6), r.uniform(-0.6, 0.6)) for _ in range(34)]
    motes = [(r.uniform(PX0, PX1), r.uniform(PY0, PY1), r.uniform(2.5, 4.5), r.uniform(1.0, 1.6), r.uniform(2.5, 4.5), r.uniform(0, 6.3))
             for _ in range(30)]
    drops = [(r.uniform(PX0 + 10, PX1 - 10), r.uniform(3.2, 4.2), r.uniform(1.6, 2.2)) for _ in range(4)]

    def frame(f):
        if f == PN - 1:
            return EMPTY(PW, PH)
        cv, yy, xx = ctx(PW, PH)
        sdf = rrect_sdf(xx, yy, PX0, PY0, PX1, PY1, 2.5)
        inside = np.clip(0.5 - sdf, 0, 1)
        nb = sample_wrap(brk, yy * SS, xx * SS)
        th = smoothstep(1.5, 5.5, f)                               # la pieza se rompe en manchas
        alive = smoothstep(th - 0.08, th + 0.08, nb) if f >= 2 else np.ones_like(xx)
        if f >= 6:
            alive = alive * 0
        if f <= 1:
            put(cv, [0.45, 0.6][f] * np.exp(-(((xx - mid) / 40) ** 2 + ((yy - cy) / 18) ** 2) / 2), DK_VIO)
        put(cv, inside * alive * [0.75, 0.88, 0.9, 0.9, 0.9, 0.9][min(f, 5)], DK_INK)
        put(cv, inside * alive * 0.3 * np.exp(-((xx - mid) ** 2) / (2 * 30.0 ** 2)), DK_DEEP)
        put(cv, alive * [1.0, 0.95, 0.8, 0.7, 0.6, 0.5][min(f, 5)] * np.exp(-sdf ** 2 / (2 * 0.9 ** 2)), DK_LIL)
        # humo violeta (densidad erosionada por ruido)
        if f >= 2:
            dens = np.zeros_like(xx)
            for x, y, t0, sp, sz, a, dr in puffs:
                age = f - t0
                if age < 0:
                    continue
                px, py = x + dr * age * K, y - sp * age * K
                rad = sz + age * 1.8
                aa = a * smoothstep(0, 0.8, age) * math.exp(-age / 3.2)
                dens += aa * np.exp(-((xx - px) ** 2 + ((yy - py) * 1.15) ** 2) / (2 * rad * rad))
            ev = sample_wrap(ero, yy * SS + f * 6 * SS, xx * SS)
            mod = 0.4 + 0.6 * smoothstep(0.3, 0.75, ev) * 1.6
            al = np.clip(1 - np.exp(-2.0 * dens * mod), 0, 0.8)
            lit = np.clip(0.4 + 0.6 * (ev - 0.4), 0, 1)[..., None]
            col = DK_DEEP[None, None] * (1 - lit) + DK_VIO[None, None] * lit
            cv.over(0, 0, al.astype(np.float32), col)
        for x, y, t0, sp, life, ph in motes:
            age = f - t0
            if age < 0 or age > life:
                continue
            a = 0.9 * (1 - age / life) * (0.6 + 0.4 * math.sin(ph + age * 2.2))
            blob(cv, x + math.sin(ph + age) * 2, y - sp * age * K, 1.1, a, DK_LIL)
        for x, t0, sz in drops:
            age = f - t0
            if age < 0:
                continue
            blob(cv, x, PY1 + 0.6 * age * age * K * 1.4, sz, 0.85 * math.exp(-age / 4), DK_INK, sx=1.0, sy=1.5)
            blob(cv, x, PY1 + 0.6 * age * age * K * 1.4 - 0.8, sz * 0.4, 0.6 * math.exp(-age / 4), DK_VIO)
        return finish(cv, (150, 96, 226))

    return [frame(f) for f in range(PN)]


# ------------------------------------------------------------------ light_toggle (one-shot)
def light_toggle():
    W, H, N = 112 * K, 112 * K, 37
    cx, cy = 56.0 * K, 56.0 * K
    fy = 86.0 * K
    r = np.random.default_rng(3)
    motes = [(cx + r.uniform(-60, 60), fy - r.uniform(0, 30), r.uniform(0.6, 1.5), r.uniform(1.0, 2.0), int(r.integers(4, 18)),
              r.uniform(12, 18)) for _ in range(16)]
    tex = [brush_tex(i + 2) for i in range(8)]

    def frame(f):
        if f == N - 1:
            return EMPTY(W, H)
        cv, yy, xx = ctx(W, H)
        d = np.hypot(xx - cx, yy - cy)
        # charco de luz cálida en el piso
        pool = smoothstep(1, 6, f) * (1 - smoothstep(24, 35, f))
        put(cv, 0.42 * pool * np.exp(-(((xx - cx) / 52) ** 2 + ((yy - fy) / 9) ** 2) / 2), LY_AMB)
        put(cv, 0.3 * pool * np.exp(-(((xx - cx) / 26) ** 2 + ((yy - fy) / 4.5) ** 2) / 2), LY_YEL)
        # halo
        halo = [0.3, 0.55, 0.6, 0.55, 0.48, 0.42][f] if f < 6 else 0.42 * (1 - smoothstep(6, 32, f))
        put(cv, halo * np.exp(-d ** 2 / (2 * (26 + 0.6 * min(f, 12)) ** 2)), LY_AMB)
        # onda
        if 1 <= f <= 11:
            k = f - 1
            put(cv, 0.5 * (1 - k / 10) * np.exp(-(d - (18 + 7 * k)) ** 2 / (2 * (1.5 + 0.35 * k) ** 2)), LY_YEL)
        # 8 rayos de pincel que salen y se retraen
        for i in range(8):
            th = i * math.pi / 4 - math.pi / 2
            ln = 72 if i % 2 == 0 else 48
            t = (f - 1 - 0.4 * (i % 2)) / 14
            if t <= 0 or t >= 1:
                continue
            ext = math.sin(math.pi * t) ** 0.7
            ri, ro = 16 + 8 * t, 16 + ln * ext
            if ro - ri < 3:
                continue
            p0 = (cx + math.cos(th) * ri, cy + math.sin(th) * ri)
            p1 = (cx + math.cos(th) * ro, cy + math.sin(th) * ro)
            a = 0.95 * (1 - 0.5 * t)
            line(cv, p0, p1, 1.0, 0.4, 0.35 * a, LY_AMB, wmid=11 if i % 2 == 0 else 8)
            line(cv, p0, p1, 0.4, 0.2, a, LY_WHITE, wmid=4.5 if i % 2 == 0 else 3.2, tex=tex[i],
                 afn=lambda q, a=a: a * smoothstep(0, 0.2, q) * (1 - 0.4 * q))
        # motas de luz
        for x, y, sp, sz, t0, life in motes:
            age = f - t0
            if age < 0 or age > life:
                continue
            a = 0.8 * math.sin(math.pi * age / life)
            blob(cv, x, y - sp * age, sz, a, LY_WHITE)
        # destello central
        ca = [0.7, 1, 1, 0.9, 0.75, 0.6, 0.5, 0.42][f] if f < 8 else 0.42 * (1 - smoothstep(8, 30, f))
        cr = [8, 14, 15, 14, 13, 12, 11, 10][f] if f < 8 else 10 - 4 * smoothstep(8, 30, f)
        put(cv, ca * np.exp(-d ** 2 / (2 * cr ** 2)) ** 0.8, LY_HOT)
        return finish(cv, (255, 214, 96))

    return [frame(f) for f in range(N)]


# ------------------------------------------------------------------ export
def write(name, frames, bleed_col):
    with open(os.path.join(SRC, name + ".json")) as fh:
        js = json.load(fh)
    m, px = js["meta"], js["meta"]["pixelfx"]
    fw, fh_ = px["frameSize"]
    cols = px["columns"]
    rows = m["size"]["h"] // fh_
    assert len(frames) == px["frames"] == len(js["frames"]), name
    assert frames[0].shape[:2] == (fh_ * K, fw * K), (name, frames[0].shape)
    img = sheet([bleed(f, bleed_col) for f in frames], cols, rows)
    Image.fromarray(img).save(os.path.join(OUT, name + ".png"), optimize=True)
    for fr in js["frames"]:
        for key in ("frame", "spriteSourceSize"):
            for k2 in ("x", "y", "w", "h"):
                fr[key][k2] *= K
        fr["sourceSize"]["w"] *= K
        fr["sourceSize"]["h"] *= K
    m["app"] = "vfx-pintados-2 (numpy+PIL)"
    m["size"] = {"w": m["size"]["w"] * K, "h": m["size"]["h"] * K}
    px["frameSize"] = [fw * K, fh_ * K]
    px["pivot"] = [px["pivot"][0] * K, px["pivot"][1] * K]
    px["name"] += " (pintado)"
    with open(os.path.join(OUT, name + ".json"), "w") as fh:
        json.dump(js, fh, indent=1)


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    rep = []
    only = set(sys.argv[3:])

    def go(name):
        return not only or name in only

    if go("extra_defeat"):
        fr = extra_defeat(); write("extra_defeat", fr, (200, 198, 192)); rep.append(f"extra_defeat: último frame vacío = {fr[-1][..., 3].max() == 0}")
    if go("extra_stun"):
        fr, nx = extra_stun(); write("extra_stun", fr, (226, 150, 92)); rep.append(loop_report("extra_stun", fr, nx))
    if go("scripted_bounce"):
        fr = scripted_bounce(); write("scripted_bounce", fr, (230, 226, 216)); rep.append(f"scripted_bounce: último frame vacío = {fr[-1][..., 3].max() == 0}")
    if go("rewind"):
        fr, nx = rewind(); write("rewind", fr, (90, 222, 244)); rep.append(loop_report("rewind", fr, nx))
    for nm, fn, col in (("light_piece_appear", light_piece_appear, (255, 214, 96)), ("light_piece_vanish", light_piece_vanish, (255, 214, 96)),
                        ("dark_piece_appear", dark_piece_appear, (150, 96, 226)), ("dark_piece_vanish", dark_piece_vanish, (150, 96, 226)),
                        ("light_toggle", light_toggle, (255, 214, 96))):
        if go(nm):
            fr = fn(); write(nm, fr, col); rep.append(f"{nm}: último frame vacío = {fr[-1][..., 3].max() == 0}")
    print("\n".join(rep))
    if not only:
        with open(os.path.join(OUT, "src", "loop_check.txt"), "w") as fh:
            fh.write("\n".join(rep) + "\n")
