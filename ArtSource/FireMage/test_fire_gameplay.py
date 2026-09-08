"""World-time integration test; runs through the real controller input queue."""
import unreal as U,json,pathlib,traceback
w=U.EditorLevelLibrary.get_game_world();p=U.GameplayStatics.get_player_character(w,0);pc=U.GameplayStatics.get_player_controller(w,0)
m=U.GameplayStatics.get_all_actors_of_class(w,U.FMFrostMonster)[0]
report={'ok':False,'checks':[]};out=pathlib.Path('F:/game/MythicVFXLab/Saved/VFXValidation/FireMage/gameplay_test.json')
def check(name,condition,detail=None):
    assert condition,name+' '+str(detail)
    report['checks'].append({'name':name,'passed':True,'detail':detail});out.write_text(json.dumps(report,indent=2));print('FIRE_VERIFY',name)
def key(k,pressed=True):pc.simulate_key(k,pressed)
def place(x=700,y=0):
    m.set_actor_location(U.Vector(x,y,122.15),False,False);m.character_movement.disable_movement()
    m.set_editor_property('health',100.);m.set_editor_property('bCombatEnabled',False);m.set_editor_property('slow_remaining',0.);m.set_editor_property('frozen_remaining',0.)
def steps():
    check('Fire APIs loaded',hasattr(p,'cast_fireball') and hasattr(p,'cast_fire_blast') and hasattr(p,'cast_flamestrike') and hasattr(m,'receive_fire'))
    p.set_actor_location(U.Vector(0,0,96.15),False,False);p.set_actor_rotation(U.Rotator(),False);p.set_editor_property('health',100.);place();p.select_next_target()
    key('Five');yield .1;key('Five',False)
    check('5 starts 1.8 second fireball cast',p.casting_spell==5 and abs(p.cast_duration-1.8)<.001 and p.cast_remaining>0,[p.cast_duration,p.cast_remaining])
    key('W');yield .2;key('W',False)
    check('Movement interrupts fireball without cooldown',p.casting_spell==0 and p.fireball_cooldown==0)
    p.set_actor_location(U.Vector(0,0,96.15),False,False);yield .2
    key('Five');yield .1;key('Five',False);yield 2.7
    check('Fireball damages target',m.health==70,m.health)
    check('Fireball does not chill or freeze',m.slow_remaining==0 and m.frozen_remaining==0)
    place();key('W');yield .25
    key('Six');yield .05;key('Six',False)
    check('6 is instant and damages for 22',m.health==78 and p.casting_spell==0,[m.health,p.casting_spell])
    check('Instant fire blast preserves movement',p.get_velocity().length()>350,p.get_velocity().length())
    hp=m.health;key('Six');yield .06;key('Six',False)
    check('Fire blast cooldown blocks repeats',m.health==hp and p.fire_blast_cooldown>7)
    key('W',False);yield .15;p.set_actor_location(U.Vector(0,0,96.15),False,False);place();p.select_next_target()
    p.cast_fireball();p.set_editor_property('fire_blast_cooldown',0.);p.cast_fire_blast()
    check('Fire Blast can be woven into a fireball cast',p.casting_spell==5 and m.health==78)
    p.cancel_cast();place()
    key('Seven');yield .1;key('Seven',False)
    check('7 starts flamestrike cast',p.casting_spell==7)
    yield 2.2
    check('Flamestrike initial damage',m.health<=72 and m.health>=64,m.health)
    yield 1.1
    check('Flamestrike burns within radius',m.health<72,m.health)
    m.set_actor_location(U.Vector(1800,0,122.15),False,False);hp=m.health;yield 1.1
    check('Leaving flamestrike stops damage',m.health==hp,[hp,m.health])
    check('Fire area does not slow target',m.slow_remaining==0 and m.frozen_remaining==0)
    yield 4.
    areas=U.GameplayStatics.get_all_actors_of_class(w,U.FMFireArea)
    check('Flamestrike actor expires',len(areas)==0,len(areas))
    place(3000);m.set_editor_property('bCombatEnabled',True);m.set_editor_property('attack_cooldown',99.);m.receive_fire(1.)
    check('Fire damage arms retaliation',m.get_editor_property('bChasing') and m.attack_cooldown<.151)
    m.set_editor_property('bCombatEnabled',False);place(320)
    p.set_editor_property('nova_cooldown',0.);p.cast_frost_nova()
    check('Instant frost nova still roots',m.frozen_remaining==5 and m.health==84)
    p.set_editor_property('health',60.);p.cast_healing_bloom();yield .7
    check('Existing healing still functions',p.health>60,p.health)
    report['ok']=True;out.write_text(json.dumps(report,indent=2));print('FIRE_GAMEPLAY_ALL_PASS')
seq=steps();ready=0.
def tick(dt):
    global ready
    try:
        now=U.GameplayStatics.get_time_seconds(w)
        if now<ready:return
        ready=now+next(seq)
    except StopIteration:U.unregister_slate_post_tick_callback(handle)
    except Exception:
        report['error']=traceback.format_exc();out.write_text(json.dumps(report,indent=2));U.unregister_slate_post_tick_callback(handle);key('W',False);print('FIRE_GAMEPLAY_FAILED',report['error'])
handle=U.register_slate_post_tick_callback(tick)

