def newmat(name,blend=U.BlendMode.BLEND_ADDITIVE,lit=False):
    path=ROOT+'/Materials/'+name;m=U.load_asset(path)
    if not m:m=at.create_asset(name,ROOT+'/Materials',U.Material,U.MaterialFactoryNew())
    mel.delete_all_material_expressions(m)
    m.set_editor_property('blend_mode',blend)
    m.set_editor_property('shading_model',U.MaterialShadingModel.MSM_DEFAULT_LIT if lit else U.MaterialShadingModel.MSM_UNLIT)
    m.set_editor_property('two_sided',True)
    mel.set_material_usage(m,U.MaterialUsage.MATUSAGE_NIAGARA_SPRITES)
    mel.set_material_usage(m,U.MaterialUsage.MATUSAGE_NIAGARA_MESH_PARTICLES)
    return m
def node(m,cls,**kw):
    e=mel.create_material_expression(m,cls)
    for k,v in kw.items():e.set_editor_property(k,v)
    return e
def connect(a,b,pin,out=''): assert mel.connect_material_expressions(a,out,b,pin),(a,b,pin)
def output(e,m,prop,out=''):assert mel.connect_material_property(e,out,prop)
def scalar(m,name,value):return node(m,U.MaterialExpressionScalarParameter,parameter_name=name,default_value=value)
def vector(m,name,value):return node(m,U.MaterialExpressionVectorParameter,parameter_name=name,default_value=U.LinearColor(*value))
def mul(m,a,b):
    e=node(m,U.MaterialExpressionMultiply);connect(a,e,'A');connect(b,e,'B');return e
def sample(m,name,tex,normal=False):
    return node(m,U.MaterialExpressionTextureSampleParameter2D,parameter_name=name,texture=U.load_asset(ROOT+'/Textures/'+tex),sampler_type=U.MaterialSamplerType.SAMPLERTYPE_NORMAL if normal else U.MaterialSamplerType.SAMPLERTYPE_LINEAR_GRAYSCALE)
def finish(m):
    mel.layout_material_expressions(m);mel.recompile_material(m);U.EditorAssetLibrary.save_loaded_asset(m)
def instance(name,parent,texture=None,energy=1,density=1,tint=(.3,.7,1,1)):
    m=U.load_asset(ROOT+'/Materials/Instances/'+name)
    if not m:m=at.create_asset(name,ROOT+'/Materials/Instances',U.MaterialInstanceConstant,U.MaterialInstanceConstantFactoryNew())
    mel.set_material_instance_parent(m,parent)
    mel.set_material_instance_scalar_parameter_value(m,'Energy',energy)
    mel.set_material_instance_scalar_parameter_value(m,'Density',density)
    mel.set_material_instance_vector_parameter_value(m,'FrostTint',U.LinearColor(*tint))
    if texture:mel.set_material_instance_texture_parameter_value(m,'Mask',U.load_asset(ROOT+'/Textures/'+texture))
    mel.update_material_instance(m);U.EditorAssetLibrary.save_loaded_asset(m);return m


