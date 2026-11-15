// The Director's Cut: something in the shot rewinds. A remix of pickups/xp-orb played backwards in neon cyan
// (the comet runs tail-first round its figure-eight), with motes spiralling in counter-clockwise, like a reel
// winding back.
import { cycle, fit, ease, TAU, WHITE } from "../../core/index.js";
import * as orb from "../pickups/xp-orb.js";
import { layer } from "./_mix.js";

export const meta = {
  name: "Rewind",
  desc: "A neon cyan comet runs backwards, tail first, round a figure-eight while motes spiral in counter-clockwise like a reel winding back.",
  tags: ["tdc", "time", "rewind", "reverse", "hologram", "loop"],
  size: [96, 72],
  origin: [48, 60],
  duration: 1.8,
  loop: true,
  fps: 30,
  bg: "dark",
  options: {
    palette: ["#03141c", "#06303e", "#0a5468", "#0e8098", "#16b4cc", "#4ae4f4", "#b4f8ff", "#ffffff"],
    motes: 12, seed: 2,
  },
};

const D = meta.duration, CY = -22;

function motes(p, t, o, far) {
  const C = fit(o.palette, 8);
  cycle(t, D, o.motes, o.seed + 5, (r) => r.range(0.6, 1.0), (age, life, r) => {
    const u = age / life, th0 = r.range(0, TAU), R0 = r.range(26, 36);
    // counter-clockwise, speeding up as they wind in
    const at = (v) => {
      const e = ease.inQuad(v), th = th0 - e * r.range(2.2, 3.2), R = R0 * (1 - e * 0.85);
      return [Math.cos(th) * R, CY + Math.sin(th) * R * 0.55];
    };
    const [x, y] = at(u);
    if ((y > CY) === far) return;
    const a = Math.sin(Math.PI * u);
    for (let k = 5; k >= 1; k--) {
      const [tx, ty] = at(Math.max(0, u - k * 0.03));
      p.dot(tx, ty, k > 3 ? C[2] : k > 1 ? C[3] : C[4], a * (1.1 - k * 0.16));
    }
    p.dot(x, y, u > 0.7 ? WHITE : C[6], a * 1.3);
    p.light(x, y, C[5], 0.35 * a);
  });
}

// two arcs of a reel on a tilted ring, winding back (counter-clockwise) once per loop, arrowheads leading
function reel(p, t, o, far) {
  const C = fit(o.palette, 8), RX = 36, RY = 15, SPAN = 1.7, rot = -TAU * (t / D);
  const pt = (th) => [Math.cos(th) * RX, CY + Math.sin(th) * RY];
  for (let j = 0; j < 2; j++) {
    const lead = rot + j * Math.PI;
    for (let i = 0; i <= 80; i++) {
      const v = i / 80, th = lead + v * SPAN, [x, y] = pt(th);
      if ((Math.sin(th) < 0) !== far) continue;
      const c = v < 0.15 ? WHITE : v < 0.5 ? C[6] : v < 0.8 ? C[5] : C[4];
      p.dot(x, y, far ? C[3] : c, (1.15 - v) * (far ? 0.7 : 1));
      if (v < 0.4) p.light(x, y, C[5], 0.18 * (1 - v));
    }
    // the arrowhead: two short barbs at the leading end, pointing along the motion (decreasing angle)
    const [hx, hy] = pt(lead);
    if ((Math.sin(lead) < 0) === far) {
      const tx = Math.sin(lead) * RX, ty = -Math.cos(lead) * RY, n = Math.hypot(tx, ty) || 1;
      const fx = tx / n, fy = ty / n;
      for (const sgn of [-1, 1]) {
        const bx = hx - fx * 4 - fy * 3 * sgn, by = hy - fy * 4 + fx * 3 * sgn;
        p.line(hx, hy, bx, by, far ? C[4] : C[6]);
      }
      p.px(hx, hy, WHITE);
    }
  }
}

export function drawBack(p, t, o) {
  reel(p, t, o, true);
  orb.drawBack && orbLayer(o).drawBack(p, t);
  motes(p, t, o, true);
}

export function draw(p, t, o) {
  orbLayer(o).draw(p, t);
  motes(p, t, o, false);
  reel(p, t, o, false);
  p.glow(0, CY, 30, fit(o.palette, 8)[4], 0.1);
}

let cache;
function orbLayer(o) {
  if (!cache || cache.o !== o) cache = { o, L: layer(orb, { palette: o.palette, seed: o.seed }, { reverse: true, loop: true, dy: 0 }) };
  return cache.L;
}
