import unreal as U,json,pathlib,math,traceback

for folder in ['FireMage','ArcaneMage','MouseControls']:
 (pathlib.Path(U.Paths.project_saved_dir())/'VFXValidation'/folder).mkdir(parents=True,exist_ok=True)
w=U.EditorLevelLibrary.get_game_world();p=U.GameplayStatics.get_player_character(w,0)
for m in U.GameplayStatics.get_all_actors_of_class(w,U.FMFrostMonster):m.set_editor_property('bCombatEnabled',False)
out=(pathlib.Path(U.Paths.project_saved_dir()) / 'VFXValidation/ArcaneMage/vfx_budgets.json');report={'ok':False,'samples':[]}
specs=[('Fireball','/ModularGameVFX/Effects/Fire/Niagara/Systems/NS_Fireball_Projectile',44),('Charge','/ModularGameVFX/Effects/Fire/Niagara/Systems/NS_Fireball_CastCharge',28),('FireImpact','/ModularGameVFX/Effects/Fire/Niagara/Systems/NS_Fire_ImpactBurst',66),('Flamestrike','/ModularGameVFX/Effects/Fire/Niagara/Systems/NS_Flamestrike_GroundBurst',90),('Telegraph','/ModularGameVFX/Effects/Fire/Niagara/Systems/NS_Flamestrike_GroundWarning',25),('ArcaneMissile','/ModularGameVFX/Effects/Arcane/Niagara/Systems/NS_ArcaneMissiles_Projectile',55),('ArcaneImpact','/ModularGameVFX/Effects/Arcane/Niagara/Systems/NS_ArcaneMissiles_Impact',32),('ArcaneChannel','/ModularGameVFX/Effects/Arcane/Niagara/Systems/NS_ArcaneMissiles_Channel',22)]
comps=[];first_frames={}
for name,path,budget in specs:
    comp=U.NiagaraFunctionLibrary.spawn_system_at_location(w,U.load_asset(path),U.Vector(450,0,70),U.Rotator(),U.Vector(1,1,1),False,True)
    assert comp,name;comps.append(comp)
def sample(index):
    name,path,budget=specs[index];comp=comps[index]
    cache=U.NiagaraSimCacheFunctionLibrary.create_niagara_sim_cache(w)
    cache=U.NiagaraSimCacheFunctionLibrary.capture_niagara_sim_cache_immediate(cache,U.NiagaraSimCacheCreateParameters(),comp,False)
    assert cache and cache.is_cache_valid(),name+' cache invalid'
    counts={};colors={};total=0
    for emitter in cache.get_emitter_names():
        ps=cache.read_position_attribute(emitter_name=emitter,local_space_to_world=False);cs=cache.read_color_attribute(emitter_name=emitter)
        assert all(math.isfinite(v) for p in ps for v in [p.x,p.y,p.z]),name+' nonfinite position'
        counts[str(emitter)]=len(ps);total+=len(ps)
        colors[str(emitter)]={'mean_alpha':sum(c.a for c in cs)/max(1,len(cs)), 'unique_positions':len(set((round(p.x,2),round(p.y,2),round(p.z,2)) for p in ps))}
        if len(ps)>1 and str(emitter)!='SilkenTrail':assert colors[str(emitter)]['unique_positions']>1,name+'/'+str(emitter)+' particles collapsed to one position'
        if str(emitter) in ['FlameHeart','FlameShell','HandFlame','RollingFlames','ConvectionFlames','DescendingFlare']:
            fs=list(cache.read_float_attribute('SubImageIndex',emitter))
            assert len(fs)==len(ps) and all(0<=f<16 for f in fs),name+' invalid flipbook frames '+str(fs)
            assert any(abs(f-round(f))>.001 for f in fs),name+' frame blending has no fractional indices'
            if len(fs)>1:assert len(set(round(f,3) for f in fs))>1,name+' flipbook phases are identical'
            colors[str(emitter)]['flipbook_frames']=fs
            token=name+'/'+str(emitter)
            if token in first_frames:
                assert fs!=first_frames[token],token+' flipbook did not advance'
                report['flipbook_advanced']=True
            else:first_frames[token]=fs
    report['samples'].append({'system':name,'total':total,'budget':budget,'emitters':counts,'colors':colors})
    assert total<=budget,name+' particle budget exceeded: '+str(total)
    if name!='ArcaneMissile':assert total==budget,name+' missing particles: '+str(total)
    out.write_text(json.dumps(report,indent=2))
def steps():
    yield .3
    for i in range(len(specs)):sample(i)
    yield .5
    sample(5)
    sample(0)
    yield 6.2
    active=[specs[i][0] for i,c in enumerate(comps) if c.is_active()]
    report['active_after_7_seconds']=active
    assert not active,'Systems failed to expire: '+str(active)
    report['ok']=True;out.write_text(json.dumps(report,indent=2));print('SPELL_VFX_BUDGETS_ALL_PASS')
seq=steps();ready=0.
def tick(dt):
    global ready
    try:
        now=U.GameplayStatics.get_time_seconds(w)
        if now<ready:return
        ready=now+next(seq)
    except StopIteration:U.unregister_slate_post_tick_callback(handle)
    except Exception:
        report['error']=traceback.format_exc();out.write_text(json.dumps(report,indent=2));U.unregister_slate_post_tick_callback(handle);print('SPELL_VFX_BUDGETS_FAILED',report['error'])
handle=U.register_slate_post_tick_callback(tick)
