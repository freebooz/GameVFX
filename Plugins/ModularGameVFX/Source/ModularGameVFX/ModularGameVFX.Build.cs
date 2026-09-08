using UnrealBuildTool;

public class ModularGameVFX : ModuleRules
{
    public ModularGameVFX(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "Niagara",
            "GameplayTags",
            "DeveloperSettings"
        });

        PrivateDependencyModuleNames.Add("Projects");
    }
}
