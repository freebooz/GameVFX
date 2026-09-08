using UnrealBuildTool;
public class ModularGameVFXHost : ModuleRules {
 public ModularGameVFXHost(ReadOnlyTargetRules Target) : base(Target) {
  PCHUsage=PCHUsageMode.UseExplicitOrSharedPCHs;
  PrivateDependencyModuleNames.AddRange(new[]{"Core","CoreUObject","Engine","UMG","ModularGameVFXShowcase"});
 }
}
