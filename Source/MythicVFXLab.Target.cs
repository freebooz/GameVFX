using UnrealBuildTool;
public class MythicVFXLabTarget : TargetRules {
 public MythicVFXLabTarget(TargetInfo Target) : base(Target) {
  DefaultBuildSettings=BuildSettingsVersion.Latest; IncludeOrderVersion=EngineIncludeOrderVersion.Latest;
  Type=TargetType.Game; ExtraModuleNames.Add("MythicVFXLab");
 }
}
