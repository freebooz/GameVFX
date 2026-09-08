using UnrealBuildTool;
public class ModularGameVFXDemo : ModuleRules {
 public ModularGameVFXDemo(ReadOnlyTargetRules Target) : base(Target) {
  PCHUsage=PCHUsageMode.NoPCHs;
  PublicDependencyModuleNames.AddRange(new[]{"Core","CoreUObject","Engine","InputCore","Niagara","AIModule","NavigationSystem","ModularGameVFX"});
  if(Target.bBuildEditor) PrivateDependencyModuleNames.AddRange(new[]{"UnrealEd","Slate"});
 }
}
