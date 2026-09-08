import unreal as U, pathlib, json, collections
reg=U.AssetRegistryHelpers.get_asset_registry()
reg.scan_paths_synchronous(['/MythicVFX','/ModularGameVFXDemo'],force_rescan=True)
opts=U.AssetRegistryDependencyOptions(include_soft_package_references=True,include_hard_package_references=True,include_searchable_names=False,include_soft_management_references=False,include_hard_management_references=False)
rows=[];imports=[];bad=[];missing=[]
for root in ['/MythicVFX','/ModularGameVFXDemo']:
 for a in reg.get_assets_by_path(root,recursive=True):
  package=str(a.package_name);cls=str(a.asset_class_path.asset_name)
  if cls=='ObjectRedirector':continue
  deps=[str(x) for x in reg.get_dependencies(a.package_name,opts)]
  row={'package':package,'class':cls,'dependencies':deps};rows.append(row)
  for d in deps:
   if d.startswith('/Game/') or d=='/Script/MythicVFXLab' or (root=='/MythicVFX' and d.startswith('/ModularGameVFXDemo/')):bad.append([package,d])
   if not d.startswith('/Script/') and not U.EditorAssetLibrary.does_asset_exist(d):missing.append([package,d])
  if package.startswith('/ModularGameVFX/Effects/') and cls in ['Texture2D','StaticMesh']:
   obj=U.load_asset(package)
   try:
    data=obj.get_editor_property('asset_import_data')
    imports.append({'package':package,'files':list(data.extract_filenames())})
   except Exception:pass
out=pathlib.Path(U.Paths.project_saved_dir())/'ModuleRelease';out.mkdir(exist_ok=True)
report={'project':U.Paths.get_project_file_path(),'counts':dict(collections.Counter(x['package'].split('/')[1] for x in rows)),'forbidden_dependencies':bad,'missing_dependencies':missing,'assets':rows,'source_imports':imports,'ok':not bad and not missing}
(out/'DependencyAudit.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps({k:v for k,v in report.items() if k not in ['assets','source_imports']},ensure_ascii=False))
