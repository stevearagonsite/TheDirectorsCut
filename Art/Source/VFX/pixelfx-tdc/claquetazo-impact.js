// The Director's Cut: the clapperboard's hit. A remix of impacts/blunt-hit (cream blob, inked wedges) and
// impacts/hit-spark (cross flare and sparks along the blow), sped up into a dry 0.3 s hit.
import { clamp } from "../../core/index.js";
import * as blunt from "../impacts/blunt-hit.js";
import * as spark from "../impacts/hit-spark.js";
import { layer } from "./_mix.js";

export const meta = {
  name: "Claquetazo impact",
  desc: "A dry clapperboard hit: a white-hot cream blob with inked hit wedges and a cross flare along the blow, sparks and a puff of grey dust, all in 0.3 s.",
  tags: ["tdc", "impact", "hit", "one-shot"],
  size: [128, 80],
  origin: [52, 64],
  duration: 0.3,
  loop: false,
  fps: 30,
  bg: "dark",
  options: {
    palette: ["#16121a", "#3e3436", "#7a6a62", "#b8a690", "#e6d6bc", "#fff4e0", "#ffffff"],
    sparks: ["#3a2a22", "#8a6a50", "#c8a07a", "#ecd0a8", "#fff0d4", "#fffaf0", "#ffffff"],
    warm: "#ffe6c4", dust: "#6c6670", clod: "#5a5250",
    height: 20, bluntSpeed: 1.8, sparkSpeed: 1.4, seed: 9,
  },
};

export function create(o) {
  const A = layer(blunt, { palette: o.palette, warm: o.warm, dust: o.dust, clod: o.clod, height: o.height, seed: o.seed }, { speed: o.bluntSpeed });
  const B = layer(spark, { palette: o.sparks, height: o.height, seed: o.seed + 1 }, { speed: o.sparkSpeed });
  let t = 0;
  const fade = (p) => (p.fade = clamp(1 - (t - meta.duration * 0.45) / (meta.duration * 0.55)));
  return {
    update(dt) { t += dt; A.update(dt); B.update(dt); },
    drawBack(p) { fade(p); A.drawBack(p); B.drawBack(p); },
    draw(p) { fade(p); A.draw(p); B.draw(p); },
  };
}
