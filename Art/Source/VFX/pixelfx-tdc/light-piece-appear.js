// The Director's Cut: a light piece appears. A spotlight sweeps the platform left to right and "reveals" it:
// a white-hot vertical blade of light with a warm halo, the bar lighting up in its wake and cooling to the
// in-game material, light dust lifting off where it passes, a glint at each end.
import { clamp, ease, fit, Rng, sparkle, WHITE } from "../../core/index.js";
import { drawBar, LIGHT, META, Motes, X0, X1, Y0, Y1 } from "./_piece.js";

const SWEEP = 0.24, END = 0.385;

export const meta = {
  name: "Light piece appear",
  desc: "A spotlight sweeps a platform of light from left to right and reveals it: white blade, warm halo, the bar lighting up and cooling behind, dust of light lifting off.",
  tags: ["tdc", "light", "platform", "reveal", "sweep", "one-shot"],
  ...META,
  options: { palette: LIGHT, seed: 4 },
};

export function create(o) {
  const C = fit(o.palette, 8), R = new Rng(o.seed), dust = new Motes();
  const head = (t) => X0 - 4 + (X1 - X0 + 8) * ease.inOutSine(clamp(t / SWEEP));
  // the time the blade passes x
  const passT = (x) => {
    const u = clamp((x - (X0 - 4)) / (X1 - X0 + 8));
    return (Math.acos(1 - 2 * u) / Math.PI) * SWEEP;
  };
  let t = 0, acc = 0;
  return {
    update(dt) {
      const s0 = head(t);
      t += dt;
      dust.update(dt);
      if (t > SWEEP + 0.01) return;
      const s1 = head(t);
      acc += Math.abs(s1 - s0) * 0.55; // dust per px swept
      while (acc >= 1) {
        acc -= 1;
        const x = s1 + R.range(-3, 1), y = R.range(Y0 - 1, Y1);
        const life = Math.min(R.range(0.1, 0.24), END - t);
        if (life <= 0.03) continue;
        const big = R.chance(0.22);
        dust.add({ kind: "dust", x, y, vx: R.range(-10, 26), vy: big ? R.range(-30, -14) : R.range(-46, -6), drag: 3, ay: -25, life: big ? Math.min(life * 1.3, END - t) : life, size: big ? 2 : 1, seed: R.range(0, 100) });
      }
    },
    draw(p) {
      const s = head(t), fade = 1 - ease.inQuad(clamp((t - SWEEP) / (END - SWEEP)));
      if (fade <= 0) { dust.draw(p, C); return; }
      // the bar: revealed behind the blade, hot where it just passed, settling to a lit rim
      drawBar(p, C, (x) => {
        if (x > s) return null;
        const age = t - passT(x);
        const b = Math.exp(-age * 16) * 0.95 + 0.15;
        return { b: b * fade, a: fade * (x > s - 2 ? 0.8 : 1) };
      }, { glowK: 0.22 });
      // the blade: a vertical shaft of light taller than the bar, white core, warm sides, dithered ends
      if (t <= SWEEP + 0.02) {
        const k = Math.min(1, t / 0.03) * (1 - clamp((t - SWEEP) / 0.04));
        const top = Y0 - 10, bot = Y1 + 7;
        // ends step down the ramp instead of dithering into dots
        const ramp = (w) => (u) => { const d = Math.abs(u - 0.47) / w; return d < 0.55 ? WHITE : d < 0.75 ? C[6] : d < 0.9 ? C[5] : C[4]; };
        if (k > 0.5) {
          p.line(s, top, s, bot, ramp(0.53));
          p.line(s - 1, top + 3, s - 1, bot - 2, ramp(0.4));
          p.line(s + 1, top + 5, s + 1, bot - 4, (u) => (Math.abs(u - 0.5) < 0.3 ? C[6] : C[5]));
        } else {
          p.line(s, top + 4, s, bot - 3, C[5]);
        }
        // a short lens streak along the bar where the blade crosses it
        p.line(s - 9, -1, s + 5, -1, (u) => (u > 0.5 && u < 0.75 ? WHITE : C[6]), (u) => k * 0.9 * clamp(1 - Math.abs(u - 0.64) * 2.4));
        p.glowLine(s, top + 2, s, bot - 2, 4, C[6], 0.45 * k);
        p.glow(s, 0, 20, C[4], 0.22 * k);
        // the cone of the reflector trailing behind the blade, as light only
        for (let d = 2; d < 22; d += 2) p.glowLine(s - d, Y0 - 2, s - d, Y1 + 1, 2, C[4], 0.05 * k * (1 - d / 22));
      }
      // glints: the start of the sweep and the moment it reaches the far end
      if (t < 0.08) sparkle(p, X0, -1, 6 * (1 - t / 0.08) + 1, WHITE, C[5], 0.6);
      const u = (t - SWEEP + 0.02) / 0.12;
      if (u > 0 && u < 1) sparkle(p, X1 - 1, -1, 8 * ease.outQuad(1 - u) + 1, WHITE, C[5], 0.7 * (1 - u));
      dust.draw(p, C);
    },
  };
}
