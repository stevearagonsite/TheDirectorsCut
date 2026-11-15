// The Director's Cut: a light piece vanishes. The platform of light flares white for a beat, then comes
// apart in 2 x 2 cells from both ends inward; each cell heats to white and leaves as a yellow spark that
// rises, cools down the ramp and flickers out.
import { clamp, ease, fit, Rng, WHITE } from "../../core/index.js";
import { drawBar, LIGHT, META, Motes, X0, X1, Y0, Y1 } from "./_piece.js";

const POP = 0.05, BREAK0 = 0.04, BREAK1 = 0.22, END = 0.385;

export const meta = {
  name: "Light piece vanish",
  desc: "A platform of light flares, then breaks into 2x2 cells from the ends inward; each cell heats white and rises as a yellow spark that cools and flickers out.",
  tags: ["tdc", "light", "platform", "dissolve", "particles", "one-shot"],
  ...META,
  options: { palette: LIGHT, seed: 7 },
};

export function create(o) {
  const C = fit(o.palette, 8), R = new Rng(o.seed), sparks = new Motes();
  // each 2 x 2 cell gets the time it lets go: ends first, the top a touch earlier, plus jitter
  const cells = new Map();
  for (let y = Y0; y < Y1; y += 2) {
    for (let x = X0; x < X1; x += 2) {
      const mid = 1 - Math.abs(x + 1) / (X1 - X0) * 2; // 0 at the ends, 1 in the middle
      const u = clamp(0.5 * mid + 0.15 * ((y - Y0) / (Y1 - Y0)) + R.range(0, 0.4));
      cells.set(`${x},${y}`, { x, y, td: BREAK0 + (BREAK1 - BREAK0) * u, gone: false });
    }
  }
  const cellOf = (x, y) => cells.get(`${x - ((x - X0) & 1)},${y - ((y - Y0) & 1)}`);
  let t = 0;
  return {
    update(dt) {
      t += dt;
      sparks.update(dt);
      for (const c of cells.values()) {
        if (c.gone || t < c.td) continue;
        c.gone = true;
        const n = R.chance(0.5) ? 1 : 0; // half the cells just wink out, so the sparks stay readable
        for (let i = 0; i < n; i++) {
          const life = Math.min(R.range(0.14, 0.3), END - t);
          if (life < 0.03) continue;
          sparks.add({ kind: "spark", x: c.x + R.range(0, 2), y: c.y + R.range(0, 2), vx: R.range(-16, 16), vy: R.range(-150, -60), ay: -110, drag: 1.6, life, size: R.chance(0.45) ? 2 : 1, seed: R.range(0, 100) });
        }
      }
    },
    draw(p) {
      const pop = t < POP ? ease.outQuad(1 - t / POP) : 0;
      if (t < BREAK1 + 0.01) {
        drawBar(p, C, (x, y) => {
          const c = cellOf(x, y);
          if (!c || c.gone) return null;
          const heat = clamp(1 - (c.td - t) / 0.05); // a cell whitens just before it goes
          return { b: Math.max(0.35 + pop * 0.6, heat), a: 1 };
        }, { glowK: 0.2 + pop * 0.3 });
      }
      if (pop > 0) p.glow(0, 0, 40, C[5], 0.3 * pop);
      sparks.draw(p, C);
    },
  };
}
