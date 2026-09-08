using UnrealBuildTool;
public class ModularGameVFXShowcase : ModuleRules
{
    public ModularGameVFXShowcase(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "GameplayTags", "ModularGameVFX", "UMG" });
        PrivateDependencyModuleNames.AddRange(new[] { "Niagara", "NiagaraCore", "Slate", "SlateCore", "InputCore", "Json", "JsonUtilities", "DeveloperSettings", "AssetRegistry" });
    }
}
