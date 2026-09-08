"""Summarize paired editor CSV captures; these are whole-frame observations."""
import csv,json,pathlib,sys,statistics,math
root=pathlib.Path('F:/game/MythicVFXLab/Saved/VFXValidation/ArcaneMage')
metrics=['FrameTime','GPUTime','GameThreadTime','RenderThreadTime','RHI/DrawCalls','Ticks/FNiagaraWorldManagerTickFunction','Exclusive/RenderThread/Niagara']
def analyze(file):
    with open(file,encoding='utf-8-sig') as f:rows=list(csv.DictReader(f))
    valid=[];elapsed=0.
    for row in rows:
        try:dt=float(row.get('FrameTime',0))/1000
        except (ValueError,TypeError):continue
        if dt<=0 or not math.isfinite(dt):continue
        elapsed+=dt;valid.append((elapsed,row))
    selected=[row for t,row in valid if 3<=t<=elapsed-1]
    summary={}
    for metric in metrics:
        vs=[]
        for row in selected:
            try:v=float(row[metric])
            except (ValueError,KeyError,TypeError):continue
            if math.isfinite(v):vs.append(v)
        if vs:
            vs.sort();summary[metric]={'mean':round(statistics.fmean(vs),4),'p95':round(vs[min(len(vs)-1,math.ceil(len(vs)*.95)-1)],4),'max':round(vs[-1],4)}
    return {'file':str(file),'duration_seconds':elapsed,'samples':len(selected),'trim':'first 3 seconds and last 1 second removed','metrics':summary}
r={'gpu':'NVIDIA GeForce RTX 4060','scope':'UE 5.8 Development Editor PIE, whole scene, no image capture while measuring. Paired observations are not isolated VFX GPU cost.','baseline':analyze(sys.argv[1]),'effects':analyze(sys.argv[2])}
for kind in ['baseline','effects']:r[kind]['setup']=json.loads((root/f'benchmark_{kind}_setup.json').read_text())
r['mean_differences']={k:round(v['mean']-r['baseline']['metrics'][k]['mean'],4) for k,v in r['effects']['metrics'].items() if k in r['baseline']['metrics']}
(root/'performance_comparison.json').write_text(json.dumps(r,indent=2));print(json.dumps(r,indent=2))
