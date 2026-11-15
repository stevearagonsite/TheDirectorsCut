// The Director's Cut: shared pieces for the light/dark platform transitions (appear / vanish).
// A "piece" is a platform made of light: a translucent bar with a bright rim, W x H px, centered on (0, 0).
// Light pieces are warm yellow, dark pieces violet. These helpers draw the bar and run the small particle
// systems the four effects share, so every transition reads as the same material.
import { clamp, mix, scale, WHITE } from "../../core/index.js";

export const W = 112, H = 10, X0 = -W / 2, X1 = W / 2, Y0 = -H / 2, Y1 = H / 2;
export const LIGHT = ["#2a1a06", "#5a3a0c", "#9a6614", "#d89a22", "#f6c63c", "#ffe27a", "#fff4c8", "#ffffff"];
export const DARK = ["#0c0716", "#1e1036", "#341a5c", "#56298e", "#7d43c8", "#a875ee", "#d2b2ff", "#f2e8ff"];
export const META = { size: [136, 60], origin: [68, 40], duration: 0.4, loop: false, fps: 25, bg: "dark" };

const isRim = (x, y) => y === Y0 || y === Y1 - 1 || x === X0 || x === X1 - 1;

/**
 * Draw the bar pixel by pixel. look(x, y) returns null (nothing) or { b: 0..1 brightness, a: 0..1 presence }.
 * Rim pixels step down the ramp by brightness (white only when hot), the inside is a truly translucent wash,
 * and every lit pixel adds a little light so the bar glows like the in-game material.
 */
export function drawBar(p, C, look, { glowK = 0.18, fill = 0.32 } = {}) {
  for (let y = Y0; y < Y1; y++) {
    for (let x = X0; x < X1; x++) {
      const s = look(x, y);
      if (!s || s.a <= 0) continue;
      const b = clamp(s.b), a = clamp(s.a);
      if (isRim(x, y)) {
        const top = y === Y0;
        const c = b > 0.85 ? WHITE : b > 0.6 ? C[6] : b > 0.3 ? (top ? C[6] : C[5]) : top ? C[5] : C[4];
        // fade the rim as true translucency, never a dither grid
        if (a >= 0.99) p.px(x, y, c);
        else p.blend(x, y, c, a);
        p.light(x, y, c, glowK * a * (0.5 + b));
      } else {
        // inside: a lit wash, brighter near the top (light from above), whiter where it's hot
        const v = 1 - (y - Y0) / H;
        const c = b > 0.7 ? mix(C[5], WHITE, (b - 0.7) / 0.3) : mix(C[3], C[5], b);
        p.blend(x, y, c, a * (fill * (0.55 + 0.45 * v) + 0.45 * b * b));
        if (b > 0.2) p.light(x, y, C[5], glowK * 0.6 * a * b);
      }
    }
  }
}

/**
 * Tiny particle list for motes, dust and smoke puffs: { x, y, vx, vy, ax, ay, drag, life, max, kind, size, seed }.
 * Every particle is spawned with life clipped to the effect's end so the last frame is empty.
 */
export class Motes {
  constructor() { this.list = []; this.t = 0; }
  add(q) { this.list.push({ ax: 0, ay: 0, drag: 0, size: 1, seed: 0, ...q, max: q.life }); }
  update(dt) {
    this.t += dt;
    for (let i = this.list.length - 1; i >= 0; i--) {
      const q = this.list[i];
      if ((q.life -= dt) <= 0) { this.list.splice(i, 1); continue; }
      q.vx += q.ax * dt; q.vy += q.ay * dt;
      q.vx -= q.vx * q.drag * dt; q.vy -= q.vy * q.drag * dt;
      if (q.kind === "smoke") q.vx += Math.sin(this.t * 9 + q.seed) * 30 * dt;
      q.x += q.vx * dt; q.y += q.vy * dt;
    }
  }
  /** kind: "spark" (a hot pixel cooling down the ramp), "dust" (a twinkling speck), "smoke" (soft puff), "drop" (ink drop). */
  draw(p, C, which) {
    for (const q of this.list) {
      if (which && which !== (q.kind === "smoke" ? "back" : "front")) continue;
      const k = q.life / q.max, x = q.x, y = q.y;
      if (q.kind === "spark") {
        const c = k > 0.75 ? WHITE : k > 0.5 ? C[6] : k > 0.25 ? C[5] : C[4];
        if (k < 0.2 && Math.sin(this.t * 60 + q.seed) < 0) continue; // flickers out
        p.px(x, y, c);
        if (q.size > 1 && k > 0.4) p.px(x, y + 1, C[k > 0.7 ? 5 : 4]); // a short tail below while it rises fast
        p.glow(x, y, 2.5 + q.size, C[5], 0.3 * k);
      } else if (q.kind === "dust") {
        const tw = 0.55 + 0.45 * Math.sin(this.t * 38 + q.seed);
        const c = tw * k > 0.6 ? WHITE : tw * k > 0.3 ? C[6] : C[5];
        if (tw * k > 0.12) p.px(x, y, c);
        if (q.size > 1 && tw * k > 0.55) { p.dot(x - 1, y, C[5], 0.7); p.dot(x + 1, y, C[5], 0.7); p.dot(x, y - 1, C[5], 0.7); p.dot(x, y + 1, C[5], 0.7); }
        p.glow(x, y, 3, C[5], 0.28 * k * tw);
      } else if (q.kind === "smoke") {
        // a soft violet puff that swells and thins; a lit cap while young
        const age = 1 - k, r = q.size * (0.6 + age * 0.9);
        const a = Math.min(1, age * 10) * Math.pow(k, 0.8) * 0.75;
        const c = mix(C[5], C[2], Math.min(1, age * 1.3));
        p.disc(x, y, r, c, (d) => a * Math.pow(1 - d, 0.7));
        p.disc(x + r * 0.1, y + r * 0.2, r * 0.6, C[1], (d) => a * 0.5 * (1 - d));
        if (age < 0.5) p.disc(x - r * 0.25, y - r * 0.35, r * 0.45, C[5], (d) => a * 0.6 * (1 - age * 2) * (1 - d));
        p.glow(x, y, r + 2, C[4], 0.14 * k);
      } else if (q.kind === "drop") {
        p.dot(x, y, C[1], Math.min(1, k * 2.5));
        p.dot(x, y - 1, scale(C[4], 0.9), Math.min(1, k * 2.5) * 0.8);
        p.light(x, y - 1, C[4], 0.25 * k);
      }
    }
  }
}
