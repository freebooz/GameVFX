"""PIE integration checks through the native PlayerInput queue."""
import unreal as U, json, pathlib, traceback

for folder in ['FireMage','ArcaneMage','MouseControls']:
 (pathlib.Path(U.Paths.project_saved_dir())/'VFXValidation'/folder).mkdir(parents=True,exist_ok=True)
mw=U.EditorLevelLibrary.get_game_world()
mp=U.GameplayStatics.get_player_character(mw,0)
mc=U.GameplayStatics.get_player_controller(mw,0)
mt=U.GameplayStatics.get_all_actors_of_class(mw,U.FMFrostMonster)
assert len(mt)==2, 'Requires two runtime monsters'
for i,m in enumerate(mt):
 m.set_editor_property('bCombatEnabled',False);m.set_editor_property('frozen_remaining',120.)
 m.character_movement.disable_movement()
 m.set_actor_location(U.Vector(500,-180 if i==0 else 180,122.15),False,False)
for projectile in U.GameplayStatics.get_all_actors_of_class(mw,U.FMFrostProjectile):projectile.destroy_actor()
mp.set_editor_property('target',None)
boom=mp.get_editor_property('CameraBoom');boom.set_editor_property('enable_camera_lag',False)
report={'ok':False,'checks':[]};held=set()
result_path=(pathlib.Path(U.Paths.project_saved_dir()) / 'VFXValidation/MouseControls/wow_controls_test.json')
def check(name,condition,detail=None):
 assert condition,name+' '+str(detail)
 report['checks'].append({'name':name,'passed':True,'detail':detail})
 result_path.write_text(json.dumps(report,indent=2));print('WOW_VERIFY',name)
def key(name,down):
 mc.simulate_key(name,down)
 if down:held.add(name)
 else:held.discard(name)
def tap(name):key(name,True);key(name,False)
def yd(a,b):return abs((a-b+180)%360-180)
def click(m):
 s=U.GameplayStatics.project_world_to_screen(mc,m.get_actor_location()+U.Vector(0,0,25),True)
 assert s,'Target not visible'
 mc.set_mouse_location(round(s.x),round(s.y));yield .25
 key('LeftMouseButton',True);yield .2
 key('LeftMouseButton',False);yield .3
def reset():
 mp.character_movement.stop_movement_immediately()
 mp.set_actor_location(U.Vector(0,0,96.15),False,False);mp.set_actor_rotation(U.Rotator(),False)
 mc.set_control_rotation(U.Rotator(pitch=-20,yaw=0,roll=0))
