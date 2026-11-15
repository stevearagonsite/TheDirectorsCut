// The Director's Cut: a dark piece appears. Ink bleeds into the platform from the left: it runs ahead along
// the outline first, the fill floods in behind a wet, wobbling front with a violet meniscus, and violet
// smoke curls up off the front. A blot splashes where it starts; a puff closes it at the far end.
import { clamp, ease, fit, noise2, Rng } from "../../core/index.js";
import { DARK, drawBar, META, Motes, X0, X1, Y0, Y1 } from "./_piece.js";

const SWEEP = 0.25, END = 0.385, LEAD = 12;

export const meta = {
  name: "Dark piece appear",
  desc: "Ink bleeds into a platform of shadow from left to right: it runs ahead along the outline, floods the fill behind a wet violet front, violet smoke curling up off it.",
  tags: ["tdc", "dark", "platform", "ink", "smoke", "reveal", "one-shot"],
  ...META,
  options: { palette: DARK, seed: 5 },
};

export function create(o) {
  const C = fit(o.palette, 8), R = new Rng(o.seed), smoke = new Motes(), seed = o.seed * 13;
  const front = (t) => X0 - 2 + (X1 - X0 + 12) * ease.outQuad(clamp(t / SWEEP));
  const edge = (y, t) => front(t) + (noise2(y * 0.28, t * 14, seed) - 0.5) * 4.5; // the wet front wobbles
  let t = 0, acc = 0;
  return {
    update(dt) {
      const f0 = front(t);
      t += dt;
      smoke.update(dt);
      if (t > SWEEP) return;
      const f1 = front(t);
      acc += (f1 - f0) * 0.2;
      while (acc >= 1) {
        acc -= 1;
        const life = Math.min(R.range(0.14, 0.28), END - t);
        if (life < 0.04) continue;
        const up = R.chance(0.7);
        smoke.add({ kind: "smoke", x: Math.min(f1, X1 - 3) + R.range(-6, 1), y: up ? Y0 + R.range(-1, 2) : Y1 + R.range(-1, 1), vx: R.range(-8, 12), vy: up ? R.range(-40, -14) : R.range(-6, 10), drag: 2, ay: -30, life, size: R.range(1.6, 5), seed: R.range(0, 100) });
      }
      if (t >= SWEEP - dt && t < SWEEP + dt) {
        for (let i = 0; i < 5; i++) smoke.add({ kind: "smoke", x: X1 - R.range(3, 10), y: R.range(Y0 - 1, Y1), vx: R.range(-4, 10), vy: R.range(-30, -6), drag: 2.5, ay: -20, life: Math.min(R.range(0.12, 0.16), END - t), size: R.range(2.5, 4.5), seed: R.range(0, 100) });
      }
    },
    draw(p) {
      const f = front(t), fade = 1 - ease.inQuad(clamp((t - SWEEP) / (END - SWEEP)));
      smoke.draw(p, C, "back");
      if (fade <= 0) return;
      const lead = Math.min(X1, f + LEAD * (1 - clamp((t - SWEEP * 0.8) / 0.06)));
      // the bar: the outline inks in ahead of the fill, the fill behind the wet front
      drawBar(p, C, (x, y) => {
        const rim = y === Y0 || y === Y1 - 1;
        const e = edge(y, t);
        if (x > (rim ? Math.max(e, lead) : e)) return null;
        const b = x > e ? 0.1 + 0.15 * clamp((lead - x) / LEAD) : 0.15 + 0.6 * Math.exp(-(e - x) / 10);
        return { b: b * (0.6 + 0.4 * fade), a: fade };
      }, { glowK: 0.24, fill: 0.36 });
      if (t <= SWEEP + 0.02) {
        const k = Math.min(1, t / 0.03) * (1 - clamp((t - SWEEP) / 0.04));
        // the wet front: a band of dark ink with a violet meniscus on its leading edge
        for (let y = Y0; y < Y1; y++) {
          const e = Math.round(edge(y, t));
          if (e < X0 || e >= X1) continue;
          for (let d = 1; d <= 4; d++) if (e - d >= X0) p.blend(e - d, y, C[1], k * (0.8 - d * 0.12));
          p.px(e, y, y === Y0 ? C[7] : C[6]);
          p.light(e, y, C[5], 0.3 * k);
        }
        // ink runs along the outline ahead of the fill: a dark tip and a bright bead
        for (const y of [Y0, Y1 - 1]) {
          if (lead < X0 + 1 || lead > X1 - 1) continue;
          p.px(lead, y, C[7]);
          p.px(lead - 1, y, C[2]);
          p.light(lead, y, C[5], 0.4 * k);
        }
        // the ink cloud billowing at the front, above and below the bar, lit violet on its upper edge
        for (let i = 0; i < 4; i++) {
          const cx = Math.min(X1, f) - 2 - i * 4 + Math.sin(t * 40 + i * 2) * 1.2, cy = i % 2 ? Y1 + 0.5 : Y0 - 0.5, r = (5.5 - i * 0.8) * (0.7 + 0.3 * k);
          p.disc(cx, cy, r, C[4], (d) => 0.85 * k * Math.pow(1 - d, 0.5));
          p.disc(cx + r * 0.15, cy + r * 0.25, r * 0.6, C[1], (d) => 0.8 * k * (1 - d * 0.6));
          p.disc(cx - r * 0.3, cy - r * 0.4, r * 0.45, C[6], (d) => 0.75 * k * (1 - d));
        }
        // ink tendrils curling back off the front, over and under the bar
        for (const [y0, dir, ph] of [[Y0 - 2, -1, 0], [Y1 + 1, 1, 2], [Y0 - 1, -1, 4]]) {
          for (let d = 0; d < 16; d++) {
            const x = Math.min(X1, f) - 3 - d, y = y0 + dir * d * 0.22 + Math.sin(d * 0.55 + t * 30 + ph) * 1.4;
            p.blend(x, y, d < 5 ? C[6] : C[4], k * (1 - d / 16));
          }
        }
        p.glow(f - 2, 0, 18, C[4], 0.28 * k);
        p.glow(f, 0, 6, C[5], 0.3 * k);
      }
      // the first drop: an ink blot that splashes at the start of the bar
      if (t < 0.1) {
        const u = t / 0.1, r = 2 + ease.outQuart(u) * 6;
        p.disc(X0 + 1, 0, r, C[1], (d) => 0.75 * (1 - u) * (1 - d * 0.5));
        p.ring(X0 + 1, 0, r + 1, 1, u < 0.35 ? C[7] : u < 0.7 ? C[5] : C[3]);
        p.light(X0 + 1, 0, C[6], 0.6 * (1 - u));
      }
      smoke.draw(p, C, "front");
    },
  };
}
