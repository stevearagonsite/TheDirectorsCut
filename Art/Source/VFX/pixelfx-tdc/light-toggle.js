// The Director's Cut: a spotlight switches on. A remix of pickups/collect-burst (white flash, cross of light,
// rings, twinkles) in warm yellow, with eight rays that shoot out with the flash and pull back in.
import { clamp, ease, fit, Rng, TAU, WHITE } from "../../core/index.js";
import * as burst from "../pickups/collect-burst.js";
import { layer } from "./_mix.js";

export const meta = {
  name: "Light toggle",
  desc: "A spotlight clicks on: a warm white flash and a cross of light, rays flaring out round it, a thin ring snapping out and warm yellow twinkles drifting off.",
  tags: ["tdc", "light", "flash", "spotlight", "one-shot"],
  size: [112, 112],
  origin: [56, 86],
  duration: 1.2,
  loop: false,
  fps: 30,
  bg: "dark",
  options: {
    palette: ["#2a1a06", "#5a3a0c", "#9a6614", "#d89a22", "#f6c63c", "#ffe27a", "#fff4c8", "#ffffff"],
    rays: 8, raysFor: 0.4, speed: 1.15, seed: 3,
  },
};

export function create(o) {
  const B = layer(burst, { palette: o.palette, seed: o.seed }, { speed: o.speed });
  const C = fit(o.palette, 8), R = new Rng(o.seed + 7), CY = -30;
  const rays = Array.from({ length: o.rays }, (_, i) => ({ a: (i / o.rays) * TAU + R.range(-0.05, 0.05), L: i % 2 ? R.range(20, 26) : R.range(34, 42), d: R.range(0, 0.03) }));
  let t = 0;
  // rays shoot out in two frames, hold a beat, then pull back to the core from the tip
  const drawRays = (p) => {
    for (const r of rays) {
      const u = (t - r.d) / o.raysFor;
      if (u < 0 || u >= 1) continue;
      const out = ease.outQuart(clamp(u / 0.12)), back = ease.inCubic(clamp((u - 0.3) / 0.7));
      const r0 = 4 + r.L * 0.1, r1 = r0 + r.L * out * (1 - back);
      if (r1 - r0 < 1) continue;
      const cx = Math.cos(r.a), sy = Math.sin(r.a);
      p.line(cx * r0, CY + sy * r0, cx * r1, CY + sy * r1, (v) => (v < 0.35 ? WHITE : v < 0.7 ? C[6] : C[5]));
      p.glowLine(cx * r0, CY + sy * r0, cx * r1, CY + sy * r1, 3, C[5], 0.35 * (1 - back));
    }
  };
  return {
    update(dt) { t += dt; B.update(dt); },
    drawBack(p) { B.drawBack(p); },
    draw(p) {
      // the warm pool of the lamp that stays on, fading out as the sheet ends
      const k = ease.outQuart(clamp(t / 0.05)) * (1 - ease.inQuad(clamp((t - 0.15) / 0.85)));
      if (k > 0) { p.glow(0, CY, 40, C[4], 0.32 * k); p.glow(0, CY, 12, C[6], 0.35 * k); }
      drawRays(p);
      B.draw(p);
    },
  };
}
