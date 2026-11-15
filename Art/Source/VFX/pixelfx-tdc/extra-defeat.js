// The Director's Cut: an extra is cut out of the shot. A film iris snaps shut on him with a lit rim, then a
// remix of spawn/poof in greys (white pop, smoke balls, cream glints) lifts away.
import { clamp, ease, fit, hex, WHITE } from "../../core/index.js";
import * as poof from "../spawn/poof.js";
import { layer } from "./_mix.js";

export const meta = {
  name: "Extra defeat",
  desc: "A film iris snaps shut on the extra with a cream rim and a white flash, then he goes up in a grey smoke poof with cream glints that lifts and thins away.",
  tags: ["tdc", "death", "despawn", "poof", "iris", "one-shot"],
  size: [112, 108],
  origin: [56, 80],
  duration: 1.5,
  loop: false,
  fps: 30,
  bg: "dark",
  options: {
    palette: ["#1c1b22", "#3e3d46", "#6a6872", "#9d9ba4", "#d2d0d6", "#f4f2ee"],
    stars: ["#d8c8a8", "#f4ead2", "#ffffff"],
    ink: "#0b0a10", rim: "#f4ead2",
    cy: -22, iris: 0.14, speed: 1.2, seed: 21,
  },
};

export function create(o) {
  const P = layer(poof, { palette: o.palette, stars: o.stars, seed: o.seed }, { speed: o.speed, delay: o.iris });
  const ink = hex(o.ink), rim = hex(o.rim), C = fit(o.palette, 6);
  let t = 0;
  const iris = (p) => {
    const I = o.iris;
    if (t > I + 0.12) return;
    if (t <= I) {
      // the iris closes: a thick black band with a lit inner rim, racing in from the edges
      const u = ease.inCubic(clamp(t / I)), R = 40 - 26 * u, w = 6 + 10 * u;
      p.ring(0, o.cy, R, w, (v) => (v < 0.18 ? rim : v < 0.3 ? C[2] : ink), 1);
      p.glowRing(0, o.cy, R - w, 4, rim, 0.25 + 0.4 * u);
    } else {
      // after the cut the band thins and blows out as a hard white ring
      const u = clamp((t - I) / 0.12), R = 14 + 24 * ease.outQuart(u), w = Math.max(1, 3 * (1 - u));
      p.ring(0, o.cy, R, w, u < 0.35 ? WHITE : rim, 1 - u);
    }
  };
  return {
    update(dt) { t += dt; P.update(dt); },
    drawBack(p) { P.drawBack(p); },
    draw(p) { P.draw(p); iris(p); },
  };
}
