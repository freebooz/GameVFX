using UnrealBuildTool;
public class ModularGameVFXEditorTarget : TargetRules {
 public ModularGameVFXEditorTarget(TargetInfo Target) : base(Target) {
  DefaultBuildSettings=BuildSettingsVersion.Latest; IncludeOrderVersion=EngineIncludeOrderVersion.Latest;
  Type=TargetType.Editor; ExtraModuleNames.Add("ModularGameVFXHost");
 }
}
