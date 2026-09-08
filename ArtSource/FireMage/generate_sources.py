"""Deterministic, original auxiliary fire textures and low-poly flame shells."""
from pathlib import Path
import numpy as np
from PIL import Image
import json, math, shutil

ROOT=Path(__file__).parent
for d in ('Textures','Meshes','Modules'): (ROOT/d).mkdir(parents=True,exist_ok=True)
hero=Path('C:/Users/Freebooz/.codex/generated_images/01a079ed-e277-76b0-8d08-46cbd2e5861f/exec-4150ac08-e8ac-479b-a77a-d235bf948e5a.png')
if hero.exists():shutil.copy2(hero,ROOT/'Textures/T_PY_HeroFlame.png')
assert (ROOT/'Textures/T_PY_HeroFlame.png').exists(),'Keep the original hero flame source texture with this project'
N=512
y,x=np.mgrid[0:N,0:N].astype(float)/(N-1)
u=x*2-1;v=y*2-1;r=np.sqrt(u*u+v*v);a=np.arctan2(v,u)
def save(name,data):
    Image.fromarray(np.uint8(np.clip(data,0,1)*255)).save(ROOT/'Textures'/f'{name}.png')
def turbulence(x,y):
    z=np.zeros_like(x)
    for k in range(1,6):
        f=2**k
        z+=np.sin(x*f*6.283+np.sin(y*f*4.21+k)*1.8+k)*np.cos(y*f*6.283+k*.76)/f
    return np.clip(.5+z*.48,0,1)
n=turbulence(x,y)
save('T_PY_Turbulence',np.stack([n,turbulence(x+.17,y+.23),turbulence(x+.41,y+.11)],-1))
# Ribbon UV: x wraps around the flame shell, y moves from base to tip.
streak=.5+.27*np.sin(x*6.283*9+np.sin(y*14)*1.8)+.18*np.sin(x*6.283*23-y*11)+.13*(n-.5)
tip=np.clip((1-y-streak*.55)*4,0,1)
ribbon=np.clip(streak*1.3,0,1)**2*tip*np.clip(y*12,0,1)
save('T_PY_FlameRibbon',ribbon)
rr=r+.018*np.sin(a*19+r*55)+.012*np.sin(a*37-r*29)
ring=np.exp(-((rr-.77)/.014)**2)*(.5+.5*n)+.36*np.exp(-((rr-.72)/.03)**2)
save('T_PY_ShockRing',ring*np.clip((1-r)*20,0,1))
vein=np.exp(-(np.sin(a*9+r*11+np.sin(a*5)*.7)/.09)**2)*np.clip((.85-r)*3,0,1)*np.clip(r*9,0,1)
scorch=np.clip((.8-r)*4,0,1)*(.12+.25*n)+vein*.7
save('T_PY_Emberbed',scorch)
spark=np.exp(-(u*u*20+v*v*150))+.5*np.exp(-(u*u*180+v*v*25))
save('T_PY_Ember',spark)
smoke=np.clip((1-r)*2,0,1)**2*np.clip((n-.18)*1.6,0,1)
save('T_PY_Smoke',smoke)

def mesh(name,rows,cols,fn):
    verts=[];uv=[];faces=[]
    for j in range(rows+1):
        for i in range(cols+1):
            verts.append(fn(i/cols,j/rows));uv.append((i/cols,1-j/rows))
    for j in range(rows):
        for i in range(cols):
            k=j*(cols+1)+i+1
            faces.extend([(k,k+1,k+cols+2),(k,k+cols+2,k+cols+1)])
    s=['# Original FireMage mesh; Unreal centimeters; no collision']
    s += [f'v {p[0]:.6f} {-p[1]:.6f} {p[2]:.6f}' for p in verts]
    s += [f'vt {p[0]:.6f} {p[1]:.6f}' for p in uv]
    s += ['f '+' '.join(f'{i}/{i}' for i in f) for f in faces]
    (ROOT/'Meshes'/f'{name}.obj').write_text('\n'.join(s))
    return {'name':name,'triangles':len(faces)}
# +X forward projectile: rear end fans into twisted, tapering fire ribbons.
def shell(s,t):
    ang=s*math.tau+t*1.9
    radius=(8+22*math.sin(t*math.pi)**.65)*(1+.10*math.sin(ang*5+t*8))
    return (38-t*190,math.cos(ang)*radius,math.sin(ang)*radius)
def crown(s,t):
    ang=s*math.tau+t*.65
    rad=170+65*t+12*math.sin(ang*7+t*5)
    return (math.cos(ang)*rad,math.sin(ang)*rad,t*(145+30*math.sin(ang*9)))
report=[mesh('SM_PY_FireballShell',12,24,shell),mesh('SM_PY_FlameCrown',8,40,crown)]
(ROOT/'source_manifest.json').write_text(json.dumps({'textures':7,'meshes':report,'hero_source':'Original imagegen luminance/alpha flame; procedural masks generated from equations, seedless deterministic.'},indent=2))
print(json.dumps(report))
