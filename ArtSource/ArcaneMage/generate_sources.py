"""Original analytic arcane masks and lightweight pointed energy core."""
from pathlib import Path
import numpy as np, math, json
from PIL import Image
ROOT=Path(__file__).parent
for folder in ('Textures','Meshes','Modules'):(ROOT/folder).mkdir(parents=True,exist_ok=True)
N=512;y,x=np.mgrid[0:N,0:N].astype(float)/(N-1);u=x*2-1;v=y*2-1;r=np.sqrt(u*u+v*v);a=np.arctan2(v,u)
def save(name,z):Image.fromarray(np.uint8(np.clip(z,0,1)*255)).save(ROOT/'Textures'/f'{name}.png')
core=np.exp(-(u*u*4+v*v*16))*np.clip((1-r)*5,0,1)
filaments=np.exp(-((r+.013*np.sin(a*13)-.54)/.012)**2)*.3
save('T_AM_Core',core+filaments)
ribbon=np.exp(-(v/.11)**2)*(.70+.16*np.sin(x*math.tau*7))+np.exp(-((v-.08*np.sin(x*math.tau*3))/.017)**2)*.55
ribbon+=.25*np.exp(-((v+.13*np.sin(x*math.tau*2))/.024)**2)
save('T_AM_Ribbon',ribbon*np.clip((1-abs(v))*5,0,1))
ring=np.exp(-((r-.77)/.008)**2)+.5*np.exp(-((r-.65)/.007)**2)
ticks=np.clip((np.cos(a*24)-.85)*12,0,1)*np.clip((r-.67)*40,0,1)*np.clip((.75-r)*40,0,1)
glyph=np.clip((np.cos(a*9+np.sin(r*49))-.8)*6,0,1)*np.clip((r-.49)*30,0,1)*np.clip((.60-r)*30,0,1)
save('T_AM_RunicSeal',ring+ticks+glyph)
spark=np.exp(-(u*u*70+v*v*70))+.4*np.exp(-(u*u*380+v*v*18))+.4*np.exp(-(u*u*18+v*v*380))
save('T_AM_Star',spark)
verts=[];uv=[];faces=[];rows=8;cols=12
for j in range(rows+1):
    t=j/rows;rad=math.sin(t*math.pi)**1.5*8
    for i in range(cols+1):
        ang=i/cols*math.tau+t*1.3;verts.append((38-t*83,math.cos(ang)*rad,math.sin(ang)*rad));uv.append((i/cols,t))
for j in range(rows):
    for i in range(cols):
        k=j*(cols+1)+i+1;faces.extend([(k,k+1,k+cols+2),(k,k+cols+2,k+cols+1)])
lines=['# Original pointed arcane core; 192 triangles']+[f'v {p[0]} {-p[1]} {p[2]}' for p in verts]+[f'vt {p[0]} {p[1]}' for p in uv]+['f '+' '.join(f'{i}/{i}' for i in f) for f in faces]
(ROOT/'Meshes/SM_AM_MissileCore.obj').write_text('\n'.join(lines))
(ROOT/'source_manifest.json').write_text(json.dumps({'textures':4,'core_triangles':len(faces),'method':'Original analytic masks and parametric spindle'},indent=2))
print('ARCANE_SOURCE_READY',len(faces))
