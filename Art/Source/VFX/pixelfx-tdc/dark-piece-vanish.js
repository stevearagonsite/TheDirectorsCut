// The Director's Cut: a dark piece vanishes. Ink floods the platform of shadow (the rim flares violet, the
// fill goes black), then it comes apart in 2 x 2 cells: each one stains dark and lets go as a curl of violet
// smoke, a few violet motes rising out of it, and the odd ink drop falling away below.
import { clamp, ease, fit, Rng } from "../../core/index.js";
import { DARK, drawBar, META, Motes, X0, X1, Y0, Y1 } from "./_piece.js";

const POP = 0.06, BREAK0 = 0.05, BREAK1 = 0.23, END = 0.385;

export const meta = {
  name: "Dark piece vanish",
  desc: "Ink floods a platform of shadow, then it breaks into 2x2 cells that stain dark and leave as violet smoke curling up, violet motes rising and ink drops falling away.",
  tags: ["tdc", "dark", "platform", "dissolve", "ink", "smoke", "one-shot"],
  ...META,
  options: { palette: DARK, seed: 9 },
};

export function create(o) {
  const C = fit(o.palette, 8), R = new Rng(o.seed), fx = new Motes();
  // cells go in drifting blots: a slow wave from the left plus jitter, the top a touch earlier
  const cells = new Map();
  for (let y = Y0; y < Y1; y += 2) {
    for (let x = X0; x < X1; x += 2) {
      const wave = 0.5 + 0.5 * Math.sin((x - X0) * 0.09 + o.seed); // blotchy, not a straight wipe
      const u = clamp(0.35 * wave + 0.3 * ((x - X0) / (X1 - X0)) + 0.1 * ((y - Y0) / (Y1 - Y0)) + R.range(0, 0.3));
      cells.set(`${x},${y}`, { x, y, td: BREAK0 + (BREAK1 - BREAK0) * u, gone: false });
    }
  }
  const cellOf = (x, y) => cells.get(`${x - ((x - X0) & 1)},${y - ((y - Y0) & 1)}`);
  let t = 0;
  return {
    update(dt) {
      t += dt;
      fx.update(dt);
      for (const c of cells.values()) {
        if (c.gone || t < c.td) continue;
        c.gone = true;
        const room = END - t, cx = c.x + 1, cy = c.y + 1;
        if (R.chance(0.3)) fx.add({ kind: "smoke", x: cx, y: cy, vx: R.range(-10, 10), vy: R.range(-45, -15), ay: -40, drag: 2, life: Math.min(R.range(0.14, 0.26), room), size: R.range(1.8, 3.6), seed: R.range(0, 100) });
        if (R.chance(0.22)) fx.add({ kind: "spark", x: cx, y: cy, vx: R.range(-12, 12), vy: R.range(-110, -50), ay: -80, drag: 1.8, life: Math.min(R.range(0.1, 0.22), room), size: 1, seed: R.range(0, 100) });
        if (c.y >= Y1 - 2 && R.chance(0.18)) fx.add({ kind: "drop", x: cx, y: cy + 1, vx: R.range(-4, 4), vy: R.range(10, 40), ay: 520, life: Math.min(R.range(0.08, 0.14), room), seed: R.range(0, 100) });
      }
    },
    draw(p) {
      const pop = t < POP ? ease.outQuad(1 - t / POP) : 0;
      fx.draw(p, C, "back");
      if (t < BREAK1 + 0.01) {
        drawBar(p, C, (x, y) => {
          const c = cellOf(x, y);
          if (!c || c.gone) return null;
          const rim = y === Y0 || y === Y1 - 1 || x === X0 || x === X1 - 1;
          return { b: rim ? 0.35 + pop * 0.55 : 0.3, a: 1 }; // the rim flares, the fill stays ink
        }, { glowK: 0.2 + pop * 0.3 });
        // the ink: the fill floods dark on the pop, and each cell stains black just before it goes
        for (const c of cells.values()) {
          if (c.gone) continue;
          const pre = clamp(1 - (c.td - t) / 0.06) * 0.85, base = 0.25 + 0.4 * clamp(t / POP);
          for (let dy = 0; dy < 2; dy++) for (let dx = 0; dx < 2; dx++) {
            const x = c.x + dx, y = c.y + dy, rim = y === Y0 || y === Y1 - 1 || x === X0 || x === X1 - 1;
            p.blend(x, y, C[0], rim ? pre * 0.8 : Math.max(base, pre));
          }
        }
      }
      if (pop > 0) { p.glow(0, 0, 40, C[4], 0.32 * pop); p.glow(0, 0, 12, C[6], 0.2 * pop); }
      fx.draw(p, C, "front");
    },
  };
}
