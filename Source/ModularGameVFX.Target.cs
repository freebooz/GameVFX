using UnrealBuildTool;
public class ModularGameVFXTarget : TargetRules {
 public ModularGameVFXTarget(TargetInfo Target) : base(Target) {
  DefaultBuildSettings=BuildSettingsVersion.Latest; IncludeOrderVersion=EngineIncludeOrderVersion.Latest;
  Type=TargetType.Game; ExtraModuleNames.Add("ModularGameVFXHost");
 }
}
