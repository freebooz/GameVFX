import unreal as U,pathlib,json
ROOT='/Game/FireMage';SRC=pathlib.Path('F:/game/MythicVFXLab/ArtSource/FireMage')
mel=U.MaterialEditingLibrary;at=U.AssetToolsHelpers.get_asset_tools()
assert not U.EditorLevelLibrary.get_game_world()
assert not (SRC/'assets_built.json').exists(),'Initial build only; patch finished materials separately'
tasks=[]
for p in (SRC/'Textures').glob('*.png'):
    t=U.AssetImportTask();t.filename=str(p);t.destination_path=ROOT+'/Textures';t.destination_name=p.stem;t.automated=True;t.replace_existing=False;t.save=True;tasks.append(t)
at.import_asset_tasks(tasks)
for p in (SRC/'Textures').glob('*.png'):
    t=U.load_asset(ROOT+'/Textures/'+p.stem);assert t,p
    t.set_editor_property('srgb',False);t.set_editor_property('lod_group',U.TextureGroup.TEXTUREGROUP_EFFECTS)
    packed='Hero' in p.stem or 'Turbulence' in p.stem
    t.set_editor_property('compression_settings',U.TextureCompressionSettings.TC_DEFAULT if 'Hero' in p.stem else (U.TextureCompressionSettings.TC_MASKS if packed else U.TextureCompressionSettings.TC_GRAYSCALE))
    t.set_editor_property('max_texture_size',1024 if 'Hero' in p.stem else 512)
    if 'Hero' in p.stem:t.set_editor_property('power_of_two_mode',U.TexturePowerOfTwoSetting.STRETCH_TO_POWER_OF_TWO)
    wrap='Turbulence' in p.stem or 'Ribbon' in p.stem
    t.set_editor_property('address_x',U.TextureAddress.TA_WRAP if wrap else U.TextureAddress.TA_CLAMP)
    t.set_editor_property('address_y',U.TextureAddress.TA_WRAP if 'Turbulence' in p.stem else U.TextureAddress.TA_CLAMP)
    U.EditorAssetLibrary.save_loaded_asset(t,False)
exec((SRC/'material_helpers.py').read_text(encoding='utf-8-sig'))
def custom(m,code,inputs,kind=U.CustomMaterialOutputType.CMOT_FLOAT3):
    c=node(m,U.MaterialExpressionCustom,code=code,output_type=kind)
    args=[]
    for name,e,out in inputs:
        ip=U.CustomInput();ip.set_editor_property('input_name',name);args.append(ip)
    c.set_editor_property('inputs',args)
    for name,e,out in inputs:connect(e,c,name,out)
    return c
temp='float h=saturate(L); float3 c=lerp(float3(1,.025,.002),float3(1,.32,.012),sqrt(h)); c=lerp(c,float3(1,.82,.30),pow(h,3)); return c;'
for name,hero in [('M_PY_Flame',True),('M_PY_Ribbon',False)]:
    m=newmat(name);uv=node(m,U.MaterialExpressionTextureCoordinate);pc=node(m,U.MaterialExpressionParticleColor)
    pan=node(m,U.MaterialExpressionPanner,speed_x=.11,speed_y=-.19);connect(uv,pan,'Coordinate')
    noise=node(m,U.MaterialExpressionTextureSampleParameter2D,parameter_name='Turbulence',texture=U.load_asset(ROOT+'/Textures/T_PY_Turbulence'),sampler_type=U.MaterialSamplerType.SAMPLERTYPE_MASKS);connect(pan,noise,'UVs')
    warp=custom(m,'return UV+(N.rg-.5)*Amount;', [('UV',uv,''),('N',noise,'RGB'),('Amount',scalar(m,'Distortion',.045 if hero else .09),'')],U.CustomMaterialOutputType.CMOT_FLOAT2)
    tex=node(m,U.MaterialExpressionTextureSampleParameter2D,parameter_name='FlameTexture',texture=U.load_asset(ROOT+'/Textures/'+('T_PY_HeroFlame' if hero else 'T_PY_FlameRibbon')),sampler_type=U.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR if hero else U.MaterialSamplerType.SAMPLERTYPE_LINEAR_GRAYSCALE);connect(warp,tex,'UVs')
    col=custom(m,temp,[('L',tex,'R')]);output(mul(m,mul(m,col,pc),scalar(m,'Energy',7 if hero else 4)),m,U.MaterialProperty.MP_EMISSIVE_COLOR)
    alpha=node(m,U.MaterialExpressionMultiply);connect(tex,alpha,'A','R');connect(pc,alpha,'B','A')
    if hero:
        aa=node(m,U.MaterialExpressionMultiply);connect(alpha,aa,'A');connect(tex,aa,'B','A');alpha=aa
    alpha=mul(m,alpha,scalar(m,'Density',.9));fade=node(m,U.MaterialExpressionDepthFade,fade_distance_default=12);connect(alpha,fade,'Opacity');output(fade,m,U.MaterialProperty.MP_OPACITY);finish(m)
