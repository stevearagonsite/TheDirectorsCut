# Uso: python3 title.py in.png out.png W H cx cy title_w [top_shade 0..1] [crop_y_frac]
# Escala/recorta la imagen a WxH (cover) y dibuja el titulo estilo cartel de los 40.
import sys
from PIL import Image, ImageDraw, ImageFont, ImageFilter

src, out, W, H, cx, cy, tw = sys.argv[1:8]
W, H = int(W), int(H)
cx, cy, tw = float(cx), float(cy), float(tw)  # fracciones del lienzo
shade = float(sys.argv[8]) if len(sys.argv) > 8 else 0.55
cyf = float(sys.argv[9]) if len(sys.argv) > 9 else 0.5

FONT = '/System/Library/Fonts/Supplemental/Futura.ttc'
S = 3  # supersampling
img = Image.open(src).convert('RGB')
r = max(W / img.width, H / img.height)
img = img.resize((round(img.width * r), round(img.height * r)), Image.LANCZOS)
x0 = (img.width - W) // 2
y0 = round((img.height - H) * cyf)
img = img.crop((x0, y0, x0 + W, y0 + H)).resize((W * S, H * S), Image.LANCZOS)
WS, HS = W * S, H * S

# velo oscuro detras del titulo (vertical, centrado en cy)
veil = Image.new('L', (WS, HS), 0)
vd = ImageDraw.Draw(veil)
band = HS * 0.30
for y in range(HS):
    d = abs(y - cy * HS) / band
    vd.line([(0, y), (WS, y)], fill=int(255 * shade * max(0, 1 - d * d)))
veil = veil.filter(ImageFilter.GaussianBlur(20 * S))
img = Image.composite(Image.new('RGB', (WS, HS), (10, 8, 24)), img, veil)

big = ImageFont.truetype(FONT, 100, index=4)
main = "DIRECTOR'S CUT"
bw = big.getlength(main)
size = int(100 * tw * WS / bw)
big = ImageFont.truetype(FONT, size, index=4)
small = ImageFont.truetype(FONT, int(size * 0.38), index=2)

mask = Image.new('L', (WS, HS), 0)
md = ImageDraw.Draw(mask)
bb = big.getbbox(main)
bh = bb[3] - bb[1]
sb = small.getbbox('THE')
sh = sb[3] - sb[1]
gap = int(size * 0.12)
total = sh + gap + bh
top = cy * HS - total / 2
bx = cx * WS - big.getlength(main) / 2
md.text((bx, top + sh + gap - bb[1]), main, font=big, fill=255)
# "THE" chico con filetes a los lados
sw = small.getlength('THE')
sx = cx * WS - sw / 2
md.text((sx, top - sb[1]), 'THE', font=small, fill=255)
ly = top + sh / 2
lw = max(2, int(size * 0.045))
md.line([(bx + size * 0.1, ly), (sx - size * 0.25, ly)], fill=255, width=lw)
md.line([(sx + sw + size * 0.25, ly), (bx + big.getlength(main) - size * 0.1, ly)], fill=255, width=lw)

# sombra
shadow = mask.filter(ImageFilter.MaxFilter(1 + 2 * (max(3, size // 22) // 2) * 1 | 1))
off = max(2, size // 18)
sh_img = Image.new('L', (WS, HS), 0)
sh_img.paste(shadow, (off, off))
sh_img = sh_img.filter(ImageFilter.GaussianBlur(size / 25))
img = Image.composite(Image.new('RGB', (WS, HS), (8, 4, 20)), img, sh_img)
# contorno
k = max(3, size // 14) | 1
outline = mask.filter(ImageFilter.MaxFilter(k))
img = Image.composite(Image.new('RGB', (WS, HS), (22, 12, 40)), img, outline)
# relleno con degrade amarillo (luz)
grad = Image.new('RGB', (WS, HS))
gd = ImageDraw.Draw(grad)
g0, g1 = top, top + total
for y in range(HS):
    t = min(1, max(0, (y - g0) / max(1, g1 - g0)))
    c0, c1 = (255, 240, 170), (236, 160, 30)
    gd.line([(0, y), (WS, y)], fill=tuple(int(a + (b - a) * t) for a, b in zip(c0, c1)))
img = Image.composite(grad, img, mask)
# halo calido suave
glow = mask.filter(ImageFilter.GaussianBlur(size / 6)).point(lambda v: int(v * 0.35))
img = Image.composite(Image.new('RGB', (WS, HS), (255, 200, 80)), img, Image.eval(glow, lambda v: v))
img = Image.composite(grad, img, mask)

img.resize((W, H), Image.LANCZOS).save(out)
print('saved', out, W, H)
