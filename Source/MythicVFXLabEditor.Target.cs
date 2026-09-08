using UnrealBuildTool;
public class MythicVFXLabEditorTarget : TargetRules {
 public MythicVFXLabEditorTarget(TargetInfo Target) : base(Target) {
  DefaultBuildSettings=BuildSettingsVersion.Latest; IncludeOrderVersion=EngineIncludeOrderVersion.Latest;
  Type=TargetType.Editor; ExtraModuleNames.Add("MythicVFXLab");
 }
}
