"""Run in an editor via `py <absolute path>`; starts an unsaved PIE session.

Exercises real rendered Niagara instances over five waves, without forcing GC.
Separates inactive pool capacity from active simulations and records process memory.
Does not save or alter the editor map. Results: Saved/ResourceAudit/runtime_soak.json.
"""
import collections
import ctypes
import json
import pathlib
import time
import traceback
import unreal as U

OUT = pathlib.Path(U.Paths.project_saved_dir()) / 'ResourceAudit/runtime_soak.json'
OUT.parent.mkdir(parents=True, exist_ok=True)
NAMES = [
    '05_Impact/NS_Ice_Frostbolt_Impact',
    '01_Cast/NS_Ice_Frostbolt_Charge',
    '06_Area/NS_Ice_FrostNova_Burst',
    '05_Impact/NS_Fire_Impact_Burst',
    '05_Impact/NS_Arcane_Missiles_Impact',
]
report = {'ok': False, 'samples': [], 'waves': 5, 'instances_per_wave': 50,
          'forced_gc': False, 'scope': 'real PIE Niagara lifetime; not a GPU frame-time benchmark'}
started = time.monotonic()
world = None

class MemoryCounters(ctypes.Structure):
    _fields_ = [('cb', ctypes.c_ulong), ('faults', ctypes.c_ulong)] + [
        (n, ctypes.c_size_t) for n in ['peak_working', 'working', 'peak_paged',
        'paged', 'peak_nonpaged', 'nonpaged', 'pagefile', 'peak_pagefile', 'private']]

def memory():
    counters = MemoryCounters()
    counters.cb = ctypes.sizeof(counters)
    kernel = ctypes.WinDLL('kernel32')
    kernel.GetCurrentProcess.restype = ctypes.c_void_p
    fn = ctypes.WinDLL('psapi').GetProcessMemoryInfo
    fn.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_ulong]
    if not fn(kernel.GetCurrentProcess(), ctypes.byref(counters), counters.cb):
        raise ctypes.WinError()
    return {'private_mb': round(counters.private / 1048576, 2),
            'working_mb': round(counters.working / 1048576, 2)}

def snapshot(stage):
    active = collections.Counter()
    total = 0
    lights = 0
    actors = U.GameplayStatics.get_all_actors_of_class(world, U.Actor)
    for actor in actors:
        lights += len(actor.get_components_by_class(U.PointLightComponent))
        for component in actor.get_components_by_class(U.NiagaraComponent):
            total += 1
            if component.is_active():
                asset = component.get_asset()
                active[asset.get_name() if asset else 'None'] += 1
    sample = {'stage': stage, 'game_seconds': U.GameplayStatics.get_time_seconds(world),
              'actors': len(actors), 'point_lights': lights,
              'registered_niagara': total, 'active': dict(active), **memory()}
    report['samples'].append(sample)
    OUT.write_text(json.dumps(report, indent=2), encoding='utf-8')
    return sample

