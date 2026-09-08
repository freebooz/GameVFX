"""Fixed-time, unedited engine capture of the player channel and curved missiles."""
import unreal as U,pathlib,json,traceback
w=U.EditorLevelLibrary.get_game_world();p=U.GameplayStatics.get_player_character(w,0);pc=U.GameplayStatics.get_player_controller(w,0);m=U.GameplayStatics.get_all_actors_of_class(w,U.FMFrostMonster)[0]
cap=U.GameplayStatics.get_all_actors_of_class(w,U.SceneCapture2D)[0];c=cap.get_component_by_class(U.SceneCaptureComponent2D)
rt=U.RenderingLibrary.create_render_target2d(w,1600,900,U.TextureRenderTargetFormat.RTF_RGBA8);rt.set_editor_property('target_gamma',2.2)
for k,v in {'texture_target':rt,'capture_source':U.SceneCaptureSource.SCS_FINAL_COLOR_LDR,'capture_every_frame':False,'capture_on_movement':False,'always_persist_rendering_state':True,'fov_angle':53.}.items():c.set_editor_property(k,v)
p.set_actor_location(U.Vector(0,0,96.15),False,False);p.set_actor_rotation(U.Rotator(),False);m.set_actor_location(U.Vector(920,0,122.15),False,False);m.set_editor_property('bCombatEnabled',False);m.character_movement.disable_movement();p.select_next_target()
pos=U.Vector(280,-1020,440);look=U.Vector(450,0,140);cap.set_actor_location_and_rotation(pos,U.MathLibrary.find_look_at_rotation(pos,look),False,False)
out=pathlib.Path('F:/game/MythicVFXLab/Saved/VFXValidation/ArcaneMage/FilmFrames_Master');out.mkdir(parents=True,exist_ok=True)
state={'frame':-25,'frames_written':0,'fps':30,'width':1600,'height':900,'finished':False}
U.SystemLibrary.execute_console_command(w,'t.OverrideFPS 30')
def tick(dt):
    try:
        f=state['frame']
        if f==0:p.cast_arcane_missiles()
        c.capture_scene()
        if f>=0:U.RenderingLibrary.export_render_target(w,rt,str(out),f'frame_{f:04d}.png');state['frames_written']+=1
        state['frame']+=1
        if state['frame']%30==0:(out/'progress.json').write_text(json.dumps(state))
        if state['frame']>=135:
            state['finished']=True;U.SystemLibrary.execute_console_command(w,'t.OverrideFPS 0');U.unregister_slate_post_tick_callback(handle);(out/'capture.json').write_text(json.dumps(state,indent=2));print('ARCANE_FILM_COMPLETE')
    except Exception:
        state['error']=traceback.format_exc();(out/'capture.json').write_text(json.dumps(state,indent=2));U.SystemLibrary.execute_console_command(w,'t.OverrideFPS 0');U.unregister_slate_post_tick_callback(handle);print(state['error'])
handle=U.register_slate_post_tick_callback(tick)
