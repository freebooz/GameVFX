import unreal as U,pathlib,json
w=U.EditorLevelLibrary.get_game_world();p=U.GameplayStatics.get_player_character(w,0);pc=U.GameplayStatics.get_player_controller(w,0);m=U.GameplayStatics.get_all_actors_of_class(w,U.FMFrostMonster)[0]
p.set_actor_location(U.Vector(0,0,96.15),False,False);p.set_actor_rotation(U.Rotator(),False);pc.set_control_rotation(U.Rotator(pitch=-24,yaw=0,roll=0));m.set_actor_location(U.Vector(700,0,122.15),False,False);m.set_editor_property('bCombatEnabled',False);m.character_movement.disable_movement();p.select_next_target()
info={'viewport':list(pc.get_viewport_size()),'project':U.Paths.get_project_file_path(),'world':'L_FrostMage_Test','effects':globals().get('WITH_EFFECTS',False)}
path=pathlib.Path('F:/game/MythicVFXLab/Saved/VFXValidation/ArcaneMage/benchmark_'+('effects' if info['effects'] else 'baseline')+'_setup.json');path.write_text(json.dumps(info,indent=2))
if info['effects']:
    # Repeated combat in a fixed scene, no screenshots or SimCache during timing.
    ready=0.;stage=0
    def bench_tick(dt):
        global ready,stage
        now=U.GameplayStatics.get_time_seconds(w)
        if now<ready:return
        m.set_editor_property('health',100.)
        if stage%2==0:
            p.set_editor_property('flamestrike_cooldown',0.);p.cast_flamestrike();ready=now+2.2
        else:
            p.set_editor_property('arcane_cooldown',0.);p.cast_arcane_missiles();ready=now+2.8
        stage+=1
        if stage>=6:U.unregister_slate_post_tick_callback(handle)
    handle=U.register_slate_post_tick_callback(bench_tick)
