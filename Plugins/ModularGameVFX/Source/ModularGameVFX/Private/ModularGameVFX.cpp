#include "Modules/ModuleManager.h"

#include "GameplayTagsManager.h"
#include "Interfaces/IPluginManager.h"
#include "VFXTypes.h"

DEFINE_LOG_CATEGORY(LogModularGameVFX);

class FModularGameVFXModule final : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("ModularGameVFX"));
        if (Plugin.IsValid())
        {
            TagConfigPath = Plugin->GetBaseDir() / TEXT("Config") / TEXT("Tags");
            UGameplayTagsManager::Get().AddTagIniSearchPath(TagConfigPath);
        }

    }

    virtual void ShutdownModule() override
    {
        if (!TagConfigPath.IsEmpty() && FModuleManager::Get().IsModuleLoaded(TEXT("GameplayTags")))
        {
            UGameplayTagsManager::Get().RemoveTagIniSearchPath(TagConfigPath);
        }
    }

private:
    FString TagConfigPath;
};

IMPLEMENT_MODULE(FModularGameVFXModule, ModularGameVFX)