reset()
def tests():
 yield 1.
 check('Pointer visible by default',mc.get_editor_property('show_mouse_cursor'))
 moon=next(c for c in mp.get_components_by_class(U.PointLightComponent) if c.get_name()=='CharacterMoonFill')
 check('Character has local moon fill without shadows',moon.intensity>0 and not moon.cast_shadows)
 yield from click(mt[0])
 check('Left click selects first monster',mp.target==mt[0] and mt[0].get_editor_property('bSelected'))
 yield from click(mt[1])
 check('Click switches target and clears old highlight',mp.target==mt[1] and not mt[0].get_editor_property('bSelected'))
 old_cam=mc.get_control_rotation().yaw;old_actor=mp.get_actor_rotation().yaw
 mc.simulate_mouse(100.,0.);yield .25
 check('Unpressed mouse does not rotate view',yd(old_cam,mc.get_control_rotation().yaw)<.1)
 key('LeftMouseButton',True);yield .3
 for _ in range(4):mc.simulate_mouse(180.,0.);yield .12
 check('Left drag rotates only camera',yd(old_cam,mc.get_control_rotation().yaw)>15 and yd(old_actor,mp.get_actor_rotation().yaw)<.1,yd(old_cam,mc.get_control_rotation().yaw))
 key('LeftMouseButton',False);yield .3
 check('Dragging does not change selected target',mp.target==mt[1])
 key('RightMouseButton',True);yield .3
 old_cam=mc.get_control_rotation().yaw
 for _ in range(4):mc.simulate_mouse(180.,0.);yield .12
 check('Right drag steers character with camera',yd(old_cam,mc.get_control_rotation().yaw)>15 and yd(mp.get_actor_rotation().yaw,mc.get_control_rotation().yaw)<.1,yd(old_cam,mc.get_control_rotation().yaw))
 origin=mp.get_actor_location();key('LeftMouseButton',True);yield .65
 check('Holding both buttons moves forward',(mp.get_actor_location()-origin).length()>100)
 check('Moon fill follows moving character',(moon.get_world_location()-mp.get_actor_location()).length()<320)
 key('RightMouseButton',False);yield .4
 check('Releasing one button keeps left camera mode',mc.get_editor_property('bLeftMouseHeld') and not mc.get_editor_property('bRightMouseHeld') and not mc.get_editor_property('show_mouse_cursor'))
 check('Dual-button movement stops after one released',mp.get_velocity().length()<5)
 key('LeftMouseButton',False);yield .25
 check('Releasing both restores pointer',mc.get_editor_property('show_mouse_cursor') and not mc.get_editor_property('bOrbiting'))
 reset();yield .25
 key('S',True);yield .6
 v=mp.get_velocity();anim=mp.current_locomotion.get_name()
 check('S backpedals without turning',v.x<-170 and abs(v.y)<2 and yd(mp.get_actor_rotation().yaw,0)<.1,[v.x,v.y])
 check('Backpedal has half forward speed',abs(v.length()-190)<2,v.length())
 check('Backpedal uses backward walk animation',anim=='MF_Unarmed_Walk_Bwd',anim)
 key('S',False);yield .3
 key('W',True);yield .5
 check('W runs along character heading',mp.get_velocity().x>375,mp.get_velocity().x)
 key('W',False);yield .3
 pos=mp.get_actor_location();key('D',True);yield .5
 check('D strafes right without turning',yd(mp.get_actor_rotation().yaw,0)<.1 and mp.get_actor_location().y-pos.y>100 and abs(mp.get_actor_location().x-pos.x)<2)
 key('D',False);yield .2
 pos=mp.get_actor_location();key('A',True);yield .5
 check('A strafes left without turning',yd(mp.get_actor_rotation().yaw,0)<.1 and pos.y-mp.get_actor_location().y>100)
 key('A',False);yield .2
 reset();yield .2
 key('E',True);yield .5
 anim=mp.current_locomotion.get_name()
 check('E strafes with sideways animation',mp.get_velocity().y>375 and yd(mp.get_actor_rotation().yaw,0)<.1 and anim=='MF_Unarmed_Jog_Right',anim)
 key('E',False);yield .3
 key('RightMouseButton',True);yield .3
 key('A',True);yield .5
 check('RMB plus A strafes left',mp.get_velocity().y<-375 and yd(mp.get_actor_rotation().yaw,mc.get_control_rotation().yaw)<.1)
 key('A',False);key('RightMouseButton',False);yield .3
 tap('NumLock');yield .55
 check('NumLock enables autorun',mc.get_editor_property('bAutoRun') and mp.get_velocity().length()>375)
 key('S',True);yield .45
 check('S cancels autorun',not mc.get_editor_property('bAutoRun') and mp.get_editor_property('bBackpedaling'))
 key('S',False);yield .3
 tap('Divide');key('W',True);yield .5
 check('Numpad divide toggles walking',mp.get_editor_property('bWalkMode') and abs(mp.get_velocity().length()-160)<2)
 key('W',False);tap('Divide');yield .3
 zoom=boom.target_arm_length;tap('MouseScrollUp');yield .2
 check('Wheel zooms in',boom.target_arm_length<zoom)
 tap('MouseScrollDown');yield .2
 check('Wheel zooms out',boom.target_arm_length==zoom)
 reset();yield .3
 yield from click(mt[0]);tap('Tab');yield .2
 check('Tab cycles targets',mp.target==mt[1])
 key('LeftShift',True);yield .15
 tap('Tab');yield .2
 key('LeftShift',False)
 check('Shift Tab reverses target cycle',mp.target==mt[0])
 tap('Escape');yield .25
 check('Escape clears target',not mp.target and not mt[0].get_editor_property('bSelected'))
 yield from click(mt[1]);tap('One');yield .5
 check('1 casts at clicked target',mp.casting_spell==1 and mp.target==mt[1])
 tap('Escape');yield .25
 check('Escape cancels cast retaining target',mp.casting_spell==0 and mp.target==mt[1])
 mp.set_editor_property('health',40.);tap('Four');yield .8
 key('RightMouseButton',True);yield .2
 key('LeftMouseButton',True);yield .6
 heal=[b for b in U.GameplayStatics.get_all_actors_of_class(mw,U.FMPetalSpell) if b.get_editor_property('bHealing')]
 e=heal[0].get_actor_location()-mp.get_actor_location() if heal else U.Vector(999,999,999)
 check('Healing centered during mouse movement',len(heal)==1 and (e.x**2+e.y**2)**.5<1,[e.x,e.y])
 check('Healing restores health while moving',mp.health>45 and mp.get_velocity().length()>300,mp.health)
 key('LeftMouseButton',False);key('RightMouseButton',False);yield .3
 startz=mp.get_actor_location().z;key('SpaceBar',True);yield .22
 check('Space jumps',mp.get_actor_location().z>startz+25)
 key('SpaceBar',False);yield .8
 report['ok']=True;result_path.write_text(json.dumps(report,indent=2));print('PB_WOW_CONTROLS_ALL_PASS')
steps=tests();ready=0.
def wow_verify_tick(dt):
 global ready
 try:
  now=U.GameplayStatics.get_time_seconds(mw)
  if now<ready:return
  ready=now+next(steps)
 except StopIteration:U.unregister_slate_post_tick_callback(wow_handle)
 except Exception:
  report['error']=traceback.format_exc();result_path.write_text(json.dumps(report,indent=2));U.unregister_slate_post_tick_callback(wow_handle)
  for name in list(held):
   try:key(name,False)
   except Exception:pass
  print('PB_WOW_CONTROLS_FAILED',report['error'])
wow_handle=U.register_slate_post_tick_callback(wow_verify_tick)
