#include "Modules/ModuleManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UObject/Package.h"
#include "Kismet/GameplayStatics.h"
#include "VFXShowcaseController.h"
#include "VFXShowcaseProfile.h"
#include "VFXShowcaseWidget.h"

// Host integration keeps the reusable Demo and Showcase plugins independent.
class FModularGameVFXHostModule : public FDefaultGameModuleImpl
{
    FDelegateHandle WorldInitHandle;
    static void AttachCombatWorkbench(UWorld* World)
    {
        if(!IsValid(World))return;
        for(TActorIterator<AVFXShowcaseController> It(World);It;++It)return;
        AVFXShowcaseController* Controller=World->SpawnActorDeferred<AVFXShowcaseController>(
            AVFXShowcaseController::StaticClass(),FTransform::Identity,nullptr,nullptr,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        if(!Controller)return;
        Controller->bUseExistingCombatWorld=true;
        Controller->Profile=LoadObject<UVFXShowcaseProfile>(nullptr,TEXT("/ModularGameVFXShowcase/Profiles/DA_VFXShowcase_DefaultProfiles.DA_VFXShowcase_DefaultProfiles"));
        Controller->WidgetClass=LoadClass<UUserWidget>(nullptr,TEXT("/ModularGameVFXShowcase/UI/WBP_VFX_Showcase.WBP_VFX_Showcase_C"));
        UGameplayStatics::FinishSpawningActor(Controller,FTransform::Identity);
    }
    static void WorldInitialized(UWorld* World,const UWorld::InitializationValues)
    {
        if(!World||(World->WorldType!=EWorldType::Game&&World->WorldType!=EWorldType::PIE))return;
        const FString Package=World->GetOutermost()->GetName();
        if(!Package.StartsWith(TEXT("/ModularGameVFXDemo/Maps/"))||!Package.EndsWith(TEXT("L_ModularGameVFX_Demo")))return;
        World->OnWorldBeginPlay.AddWeakLambda(World,[World](){AttachCombatWorkbench(World);});
    }
public:
    virtual void StartupModule() override
    {
        WorldInitHandle=FWorldDelegates::OnPostWorldInitialization.AddStatic(&WorldInitialized);
    }
    virtual void ShutdownModule() override
    {
        FWorldDelegates::OnPostWorldInitialization.Remove(WorldInitHandle);
    }
};
IMPLEMENT_PRIMARY_GAME_MODULE(FModularGameVFXHostModule, ModularGameVFXHost, "ModularGameVFX");