for name,blend in [('M_PY_Additive',U.BlendMode.BLEND_ADDITIVE),('M_PY_Smoke',U.BlendMode.BLEND_TRANSLUCENT)]:
    m=newmat(name,blend);pc=node(m,U.MaterialExpressionParticleColor);tex=sample(m,'Mask','T_PY_Ember')
    output(mul(m,pc,scalar(m,'Energy',6)),m,U.MaterialProperty.MP_EMISSIVE_COLOR)
    alpha=node(m,U.MaterialExpressionMultiply);connect(tex,alpha,'A','R');connect(pc,alpha,'B','A');alpha=mul(m,alpha,scalar(m,'Density',1))
    fade=node(m,U.MaterialExpressionDepthFade,fade_distance_default=16);connect(alpha,fade,'Opacity');output(fade,m,U.MaterialProperty.MP_OPACITY);finish(m)
for name,parent,tex,energy,density in [
    ('MI_PY_Ember','M_PY_Additive','T_PY_Ember',9,1),
    ('MI_PY_Ring','M_PY_Additive','T_PY_ShockRing',5,.9),
    ('MI_PY_Emberbed','M_PY_Additive','T_PY_Emberbed',1.5,.7),
    ('MI_PY_Smoke','M_PY_Smoke','T_PY_Smoke',.08,.4)]:
    instance(name,U.load_asset(ROOT+'/Materials/'+parent),tex,energy,density)
tasks=[]
for p in (SRC/'Meshes').glob('*.obj'):
    t=U.AssetImportTask();t.filename=str(p);t.destination_path=ROOT+'/Meshes';t.destination_name=p.stem;t.automated=True;t.replace_existing=False;t.save=True
    o=U.FbxImportUI();o.import_mesh=True;o.import_materials=False;o.import_textures=False;o.import_as_skeletal=False;o.mesh_type_to_import=U.FBXImportType.FBXIT_STATIC_MESH
    o.static_mesh_import_data.combine_meshes=True;o.static_mesh_import_data.normal_import_method=U.FBXNormalImportMethod.FBXNIM_COMPUTE_NORMALS;t.options=o;tasks.append(t)
at.import_asset_tasks(tasks)
report=[]
for p in (SRC/'Meshes').glob('*.obj'):
    mesh=U.load_asset(ROOT+'/Meshes/'+p.stem);assert mesh,p
    mesh.set_material(0,U.load_asset(ROOT+'/Materials/M_PY_Ribbon'));U.EditorAssetLibrary.save_loaded_asset(mesh,False)
    b=mesh.get_bounds();report.append({'mesh':p.stem,'bounds':str(b)})
U.EditorAssetLibrary.save_directory(ROOT,False,True);(SRC/'assets_built.json').write_text(json.dumps(report));print('FIRE_ASSETS_READY',json.dumps(report))

