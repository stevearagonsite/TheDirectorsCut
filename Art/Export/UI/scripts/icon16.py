# Version simplificada pixel-aligned del icono para 16 px (y 32 px), coherente con icon_g2.
from PIL import Image, ImageDraw
def draw(n):
    S=16; N=n*S; u=N/16  # u = 1 px de la grilla de 16
    BG=(14,10,30); Y=(250,196,72); G=(108,112,134); GD=(76,78,98); K=(12,12,16); Wt=(240,240,236)
    im=Image.new('RGB',(N,N),BG); d=ImageDraw.Draw(im)
    def R(x0,y0,x1,y1,c): d.rectangle([x0*u,y0*u,x1*u-1,y1*u-1],fill=c)
    # tablero (con contorno amarillo)
    R(1,8,15,16,Y); R(2,9,14,15,G); R(2,14,14,15,GD)
    # palo de abajo: contorno + franjas de 2px
    R(1,5,15,8,Y); R(2,6,14,8,K)
    for x in (3,7,11): d.polygon([(x*u,8*u),((x+1)*u,6*u),((x+3)*u,6*u),((x+2)*u,8*u)],fill=Wt)
    # palo de arriba, abierto en angulo (bisagra a la izquierda)
    import math
    a=math.radians(-14); cx,cy=1.6*u,5.0*u
    def rot(px,py):
        px-=cx; py-=cy
        return (cx+px*math.cos(a)-py*math.sin(a), cy+px*math.sin(a)+py*math.cos(a))
    L=12.3*u; h=1.9*u; o=0.85*u
    d.polygon([rot(cx-o,cy-h-o),rot(cx+L+o,cy-h-o),rot(cx+L+o,cy+o),rot(cx-o,cy+o)],fill=Y)
    d.polygon([rot(cx,cy-h),rot(cx+L,cy-h),rot(cx+L,cy),rot(cx,cy)],fill=K)
    for s in (1.3,5.0,8.7):
        x=cx+s*u
        d.polygon([rot(x,cy),rot(x+u,cy-h),rot(x+3*u,cy-h),rot(x+2*u,cy)],fill=Wt)
    return im.resize((n,n),Image.LANCZOS)
for n in (16,32):
    draw(n).save(f'icon_hand_{n}.png')
