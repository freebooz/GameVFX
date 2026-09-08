#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "VFXShowcaseController.h"
#include "VFXShowcaseProfile.h"
#include "VFXShowcaseEnvironment.h"
#include "Components/SceneComponent.h"
#include "HAL/IConsoleManager.h"
#include "ModularGameVFXBlueprintLibrary.h"
#include "ModularGameVFXSettings.h"
#include "VFXCatalog.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFXShowcaseFilterTest,"ModularGameVFXShowcase.Filters.Composition",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FVFXShowcaseFilterTest::RunTest(const FString&)
{
    FVFXShowcaseEntry Row;Row.DisplayName=FText::FromString(TEXT("Lightning Heavy Impact"));Row.Category=EVFXCategory::Impact;Row.Element=EVFXElement::Lightning;Row.Form=EVFXForm::Burst;Row.Review.Status=EVFXReviewStatus::Fail;
    FVFXShowcaseFilter Filter;Filter.Category=EVFXCategory::Impact;Filter.Element=EVFXElement::Lightning;Filter.Search=TEXT("HEAVY");Filter.bFilterReviewStatus=true;Filter.ReviewStatus=EVFXReviewStatus::Fail;
    TestTrue(TEXT("Category + element + case-insensitive name + status compose"),Row.Matches(Filter,{}));
    Filter.Element=EVFXElement::Ice;TestFalse(TEXT("Mismatched element excluded"),Row.Matches(Filter,{}));
    Filter.Element=EVFXElement::None;Filter.Search=TEXT("Burst");TestTrue(TEXT("Search includes Form metadata"),Row.Matches(Filter,{}));
    Filter.bFavoritesOnly=true;TestFalse(TEXT("Favorite filter is independent"),Row.Matches(Filter,{}));Row.Review.bFavorite=true;TestTrue(TEXT("Favorite included"),Row.Matches(Filter,{}));
    Filter.bRecentOnly=true;TestFalse(TEXT("Never selected item excluded by Recent"),Row.Matches(Filter,{}));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFXShowcaseGateTest,"ModularGameVFXShowcase.Review.SixEvidenceGates",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FVFXShowcaseGateTest::RunTest(const FString&)
{
    FVFXShowcaseReview Review;Review.AssetVersion=TEXT("test-version");
    TestFalse(TEXT("Default review never ready"),Review.IsProductionReady());
    for(uint8 I=0;I<6;++I)
    {
        FVFXReviewEvidence Gate;Gate.Gate=EVFXReviewGate(I);Gate.Result=EVFXGateResult::Approved;Gate.AssetVersion=Review.AssetVersion;Gate.Evidence=TEXT("unit-test evidence only");Gate.Reviewer=TEXT("Automation");Gate.TimestampUtc=FDateTime::UtcNow().ToIso8601();Review.Gates.Add(Gate);
        if(I<5)TestFalse(TEXT("Partial approval cannot grant Production Ready"),Review.IsProductionReady());
    }
    TestTrue(TEXT("Six evidenced current-version gates satisfy predicate"),Review.IsProductionReady());
    Review.Gates[0].Evidence.Reset();TestFalse(TEXT("Empty evidence blocks"),Review.IsProductionReady());Review.Gates[0].Evidence=TEXT("unit-test");
    Review.Gates[0].Reviewer.Reset();TestFalse(TEXT("Missing reviewer blocks"),Review.IsProductionReady());Review.Gates[0].Reviewer=TEXT("Automation");
    Review.AssetVersion=TEXT("modified-asset-version");TestFalse(TEXT("Asset changes invalidate previous approvals"),Review.IsProductionReady());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFXShowcaseStressTest,"ModularGameVFXShowcase.Stress.BoundedInstances",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FVFXShowcaseStressTest::RunTest(const FString&)
{
    TestEqual(TEXT("No negative/empty stress workload"),AVFXShowcaseController::ClampStressCount(-100),1);
    TestEqual(TEXT("Maximum load cannot exceed 100"),AVFXShowcaseController::ClampStressCount(MAX_int32),100);
    for(int32 Count:{1,5,10,20,50,100})TestEqual(TEXT("Requested supported count preserved"),AVFXShowcaseController::ClampStressCount(Count),Count);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFXShowcaseProfileTest,"ModularGameVFXShowcase.Profile.TagLookup",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FVFXShowcaseProfileTest::RunTest(const FString&)
{
    UVFXShowcaseProfile* Profile=NewObject<UVFXShowcaseProfile>();TestNull(TEXT("Empty profile has no implicit entry"),Profile->Find(FGameplayTag()));
    FVFXShowcaseProfileEntry Entry;Entry.PreviewMode=EVFXPreviewMode::Screen;Profile->Entries.Add(Entry);
    TestTrue(TEXT("Explicit screen mode retained"),Profile->Find(FGameplayTag())->PreviewMode==EVFXPreviewMode::Screen);
    TestEqual(TEXT("Default distance uses centimeters"),Entry.PreviewDistance,1000.f);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFXShowcaseLifecycleTest,"ModularGameVFXShowcase.Manager.OwnerLifecycle",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FVFXShowcaseLifecycleTest::RunTest(const FString&)
{
    UVFXCatalog* Catalog=GetDefault<UModularGameVFXSettings>()->DefaultCatalog.LoadSynchronous();
    if(!Catalog||Catalog->Entries.IsEmpty()){AddError(TEXT("Default catalog required for real Manager lifecycle test."));return false;}
    UGameInstance* Instance=NewObject<UGameInstance>(GEngine);Instance->AddToRoot();Instance->InitializeStandalone(FName(TEXT("VFXShowcaseLifecycleTest")));
    UWorld* World=Instance->GetWorld();
    if(!World){AddError(TEXT("Standalone GameInstance did not create a world."));Instance->Shutdown();Instance->RemoveFromRoot();return false;}
    UObject* OwnerA=NewObject<UObject>(Instance);UObject* OwnerB=NewObject<UObject>(Instance);
    FVFXPlayRequest Request;Request.VFXTag=Catalog->Entries[0].VFXTag;Request.Owner=OwnerA;
    const FVFXHandle HandleA=UModularGameVFXBlueprintLibrary::PlayVFX(World,Request,FOnModularVFXReady());
    Request.Owner=OwnerB;
    const FVFXHandle HandleB=UModularGameVFXBlueprintLibrary::PlayVFX(World,Request,FOnModularVFXReady());
    TestTrue(TEXT("Manager allocates valid pending or spawned handle A"),HandleA.IsValid());TestTrue(TEXT("Manager allocates handle B"),HandleB.IsValid());
    TestTrue(TEXT("A is present in Manager registry before stop"),UModularGameVFXBlueprintLibrary::IsVFXHandleValid(World,HandleA));
    TestTrue(TEXT("B is present in Manager registry before stop"),UModularGameVFXBlueprintLibrary::IsVFXHandleValid(World,HandleB));
    UModularGameVFXBlueprintLibrary::StopAllVFXForOwner(World,OwnerA,true);
    TestFalse(TEXT("Owner stop invalidates A"),UModularGameVFXBlueprintLibrary::IsVFXHandleValid(World,HandleA));
    TestTrue(TEXT("Owner stop preserves unrelated B"),UModularGameVFXBlueprintLibrary::IsVFXHandleValid(World,HandleB));
    UModularGameVFXBlueprintLibrary::StopVFXImmediate(World,HandleB);
    int32 Managed=-1,Active=-1;UModularGameVFXBlueprintLibrary::GetVFXRuntimeCounts(World,Managed,Active);
    TestEqual(TEXT("All pending/active handles removed"),Managed,0);TestEqual(TEXT("No active systems after immediate stop"),Active,0);
    Instance->Shutdown();GEngine->DestroyWorldContext(World);World->DestroyWorld(false);Instance->RemoveFromRoot();
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFXShowcasePreviewRegressionTest,"ModularGameVFXShowcase.Preview.SurfaceProjectileAndQuality",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FVFXShowcasePreviewRegressionTest::RunTest(const FString&)
{
    UGameInstance* Instance=NewObject<UGameInstance>(GEngine);Instance->AddToRoot();Instance->InitializeStandalone(FName(TEXT("VFXShowcasePreviewRegression")));
    UWorld* World=Instance->GetWorld();
    if(!World){AddError(TEXT("Standalone world required"));Instance->Shutdown();Instance->RemoveFromRoot();return false;}
    AVFXShowcaseEnvironment* Environment=World->SpawnActor<AVFXShowcaseEnvironment>();
    AVFXShowcaseController* Controller=World->SpawnActor<AVFXShowcaseController>();
    if(!Environment||!Controller)
    {
        AddError(TEXT("Preview actors could not spawn"));Instance->Shutdown();GEngine->DestroyWorldContext(World);World->DestroyWorld(false);Instance->RemoveFromRoot();return false;
    }
    Controller->Environment=Environment;
    Environment->SetSurface(TEXT("Ground"));
    FTransform Surface=Environment->GetSurfaceTransform();
    TestTrue(TEXT("Ground preview lies on actual top face"),Surface.GetLocation().Equals(FVector(1000,0,5.5),.01));
    TestTrue(TEXT("Ground normal faces up"),Surface.GetUnitAxis(EAxis::Z).Equals(FVector::UpVector,.001));
    Environment->SetSurface(TEXT("Wood"));
    const FVector WoodNormal=Environment->GetSurfaceTransform().GetUnitAxis(EAxis::Z);
    Environment->SetSurface(TEXT("Metal"));
    Surface=Environment->GetSurfaceTransform();
    TestTrue(TEXT("Wall normal rotates when surface orientation changes"),!Surface.GetUnitAxis(EAxis::Z).Equals(WoodNormal,.001));
    TestTrue(TEXT("Wall preview does not remain at ordinary target/ground point"),Surface.GetLocation().Y>700&&Surface.GetLocation().Z>100);

    IConsoleVariable* Effects=IConsoleManager::Get().FindConsoleVariable(TEXT("sg.EffectsQuality"));
    IConsoleVariable* Niagara=IConsoleManager::Get().FindConsoleVariable(TEXT("fx.Niagara.QualityLevel"));
    const int32 EffectsValue=Effects?Effects->GetInt():-1,NiagaraValue=Niagara?Niagara->GetInt():-1;
    const uint32 EffectsSource=Effects?uint32(Effects->GetFlags()&ECVF_SetByMask):0,NiagaraSource=Niagara?uint32(Niagara->GetFlags()&ECVF_SetByMask):0;
    Controller->SetQuality(EVFXShowcaseQuality::VRMobile);Controller->SetQuality(EVFXShowcaseQuality::High);
    Controller->EndPlay(EEndPlayReason::Destroyed);
    if(Effects){TestEqual(TEXT("Effects value restored after quality preview"),Effects->GetInt(),EffectsValue);TestEqual(TEXT("Effects priority restored without SetByCode promotion"),uint32(Effects->GetFlags()&ECVF_SetByMask),EffectsSource);}
    if(Niagara){TestEqual(TEXT("Niagara value restored after quality preview"),Niagara->GetInt(),NiagaraValue);TestEqual(TEXT("Niagara priority restored"),uint32(Niagara->GetFlags()&ECVF_SetByMask),NiagaraSource);}

    Controller->RefreshCatalog();
    const TArray<FVFXShowcaseEntry> Rows=Controller->GetEntries();
    const FVFXShowcaseEntry* Projectile=Rows.FindByPredicate([](const FVFXShowcaseEntry& Row){return Row.Category==EVFXCategory::Projectile;});
    if(!Projectile)AddError(TEXT("A registered projectile is required for actual preview regression"));
    else
    {
        Controller->SelectEntry(Projectile->VFXTag,false);Controller->PlayStress(1);Controller->Tick(2.5f);
        TArray<USceneComponent*> Components;Controller->GetComponents<USceneComponent>(Components);
        int32 Anchors=0;
        for(USceneComponent* Component:Components)
            if(Component!=Controller->GetRootComponent()&&Component->GetClass()==USceneComponent::StaticClass())
            {
                ++Anchors;TestTrue(TEXT("Stress projectile reaches its actual advertised 3D target"),Component->GetComponentLocation().Equals(Environment->GetStressPoint()+FVector(300,0,100),.01));
            }
        TestEqual(TEXT("Single projectile has one movement anchor"),Anchors,1);
        Controller->Stop(true);
    }
    Controller->Destroy();Environment->Destroy();Instance->Shutdown();GEngine->DestroyWorldContext(World);World->DestroyWorld(false);Instance->RemoveFromRoot();return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFXShowcaseNotesRegressionTest,"ModularGameVFXShowcase.Review.NotesPreserveGates",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FVFXShowcaseNotesRegressionTest::RunTest(const FString&)
{
    UGameInstance* Instance=NewObject<UGameInstance>(GEngine);Instance->AddToRoot();Instance->InitializeStandalone(FName(TEXT("VFXShowcaseNotesRegression")));
    UWorld* World=Instance->GetWorld();
    if(!World){AddError(TEXT("Standalone world required"));Instance->Shutdown();Instance->RemoveFromRoot();return false;}
    AVFXShowcaseController* Controller=World->SpawnActor<AVFXShowcaseController>();
    if(!Controller){AddError(TEXT("Controller required"));Instance->Shutdown();GEngine->DestroyWorldContext(World);World->DestroyWorld(false);Instance->RemoveFromRoot();return false;}
    Controller->RefreshCatalog();const TArray<FVFXShowcaseEntry> Rows=Controller->GetEntries();
    if(Rows.IsEmpty())AddError(TEXT("Catalog entry required for actual notes edit test"));
    else
    {
        Controller->SelectEntry(Rows[0].VFXTag,false);
        Controller->SetReviewGate(EVFXReviewGate::Visual,EVFXGateResult::Failed,TEXT("SYNTHETIC transient test; not user review"),TEXT("Automation"));
        const FVFXShowcaseReview Before=Controller->GetReview(Rows[0].VFXTag);
        TestTrue(TEXT("Notes method accepts prose without changing gates"),Controller->SetReviewNotes(TEXT("Updated test note"),TEXT("Test reason")));
        const FVFXShowcaseReview After=Controller->GetReview(Rows[0].VFXTag);
        TestTrue(TEXT("Review status remains unchanged"),After.Status==Before.Status);
        TestEqual(TEXT("Gate count retained"),After.Gates.Num(),Before.Gates.Num());
        TestEqual(TEXT("Evidence asset version retained"),After.AssetVersion,Before.AssetVersion);
        if(!Before.Gates.IsEmpty()&&!After.Gates.IsEmpty())
        {TestTrue(TEXT("Failed gate remains failed"),After.Gates[0].Result==Before.Gates[0].Result);TestEqual(TEXT("Evidence retained"),After.Gates[0].Evidence,Before.Gates[0].Evidence);}
        TestEqual(TEXT("New notes visible"),After.Notes,FString(TEXT("Updated test note")));
        TestEqual(TEXT("New failure reason visible"),After.FailReason,FString(TEXT("Test reason")));
        // Intentionally no SaveReviews: synthetic fixtures remain in this actor only.
    }
    Controller->Destroy();Instance->Shutdown();GEngine->DestroyWorldContext(World);World->DestroyWorld(false);Instance->RemoveFromRoot();return true;
}
#endif
