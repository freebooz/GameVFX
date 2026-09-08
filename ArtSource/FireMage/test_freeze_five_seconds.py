import unreal as U,json,pathlib,traceback
w=U.EditorLevelLibrary.get_game_world();p=U.GameplayStatics.get_player_character(w,0);m=U.GameplayStatics.get_all_actors_of_class(w,U.FMFrostMonster)[0]
out=pathlib.Path('F:/game/MythicVFXLab/Saved/VFXValidation/FireMage/freeze_five_seconds.json');report={'ok':False,'checks':[]}
def check(name,value,detail=None):
    assert value,name+' '+str(detail)
    report['checks'].append({'name':name,'passed':True,'detail':detail});out.write_text(json.dumps(report,indent=2));print('FREEZE_VERIFY',name)
def steps():
    p.set_actor_location(U.Vector(0,0,96.15),False,False);m.set_actor_location(U.Vector(350,0,122.15),False,False);m.set_editor_property('health',100.);m.set_editor_property('cast_remaining',2.)
    p.cast_frost_nova();start=m.get_actor_location()
    check('Nova applies exactly five seconds immediately',m.frozen_remaining==5,m.frozen_remaining)
    check('Nova cancels attack and disables movement',m.cast_remaining==0 and m.character_movement.movement_mode==U.MovementMode.MOVE_NONE)
    yield 4.55
    delta=m.get_actor_location()-start
    check('Monster remains immobile beyond four seconds',m.frozen_remaining>.1 and delta.length()<.1,[m.frozen_remaining,delta.length()])
    check('Frozen monster cannot resume casting early',m.cast_remaining==0)
    yield .8
    check('Movement restores after five seconds',m.frozen_remaining==0 and m.character_movement.movement_mode==U.MovementMode.MOVE_WALKING)
    check('Monster resumes retaliation after thaw',m.get_editor_property('bChasing') and m.cast_remaining>0,m.cast_remaining)
    report['ok']=True;out.write_text(json.dumps(report,indent=2));print('FREEZE_FIVE_SECONDS_ALL_PASS')
seq=steps();ready=0.
def tick(dt):
    global ready
    try:
        now=U.GameplayStatics.get_time_seconds(w)
        if now<ready:return
        ready=now+next(seq)
    except StopIteration:U.unregister_slate_post_tick_callback(handle)
    except Exception:
        report['error']=traceback.format_exc();out.write_text(json.dumps(report,indent=2));U.unregister_slate_post_tick_callback(handle);print('FREEZE_FIVE_SECONDS_FAILED',report['error'])
handle=U.register_slate_post_tick_callback(tick)
