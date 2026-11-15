from PIL import Image, ImageDraw, ImageFont, ImageFilter
import numpy as np
W,H=1920,1080
def cover(p):
    im=Image.open(p).convert('RGB'); r=max(W/im.width,H/im.height)
    im=im.resize((round(im.width*r),round(im.height*r)),Image.LANCZOS)
    x0=(im.width-W)//2; y0=(im.height-H)//2
    return im.crop((x0,y0,x0+W,y0+H))
# --- title bg: oscurecer suave la franja izquierda (menu) ---
bg=cover('title_g1.png')
a=np.asarray(bg).astype(np.float32)
x=np.linspace(0,1,W)[None,:,None]
k=np.clip((0.42-x)/0.42,0,1)**1.5*0.45   # hasta 45% mas oscuro en el borde izq
dark=np.array([10,8,24],np.float32)
a=a*(1-k)+dark*k
Image.fromarray(a.clip(0,255).astype(np.uint8)).save('../title_bg_1920x1080.png')
# --- loading ---
ld=cover('loading_g1.png')
S=3
big=ld.resize((W*S,H*S),Image.LANCZOS)
f=ImageFont.truetype('/System/Library/Fonts/Supplemental/Chalkduster.ttf',100*S)
txt='TAKE 1'
tw=f.getlength(txt); size=int(100*S*440*S/tw)
f=ImageFont.truetype('/System/Library/Fonts/Supplemental/Chalkduster.ttf',size)
m=Image.new('L',big.size,0); d=ImageDraw.Draw(m)
bb=d.textbbox((0,0),txt,font=f)
cx,cy=967*S,680*S
d.text((cx-(bb[0]+bb[2])/2, cy-(bb[1]+bb[3])/2),txt,font=f,fill=255)
glow=m.filter(ImageFilter.GaussianBlur(6*S)).point(lambda v:int(v*0.25))
big=Image.composite(Image.new('RGB',big.size,(235,225,200)),big,glow)
big=Image.composite(Image.new('RGB',big.size,(236,233,224)),big,m.point(lambda v:int(v*0.92)))
big.resize((W,H),Image.LANCZOS).save('../loading_1920x1080.png')
print('ok')