def run():
    global world
    while not U.EditorLevelLibrary.get_game_world():
        yield .2
    world = U.EditorLevelLibrary.get_game_world()
    for monster in U.GameplayStatics.get_all_actors_of_class(world, U.FMFrostMonster):
        monster.set_editor_property('bCombatEnabled', False)
    systems = [U.load_asset('/ModularGameVFX/ModularGameVFXLibrary/' + n) for n in NAMES]
    assert all(systems), 'Missing current Niagara assets'
    yield 8
    baseline = snapshot('baseline')
    for wave in range(5):
        for system in systems:
            for i in range(10):
                component = U.NiagaraFunctionLibrary.spawn_system_at_location(
                    world, system, U.Vector(450, i * 40, 90), U.Rotator(),
                    U.Vector(1, 1, 1), True, True, U.NCPoolMethod.AUTO_RELEASE, False)
                assert component, 'Failed to spawn ' + system.get_name()
        yield .2
        peak = snapshot('wave_%d_active' % wave)
        for system in systems:
            name = system.get_name()
            assert peak['active'].get(name, 0) >= baseline['active'].get(name, 0) + 10, name + ' was not active'
        yield 9.8
        settled = snapshot('wave_%d_settled' % wave)
        for system in systems:
            name = system.get_name()
            assert settled['active'].get(name, 0) <= baseline['active'].get(name, 0), name + ' did not finish'
        assert settled['point_lights'] == baseline['point_lights'], 'Lights accumulated'
    player = U.GameplayStatics.get_player_character(world, 0)
    controller = U.GameplayStatics.get_player_controller(world, 0)
    assert player and controller, 'Combat player required'
    for key, sign in [('D', 1), ('A', -1)]:
        start = player.get_actor_location()
        yaw = player.get_actor_rotation().yaw
        right = player.get_actor_right_vector()
        controller.simulate_key(key, True)
        yield .6
        controller.simulate_key(key, False)
        offset = player.get_actor_location() - start
        lateral = offset.x * right.x + offset.y * right.y
        assert lateral * sign > 80, key + ' failed to strafe'
        assert abs((player.get_actor_rotation().yaw-yaw+180) % 360-180) < .1, key + ' turned the player'
        yield .3
    report['ad_strafe'] = True
    monsters = U.GameplayStatics.get_all_actors_of_class(world, U.FMFrostMonster)
    target = monsters[0]
    target.set_actor_location(player.get_actor_location()+player.get_actor_forward_vector()*600, False, False)
    target.set_editor_property('bCombatEnabled', True)
    target.set_editor_property('bChasing', False)
    player.set_editor_property('health', 100)
    yield 4
    assert not target.get_editor_property('bChasing') and player.health == 100, 'Monster acquired player without damage'
    target.receive_fire(1)
    assert target.get_editor_property('bChasing'), 'Monster did not retaliate after damage'
    target.set_editor_property('bCombatEnabled', False)
    yield 6
    report['retaliation_only'] = True
    combat_baseline = snapshot('combat_baseline')
    spells = [('cast_frostbolt', 'bolt_cooldown', 2.5), ('cast_frost_nova', 'nova_cooldown', .3),
              ('cast_petal_bloom', 'petal_cooldown', 1), ('cast_healing_bloom', 'healing_cooldown', .7),
              ('cast_fireball', 'fireball_cooldown', 2.5), ('cast_fire_blast', 'fire_blast_cooldown', .3),
              ('cast_flamestrike', 'flamestrike_cooldown', 2.5), ('cast_arcane_missiles', 'arcane_cooldown', 3)]
    for wave in range(3):
        target.set_editor_property('health', 10000)
        player.set_editor_property('health', 50)
        player.set_editor_property('target', target)
        for method, cooldown, delay in spells:
            player.set_editor_property(cooldown, 0)
            getattr(player, method)()
            if delay >= .7:
                assert player.casting_spell != 0, method + ' did not begin casting'
            yield delay
        snapshot('combat_%d_active' % wave)
        yield 10
        settled = snapshot('combat_%d_settled' % wave)
        assert settled['actors'] == combat_baseline['actors'], 'Spell actors accumulated'
        assert settled['point_lights'] == combat_baseline['point_lights'], 'Spell lights accumulated'
        assert sum(settled['active'].values()) <= sum(combat_baseline['active'].values()), 'Combat Niagara remained active'
    report['combat_spell_casts'] = 24
    report['ok'] = True
    OUT.write_text(json.dumps(report, indent=2), encoding='utf-8')
    print('RESOURCE_LIFETIME_PASS', str(OUT))

sequence = run()
ready = started

def tick(dt):
    global ready
    try:
        now = time.monotonic()
        if now - started > 240:
            raise TimeoutError('PIE lifetime test exceeded 240 seconds')
        if now < ready:
            return
        ready = now + next(sequence)
    except StopIteration:
        U.unregister_slate_post_tick_callback(handle)
    except Exception:
        report['error'] = traceback.format_exc()
        OUT.write_text(json.dumps(report, indent=2), encoding='utf-8')
        U.unregister_slate_post_tick_callback(handle)
        print('RESOURCE_LIFETIME_FAILED', report['error'])

handle = U.register_slate_post_tick_callback(tick)
if not U.EditorLevelLibrary.get_game_world():
    U.get_editor_subsystem(U.LevelEditorSubsystem).editor_request_begin_play()
