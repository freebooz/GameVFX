import unreal as U,pathlib,json
ROOT='/Game/ArcaneMage';SRC=pathlib.Path('F:/game/MythicVFXLab/ArtSource/ArcaneMage');at=U.AssetToolsHelpers.get_asset_tools();mel=U.MaterialEditingLibrary
assert not U.EditorLevelLibrary.get_game_world()
assert not (SRC/'assets_built.json').exists()
tasks=[]
for p in (SRC/'Textures').glob('*.png'):
    t=U.AssetImportTask();t.filename=str(p);t.destination_path=ROOT+'/Textures';t.destination_name=p.stem;t.automated=True;t.replace_existing=False;t.save=True;tasks.append(t)
at.import_asset_tasks(tasks)
for p in (SRC/'Textures').glob('*.png'):
    t=U.load_asset(ROOT+'/Textures/'+p.stem);t.set_editor_property('srgb',False);t.set_editor_property('compression_settings',U.TextureCompressionSettings.TC_GRAYSCALE);t.set_editor_property('max_texture_size',512);t.set_editor_property('lod_group',U.TextureGroup.TEXTUREGROUP_EFFECTS)
    t.set_editor_property('address_x',U.TextureAddress.TA_WRAP if 'Ribbon' in p.stem else U.TextureAddress.TA_CLAMP);t.set_editor_property('address_y',U.TextureAddress.TA_CLAMP);U.EditorAssetLibrary.save_loaded_asset(t,False)
exec(pathlib.Path('F:/game/MythicVFXLab/ArtSource/FireMage/material_helpers.py').read_text(encoding='utf-8-sig'))
m=newmat('M_AM_Energy');pc=node(m,U.MaterialExpressionParticleColor);tex=sample(m,'Mask','T_AM_Core')
output(mul(m,pc,scalar(m,'Energy',6)),m,U.MaterialProperty.MP_EMISSIVE_COLOR)
a=node(m,U.MaterialExpressionMultiply);connect(tex,a,'A','R');connect(pc,a,'B','A');output(a,m,U.MaterialProperty.MP_OPACITY);mel.set_material_usage(m,U.MaterialUsage.MATUSAGE_NIAGARA_RIBBONS);finish(m)
for name,tex,en in [('MI_AM_Core','T_AM_Core',7),('MI_AM_Ribbon','T_AM_Ribbon',5),('MI_AM_Seal','T_AM_RunicSeal',3),('MI_AM_Star','T_AM_Star',10)]:instance(name,m,tex,en,1)
# A tiny opaque emissive spindle gives the missile a solid readable center.
m=newmat('M_AM_Spindle',U.BlendMode.BLEND_OPAQUE);pc=node(m,U.MaterialExpressionParticleColor)
f=node(m,U.MaterialExpressionFresnel,exponent=2.2,base_reflect_fraction=.04)
l=node(m,U.MaterialExpressionLinearInterpolate);connect(vector(m,'CoreTint',(.42,.74,1,1)),l,'A');connect(vector(m,'RimTint',(.33,.065,1,1)),l,'B');connect(f,l,'Alpha')
output(mul(m,mul(m,l,pc),scalar(m,'Energy',4)),m,U.MaterialProperty.MP_EMISSIVE_COLOR);finish(m)
t=U.AssetImportTask();t.filename=str(SRC/'Meshes/SM_AM_MissileCore.obj');t.destination_path=ROOT+'/Meshes';t.destination_name='SM_AM_MissileCore';t.automated=True;t.save=True
o=U.FbxImportUI();o.import_mesh=True;o.import_materials=False;o.import_textures=False;o.import_as_skeletal=False;o.mesh_type_to_import=U.FBXImportType.FBXIT_STATIC_MESH;o.static_mesh_import_data.combine_meshes=True;o.static_mesh_import_data.normal_import_method=U.FBXNormalImportMethod.FBXNIM_COMPUTE_NORMALS;t.options=o;at.import_asset_tasks([t])
mesh=U.load_asset(ROOT+'/Meshes/SM_AM_MissileCore');assert mesh;mesh.set_material(0,m);U.EditorAssetLibrary.save_loaded_asset(mesh,False)
U.EditorAssetLibrary.save_directory(ROOT,False,True);(SRC/'assets_built.json').write_text(json.dumps({'built':True}));print('ARCANE_ASSETS_READY')
