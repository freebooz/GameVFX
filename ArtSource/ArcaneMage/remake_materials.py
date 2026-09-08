import unreal as U,pathlib
ROOT='/Game/ArcaneMage';SRC=pathlib.Path('F:/game/MythicVFXLab/ArtSource/ArcaneMage');at=U.AssetToolsHelpers.get_asset_tools();mel=U.MaterialEditingLibrary
assert not U.EditorLevelLibrary.get_game_world()
t=U.AssetImportTask();t.filename=str(SRC/'Textures/T_AM_ArcanePlasma.png');t.destination_path=ROOT+'/Textures';t.automated=True;t.replace_existing=True;t.save=True;at.import_asset_tasks([t])
tex=U.load_asset(ROOT+'/Textures/T_AM_ArcanePlasma')
for k,v in {'srgb':True,'compression_settings':U.TextureCompressionSettings.TC_DEFAULT,'max_texture_size':1024,'power_of_two_mode':U.TexturePowerOfTwoSetting.STRETCH_TO_POWER_OF_TWO,'lod_group':U.TextureGroup.TEXTUREGROUP_EFFECTS,'address_x':U.TextureAddress.TA_CLAMP,'address_y':U.TextureAddress.TA_CLAMP}.items():tex.set_editor_property(k,v)
U.EditorAssetLibrary.save_loaded_asset(tex,False)
exec(pathlib.Path('F:/game/MythicVFXLab/ArtSource/FireMage/material_helpers.py').read_text(encoding='utf-8-sig'))
m=newmat('M_AM_Plasma');pc=node(m,U.MaterialExpressionParticleColor);uv=node(m,U.MaterialExpressionTextureCoordinate);tm=node(m,U.MaterialExpressionTime)
warp=node(m,U.MaterialExpressionCustom);warp.set_editor_property('code','float2 p=UV; p.x+=sin(UV.y*24-Time*7)*.014*sin(UV.y*3.14159); p.y+=sin(UV.x*19+Time*5)*.008; return p;');warp.set_editor_property('output_type',U.CustomMaterialOutputType.CMOT_FLOAT2)
ins=[]
for n in ['UV','Time']:
    i=U.CustomInput();i.set_editor_property('input_name',n);ins.append(i)
warp.set_editor_property('inputs',ins);connect(uv,warp,'UV');connect(tm,warp,'Time')
s=node(m,U.MaterialExpressionTextureSampleParameter2D,parameter_name='Plasma',texture=tex,sampler_type=U.MaterialSamplerType.SAMPLERTYPE_COLOR);connect(warp,s,'UVs')
output(mul(m,mul(m,s,pc),scalar(m,'Energy',3.2)),m,U.MaterialProperty.MP_EMISSIVE_COLOR)
a=node(m,U.MaterialExpressionMultiply);connect(s,a,'A','A');connect(pc,a,'B','A');output(a,m,U.MaterialProperty.MP_OPACITY);finish(m)
instance('MI_AM_Plasma',m,None,3.2)
instance('MI_AM_ImpactPlasma',m,None,2.6)
# Flowing broad wake with two thin bright strands and softer violet body.
m=newmat('M_AM_FlowingWake');uv=node(m,U.MaterialExpressionTextureCoordinate);tm=node(m,U.MaterialExpressionTime);pc=node(m,U.MaterialExpressionParticleColor)
c=node(m,U.MaterialExpressionCustom);c.set_editor_property('code','float x=UV.x*6.28318; float y=UV.y-.5; float w1=sin(x*1.6-Time*8)*.22; float w2=sin(x*1.6-Time*8+3.14159)*.22; float strands=exp(-pow((y-w1)*45,2))+exp(-pow((y-w2)*45,2)); float body=exp(-y*y*26)*(0.35+0.18*sin(x*4-Time*15)); return float4(float3(.17,.025,.8)*body+float3(.38,.6,1)*strands, saturate(body+strands));');c.set_editor_property('output_type',U.CustomMaterialOutputType.CMOT_FLOAT4)
ins=[]
for n in ['UV','Time']:
    i=U.CustomInput();i.set_editor_property('input_name',n);ins.append(i)
c.set_editor_property('inputs',ins);connect(uv,c,'UV');connect(tm,c,'Time')
rgb=node(m,U.MaterialExpressionComponentMask,r=True,g=True,b=True,a=False);connect(c,rgb,'');output(mul(m,mul(m,rgb,pc),scalar(m,'Energy',4)),m,U.MaterialProperty.MP_EMISSIVE_COLOR)
mask=node(m,U.MaterialExpressionComponentMask,r=False,g=False,b=False,a=True);connect(c,mask,'');a=node(m,U.MaterialExpressionMultiply);connect(mask,a,'A');connect(pc,a,'B','A');output(a,m,U.MaterialProperty.MP_OPACITY);mel.set_material_usage(m,U.MaterialUsage.MATUSAGE_NIAGARA_RIBBONS);finish(m)
U.EditorAssetLibrary.save_directory(ROOT,False,True);print('ARCANE_REMAKE_MATERIALS_READY')
