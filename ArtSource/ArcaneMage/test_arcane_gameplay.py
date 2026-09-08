import unreal as U,json,pathlib,traceback
w=U.EditorLevelLibrary.get_game_world();p=U.GameplayStatics.get_player_character(w,0);pc=U.GameplayStatics.get_player_controller(w,0);m=U.GameplayStatics.get_all_actors_of_class(w,U.FMFrostMonster)[0]
out=pathlib.Path('F:/game/MythicVFXLab/Saved/VFXValidation/ArcaneMage');out.mkdir(parents=True,exist_ok=True)
report={'ok':False,'checks':[]}
def check(name,value,detail=None):
    assert value,name+' '+str(detail)
    report['checks'].append({'name':name,'passed':True,'detail':detail});(out/'gameplay_test.json').write_text(json.dumps(report,indent=2));print('ARCANE_VERIFY',name)
def reset(x=1000):
    m.set_editor_property('bCombatEnabled',False);m.character_movement.disable_movement();m.set_editor_property('health',100.);m.set_editor_property('slow_remaining',0.);m.set_editor_property('frozen_remaining',0.);m.set_actor_location(U.Vector(x,0,122.15),False,False)
    p.set_actor_location(U.Vector(0,0,96.15),False,False);p.set_editor_property('health',100.);p.set_editor_property('arcane_cooldown',0.)
def steps():
    check('Arcane channel API available',hasattr(p,'cast_arcane_missiles'))
    reset();p.select_next_target();pc.simulate_key('Eight',True);yield .12;pc.simulate_key('Eight',False)
    check('8 begins a channel and releases first missile',p.casting_spell==8 and p.arcane_shots_fired==1,[p.casting_spell,p.arcane_shots_fired])
    yield 3.5
    check('Exactly five missiles released',p.arcane_shots_fired==5,p.arcane_shots_fired)
    check('Five hits cause 45 damage',m.health==55,m.health)
    check('Channel completes normally',p.casting_spell==0)
    check('Arcane damage does not chill or freeze',m.slow_remaining==0 and m.frozen_remaining==0)
    yield .8
    check('Missile actors clean up after impact',len(U.GameplayStatics.get_all_actors_of_class(w,U.FMArcaneMissile))==0)
    reset();p.cast_arcane_missiles();yield .2;pc.simulate_key('W',True);yield .15;pc.simulate_key('W',False)
    check('Movement interrupts channel',p.casting_spell==0 and p.arcane_shots_fired==1)
    yield 2.
    check('Interrupted channel produces no extra missiles',p.arcane_shots_fired==1 and m.health==91,[p.arcane_shots_fired,m.health])
    reset();p.cast_arcane_missiles();yield .15;pc.simulate_key('Escape',True);yield .1;pc.simulate_key('Escape',False)
    check('Esc interrupts without ending play',p.casting_spell==0 and U.EditorLevelLibrary.get_game_world() is not None)
    check('Interrupt does not refund cooldown',p.arcane_cooldown>3)
    yield 2.
    reset(3000);p.cast_arcane_missiles()
    check('Out of range cast is rejected',p.casting_spell==0 and p.arcane_cooldown==0)
    reset();p.cast_arcane_missiles();yield .15;m.receive_arcane(1000.);yield .2
    check('Lost target stops channel and future missiles',p.casting_spell==0 and p.arcane_shots_fired==1)
    report['ok']=True;(out/'gameplay_test.json').write_text(json.dumps(report,indent=2));print('ARCANE_GAMEPLAY_ALL_PASS')
seq=steps();ready=0.
def tick(dt):
    global ready
    try:
        now=U.GameplayStatics.get_time_seconds(w)
        if now<ready:return
        ready=now+next(seq)
    except StopIteration:U.unregister_slate_post_tick_callback(handle)
    except Exception:
        report['error']=traceback.format_exc();(out/'gameplay_test.json').write_text(json.dumps(report,indent=2));U.unregister_slate_post_tick_callback(handle);print('ARCANE_GAMEPLAY_FAILED',report['error'])
handle=U.register_slate_post_tick_callback(tick)
