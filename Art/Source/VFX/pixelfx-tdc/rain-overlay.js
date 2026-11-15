// The Director's Cut: a rain card to lay over the whole view. Thin slanted streaks at three depths on a
// transparent canvas, no floor, no puddles, no haze: only the drops. A pure function of t; loops exactly.
import { fit, rand } from "../../core/index.js";
import { clip, trips } from "../weather/_lib.js";

export const meta = {
  name: "Rain overlay",
  desc: "Fine blue-white rain streaks at three depths falling on a transparent canvas, for a full-screen card.",
  tags: ["tdc", "weather", "rain", "overlay", "loop"],
  size: [256, 256],
  origin: [128, 256],
  duration: 2,
  loop: true,
  fps: 30,
  bg: "dark",
  options: {
    palette: ["#0e1322", "#1a2440", "#283a5e", "#3a5482", "#5674a8", "#84a6d6", "#c0dcf8", "#f2f8ff"],
    density: 1,
    seed: 7,
  },
};

const D = 2, SLANT = -0.12;
// far to near: count, falls per loop, streak length, color index, alpha
const LAYERS = [
  { n: 70, m: [2, 2], len: 5, col: 4, a: 0.35 },
  { n: 40, m: [3, 3], len: 9, col: 5, a: 0.6 },
  { n: 16, m: [4, 4], len: 16, col: 6, a: 0.85 },
];

export function draw(p, t, o) {
  t = Math.round(t * 3e4) / 3e4;
  const q = clip(p, meta), C = fit(o.palette, 8);
  LAYERS.forEach((L, li) => {
    const sd = o.seed * 13 + li;
    trips(t, D, Math.round(L.n * o.density), sd, L.m[0], L.m[1], (u, k, m, i) => {
      const j = ((k % m) + m) % m;
      const x0 = rand(sd, i, j, 2) * 300 - 150;
      const y = -256 - L.len + (256 + 2 * L.len) * u, x = x0 - y * SLANT;
      for (let s = 0; s < L.len; s++) {
        const v = s / L.len; // 0 head .. 1 tail
        q.blend(x + s * SLANT, y - s, v < 0.15 ? C[L.col + 1] : C[L.col], L.a * (1 - v * 0.8));
      }
    });
  });
}
