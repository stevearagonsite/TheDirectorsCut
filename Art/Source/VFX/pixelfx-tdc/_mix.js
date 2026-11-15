// Remix helper for The Director's Cut: run existing effects as layers of a new one, with their own options,
// a time scale, a delay, a shift and optional time reversal (pure effects only).
import { resolveOptions } from "../../core/fx.js";

export function layer(mod, o = {}, { speed = 1, delay = 0, dx = 0, dy = 0, reverse = false, loop = false } = {}) {
  const meta = mod.meta, D = meta.duration, opts = resolveOptions(meta, o);
  const impl = mod.create ? mod.create(opts) : mod;
  let t = -delay * speed;
  const local = (tt) => {
    if (loop || meta.loop) {
      const u = ((tt % D) + D) % D;
      return reverse ? (D - u) % D : u;
    }
    return reverse ? D - Math.min(tt, D) : Math.min(tt, D);
  };
  const paint = (p, fn, tt) => {
    if (!fn || tt < 0) return;
    const ox = p.tx, oy = p.ty;
    p.tx += dx * p.fx;
    p.ty += dy;
    fn.call(impl, p, local(tt), opts);
    p.tx = ox;
    p.ty = oy;
  };
  return {
    opts,
    get t() { return t; },
    update(dt) {
      const before = t;
      t += dt * speed;
      if (!impl.update || t <= 0) return;
      const step = t - Math.max(0, before);
      const n = Math.max(1, Math.ceil(step * 120 - 1e-9));
      for (let i = 0; i < n; i++) impl.update(step / n, opts);
    },
    // the outer time is passed in for pure effects so loops stay seamless
    drawBack(p, T) { paint(p, impl.drawBack, T === undefined ? t : (T - delay) * speed); },
    draw(p, T) { paint(p, impl.draw, T === undefined ? t : (T - delay) * speed); },
  };
}
