// The Director's Cut: steam rising from a manhole / sewer grate. Soft translucent blue-grey puffs that leave the
// grate narrow, swell, sway and thin out as they climb. A pure function of t; loops exactly.
import { fit, rand, clamp } from "../../core/index.js";
import { clip, trips } from "../weather/_lib.js";

export const meta = {
  name: "Steam vent",
  desc: "Blue-grey steam curling up from a street grate in a slow loop, puffs swelling and thinning as they rise.",
  tags: ["tdc", "steam", "vapor", "smoke", "street", "loop"],
  size: [128, 256],
  origin: [64, 252],
  duration: 3,
  loop: true,
  fps: 30,
  bg: "dark",
  options: {
    palette: ["#1c2230", "#2c3446", "#3e4860", "#56627a", "#717d94", "#909bb0", "#b4bdcc", "#d6dce6"],
    count: 46,
    seed: 3,
  },
};

const D = 3, TAU = Math.PI * 2;

export function draw(p, t, o) {
  t = Math.round(t * 3e4) / 3e4;
  const q = clip(p, meta), C = fit(o.palette, 8), S = o.seed;
  const puffs = [];
  trips(t, D, o.count, S, 1, 2, (u, k, m, i) => {
    const j = ((k % m) + m) % m;
    const r0 = rand(S, i, j, 1), r1 = rand(S, i, j, 2), r2 = rand(S, i, j, 3);
    const H = 150 + 90 * r1; // how high it climbs
    const e = 1 - (1 - u) * (1 - u) * 0.6 - 0.4 * (1 - u); // rises fast, slows down
    const y = -2 - H * clamp(e);
    const sway = Math.sin(TAU * (t / D) * m + r2 * TAU) * (4 + 14 * u);
    const x = (r0 - 0.5) * 20 + sway + 8 * u * u;
    const r = 8 + 24 * Math.pow(u, 0.8) * (0.7 + 0.5 * r2);
    const a = clamp(u / 0.12) * Math.pow(1 - u, 1.4) * (0.38 + 0.16 * r1);
    puffs.push({ x, y, r, a, u, tone: r1 });
  });
  // the oldest (widest, highest) behind, fresh dense puffs in front
  puffs.sort((A, B) => B.u - A.u);
  for (const f of puffs) {
    const core = f.u < 0.3 ? C[5] : f.tone > 0.5 ? C[5] : C[4];
    q.disc(f.x, f.y, f.r, (d) => (d < 0.5 ? core : C[4]), (d) => f.a * (1 - d) * (1 - d) * 1.3);
  }
}
