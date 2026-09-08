"""Measured input transfer, pointer capture, zoom and hotbar integration in live PIE."""
import unreal as U,json,pathlib,traceback

for folder in ['FireMage','ArcaneMage','MouseControls']:
 (pathlib.Path(U.Paths.project_saved_dir())/'VFXValidation'/folder).mkdir(parents=True,exist_ok=True)
w=U.EditorLevelLibrary.get_game_world();p=U.GameplayStatics.get_player_character(w,0);pc=U.GameplayStatics.get_player_controller(w,0);m=U.GameplayStatics.get_all_actors_of_class(w,U.FMFrostMonster)[0]
m.set_editor_property('bCombatEnabled',False);m.set_editor_property('frozen_remaining',120.);m.character_movement.disable_movement();m.set_actor_location(U.Vector(650,0,122.15),False,False)
boom=p.get_editor_property('CameraBoom');cam=p.get_editor_property('FollowCamera');boom.set_editor_property('enable_camera_lag',False)
p.set_actor_location(U.Vector(0,0,96.15),False,False);p.set_actor_rotation(U.Rotator(),False);pc.set_control_rotation(U.Rotator(pitch=-20,yaw=0));pc.set_editor_property('bSmartCameraFollow',False)
out=(pathlib.Path(U.Paths.project_saved_dir()) / 'VFXValidation/MouseControls/precision.json');report={'ok':False,'checks':[]};held=set()
def ck(name,yes,detail=None):
    assert yes,name+' '+str(detail)
    report['checks'].append({'name':name,'passed':True,'detail':detail});out.write_text(json.dumps(report,indent=2))
def key(k,down):
    pc.simulate_key(k,down)
    if down:held.add(k)
    else:held.discard(k)
def tap(k):key(k,True);key(k,False)
def delta(a,b):return (a-b+180)%360-180
def rotate_sample(fov,spacing):
    cam.set_field_of_view(fov);pc.set_control_rotation(U.Rotator(pitch=-20,yaw=0));pc.set_mouse_location(310,260);yield .15
    state['press']=list(pc.get_mouse_position());key('RightMouseButton',True);yield .15
    for i in range(4):pc.simulate_mouse(100,100);yield spacing
    state['yaw']=pc.get_control_rotation().yaw;state['pitch']=pc.get_control_rotation().pitch
    key('RightMouseButton',False);yield .15
state={}
def steps():
    yield .8
    ck('Yaw and pitch defaults use 180:90',pc.camera_yaw_move_speed==180 and pc.camera_pitch_move_speed==90)
    yield from rotate_sample(65,.025)
    a=state['yaw'];pitch=state['pitch'];ck('Raw horizontal transfer is 0.1 degree per count',abs(a-40)<.05,a);ck('Vertical transfer is half horizontal',abs(pitch)<.05,pitch)
    loc=pc.get_mouse_position();ck('Release restores press position',abs(loc[0]-state['press'][0])<2 and abs(loc[1]-state['press'][1])<2,{'press':state['press'],'release':list(loc)})
    yield from rotate_sample(95,.08)
    ck('Mouse rotation independent of FOV and event spacing',abs(state['yaw']-a)<.05,[a,state['yaw']]);cam.set_field_of_view(65)
    pc.set_editor_property('mouse_sensitivity_multiplier',1.5);yield from rotate_sample(65,.035)
    ck('Sensitivity multiplier scales linearly',abs(state['yaw']-60)<.05,state['yaw']);pc.set_editor_property('mouse_sensitivity_multiplier',1.)
    pc.set_control_rotation(U.Rotator(pitch=-20,yaw=0));p.set_actor_rotation(U.Rotator(),False);pc.set_editor_property('bSmartCameraFollow',True)
    key('LeftMouseButton',True);yield .15
    for i in range(4):pc.simulate_mouse(150,0);yield .05
    key('LeftMouseButton',False);yield .1
    before=pc.get_control_rotation().yaw;key('W',True);yield 1.
    ck('Smart camera returns behind moving character',abs(delta(pc.get_control_rotation().yaw,p.get_actor_rotation().yaw))<2,[before,pc.get_control_rotation().yaw]);key('W',False);yield .25
    # Reverse button release order preserves RMB steering and stops dual-button movement.
    pc.set_mouse_location(310,260);key('LeftMouseButton',True);yield .15;key('RightMouseButton',True);yield .3;key('LeftMouseButton',False);yield .3
    ck('LMB first release preserves RMB mode',pc.get_editor_property('bRightMouseHeld') and not pc.show_mouse_cursor and p.get_velocity().length()<5)
    key('RightMouseButton',False);yield .15
    for i in range(10):tap('MouseScrollUp')
    yield 1.3
    ck('Wheel reaches first person with own mesh hidden',boom.target_arm_length==0 and p.mesh.get_editor_property('owner_no_see'))
    for i in range(10):tap('MouseScrollDown')
    yield 1.3
    ck('Zooming out restores third person mesh',boom.target_arm_length>=690 and not p.mesh.get_editor_property('owner_no_see'))
    width,height=pc.get_viewport_size();cols=4 if width<1280 else 8;rows=2 if cols==4 else 1;cw=min(170,(width-64-(cols-1)*8)/cols);bar=cols*cw+(cols-1)*8
    x=(width-bar)/2+(cw+8)+cw/2;y=height-102-(rows-1)*72+30
    p.set_editor_property('nova_cooldown',0.);pc.set_mouse_location(round(x),round(y));yield .2
    key('LeftMouseButton',True);yield .15;key('LeftMouseButton',False);yield .15
    ck('Mouse activates Frost Nova hotbar button',p.nova_cooldown>5 and not pc.get_editor_property('bOrbiting'),p.nova_cooldown)
    # Captured cursor position over a hotbar card must not steal the second button.
    pc.set_mouse_location(310,260);key('RightMouseButton',True);yield .15;pc.set_mouse_location(round(x),round(y));key('LeftMouseButton',True);yield .25
    ck('Both-button input remains movement during capture',pc.get_editor_property('bLeftMouseHeld') and pc.get_editor_property('bRightMouseHeld') and p.get_velocity().length()>300)
    key('LeftMouseButton',False);key('RightMouseButton',False);yield .2
    report['ok']=True;out.write_text(json.dumps(report,indent=2));print('MOUSE_PRECISION_ALL_PASS')
seq=steps();ready=0.
def tick(dt):
    global ready
    try:
        now=U.GameplayStatics.get_time_seconds(w)
        if now<ready:return
        ready=now+next(seq)
    except StopIteration:U.unregister_slate_post_tick_callback(handle)
    except Exception:
        report['error']=traceback.format_exc();out.write_text(json.dumps(report,indent=2));U.unregister_slate_post_tick_callback(handle)
        for k in list(held):key(k,False)
        print('MOUSE_PRECISION_FAILED',report['error'])
handle=U.register_slate_post_tick_callback(tick)
