// The Director's Cut: the blow bounces off a scripted (immune) extra. A remix of impacts/parry in black and
// white (four-point star, a ring breaking into arcs, sparks glancing up and raining down) with an inked "X"
// stamped over the contact point.
import { clamp, ease, hex, WHITE } from "../../core/index.js";
import * as parry from "../impacts/parry.js";
import { layer } from "./_mix.js";

export const meta = {
  name: "Scripted bounce",
  desc: "A blow glances off an immune extra: a white four-point star and ring, a black X with a white rim stamped on the contact point, grey sparks deflected up and away.",
  tags: ["tdc", "impact", "block", "immune", "deflect", "one-shot"],
  size: [128, 112],
  origin: [64, 96],
  duration: 0.8,
  loop: false,
  fps: 30,
  bg: "dark",
  options: {
    palette: ["#0b0a10", "#24222a", "#46434e", "#7a7682", "#b4b0ba", "#e6e4ea", "#ffffff"],
    ink: "#0b0a10", height: 30, arm: 9, speed: 1.25, seed: 11,
  },
};

export function create(o) {
  const P = layer(parry, { palette: o.palette, height: o.height, seed: o.seed }, { speed: o.speed });
  const ink = hex(o.ink), Y = -o.height;
  let t = 0;
  const stamp = (p) => {
    if (t < 0.02 || t > 0.5) return;
    const pop = 1 + 0.5 * (1 - ease.outBack(clamp((t - 0.02) / 0.08))), A = o.arm * pop;
    const a = t < 0.34 ? 1 : 1 - (t - 0.34) / 0.16;
    // jolt sideways for a few frames, like the extra didn't budge but the hit did
    const jx = t < 0.14 ? Math.round(Math.sin(t * 90) * 1.5) : 0;
    for (const [w, c] of [[6, WHITE], [3.2, ink]]) {
      p.beam(jx - A, Y - A, jx + A, Y + A, w, c, a);
      p.beam(jx - A, Y + A, jx + A, Y - A, w, c, a);
    }
    p.glow(jx, Y, 14, WHITE, 0.35 * a);
  };
  return {
    update(dt) { t += dt; P.update(dt); },
    drawBack(p) { P.drawBack(p); },
    draw(p) { P.draw(p); stamp(p); },
  };
}
