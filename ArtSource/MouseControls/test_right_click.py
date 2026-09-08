import unreal as U,json,pathlib,traceback
w=U.EditorLevelLibrary.get_game_world();p=U.GameplayStatics.get_player_character(w,0);pc=U.GameplayStatics.get_player_controller(w,0);m=U.GameplayStatics.get_all_actors_of_class(w,U.FMFrostMonster)[0]
m.set_editor_property('bCombatEnabled',False);m.set_editor_property('frozen_remaining',60.);m.character_movement.disable_movement();m.set_actor_location(U.Vector(450,0,122.15),False,False);p.set_actor_location(U.Vector(0,0,96.15),False,False);p.set_actor_rotation(U.Rotator(),False);pc.set_control_rotation(U.Rotator(pitch=-20,yaw=0));p.get_editor_property('CameraBoom').set_editor_property('enable_camera_lag',False)
out=pathlib.Path('F:/game/MythicVFXLab/Saved/VFXValidation/MouseControls/right_click.json');r={'ok':False,'checks':[]}
def ck(n,v,d=None):
    assert v,n+' '+str(d)
    r['checks'].append({'name':n,'passed':True,'detail':d});out.write_text(json.dumps(r,indent=2))
def key(k,v):pc.simulate_key(k,v)
def steps():
    yield .8
    s=U.GameplayStatics.project_world_to_screen(pc,m.get_actor_location(),True);pc.set_mouse_location(round(s.x),round(s.y));yield .2
    key('RightMouseButton',True);yield .15;key('RightMouseButton',False);yield .2
    ck('Right click selects monster and enables autoattack',p.target==m and p.get_editor_property('bAutoAttacking'))
    ck('Autoattack does not strike outside melee range',m.health==100,m.health)
    pc.set_mouse_location(20,220);yield .2;key('RightMouseButton',True);yield .15;key('RightMouseButton',False);yield .2
    ck('Empty right click preserves target',p.target==m)
    m.set_actor_location(U.Vector(185,0,122.15),False,False);yield .25
    ck('Right-click autoattack strikes in melee range',m.health==94,m.health)
    yield .5;ck('Autoattack obeys swing interval',m.health==94,m.health)
    key('Escape',True);key('Escape',False);yield .2
    ck('Escape stops autoattack and clears target',not p.target and not p.get_editor_property('bAutoAttacking'))
    r['ok']=True;out.write_text(json.dumps(r,indent=2));print('RIGHT_CLICK_ALL_PASS')
seq=steps();ready=0.
def tick(dt):
    global ready
    try:
        now=U.GameplayStatics.get_time_seconds(w)
        if now<ready:return
        ready=now+next(seq)
    except StopIteration:U.unregister_slate_post_tick_callback(handle)
    except Exception:
        r['error']=traceback.format_exc();out.write_text(json.dumps(r,indent=2));U.unregister_slate_post_tick_callback(handle);key('RightMouseButton',False);print('RIGHT_CLICK_FAILED',r['error'])
handle=U.register_slate_post_tick_callback(tick)
