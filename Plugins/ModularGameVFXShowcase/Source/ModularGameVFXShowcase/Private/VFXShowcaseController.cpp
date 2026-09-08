#include "VFXShowcaseController.h"
#include "VFXShowcaseEnvironment.h"
#include "VFXShowcaseWidget.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "VFXCatalog.h"
#include "ModularGameVFXBlueprintLibrary.h"
#include "ModularGameVFXSettings.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraEffectType.h"
#include "NiagaraEmitter.h"
#include "NiagaraEmitterHandle.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "EngineUtils.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "HAL/FileManager.h"
#include "JsonObjectConverter.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/PackageName.h"
#include "Misc/SecureHash.h"
#include "Misc/EngineVersion.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "HAL/PlatformMisc.h"
#include "UnrealClient.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/UObjectIterator.h"

namespace
{
FString VersionFor(const FVFXCatalogEntry& Entry, TMap<FName,FString>& FileHashes)
{
    FString Serialized;
    FJsonObjectConverter::UStructToJsonObjectString(Entry,Serialized);
    Serialized+=FEngineVersion::Current().ToString();
    IAssetRegistry& Registry=FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
    TSet<FName> Visited;
    TArray<FName> Pending{FName(*Entry.NiagaraSystem.ToSoftObjectPath().GetLongPackageName())};
    while(!Pending.IsEmpty())
    {
        const FName Package=Pending.Pop(EAllowShrinking::No);if(Visited.Contains(Package))continue;Visited.Add(Package);
        const FString Path=Package.ToString();
        // Engine assets are covered by engine build version. Content dependencies include foreign project paths too.
        if(Path.StartsWith(TEXT("/Engine/"))||Path.StartsWith(TEXT("/Script/")))continue;
        TArray<FName> Dependencies;Registry.GetDependencies(Package,Dependencies);Pending.Append(Dependencies);
        if(!FileHashes.Contains(Package))
        {
            FString Filename;FString Hash=TEXT("missing-file");
            if(FPackageName::TryConvertLongPackageNameToFilename(Path,Filename,FPackageName::GetAssetPackageExtension())&&IFileManager::Get().FileExists(*Filename))
                Hash=LexToString(FMD5Hash::HashFile(*Filename));
            FileHashes.Add(Package,Hash);
        }
    }
    TArray<FName> Sorted=Visited.Array();Sorted.Sort(FNameLexicalLess());
    for(FName Package:Sorted)Serialized+=Package.ToString()+TEXT(":")+FileHashes.FindRef(Package)+TEXT(";");
    return FMD5::HashAnsiString(*Serialized);
}
int32 QualityIndex(EVFXShowcaseQuality Value)
{
    switch(Value){case EVFXShowcaseQuality::Cinematic:return 4;case EVFXShowcaseQuality::High:return 3;case EVFXShowcaseQuality::Medium:return 2;case EVFXShowcaseQuality::Low:return 1;default:return 0;}
}
}
AVFXShowcaseController::AVFXShowcaseController()
{
    PrimaryActorTick.bCanEverTick=true;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("ShowcaseRoot")));
}
void AVFXShowcaseController::BeginPlay()
{
    Super::BeginPlay();
    if(!Environment)
        for(TActorIterator<AVFXShowcaseEnvironment> It(GetWorld());It;++It){Environment=*It;break;}
    if(!Environment) Environment=GetWorld()->SpawnActor<AVFXShowcaseEnvironment>();
    RefreshCatalog();
    LastTickTime=FPlatformTime::Seconds();
    SavedTimeDilation=UGameplayStatics::GetGlobalTimeDilation(this);
    if(APlayerController* PC=UGameplayStatics::GetPlayerController(this,0))
    {
        if(PC->IsLocalController())
        {
            TSubclassOf<UUserWidget> Class=WidgetClass;if(!Class)Class=UVFXShowcaseWidget::StaticClass();
            LiveWidget=CreateWidget<UUserWidget>(PC,Class);
            if(UVFXShowcaseWidget* Widget=Cast<UVFXShowcaseWidget>(LiveWidget)) Widget->Controller=this;
            if(LiveWidget) LiveWidget->AddToViewport();
            PC->bShowMouseCursor=true;
            FInputModeGameAndUI InputMode; InputMode.SetHideCursorDuringCapture(false); PC->SetInputMode(InputMode);
            if(Environment) PC->SetViewTarget(Environment);
        }
    }
    if(FParse::Param(FCommandLine::Get(),TEXT("VFXShowcaseSmokeTest"))) StartSmokeTest();
}
void AVFXShowcaseController::EndPlay(const EEndPlayReason::Type Reason)
{
    ClearPlayback(true);
    if(LiveWidget) LiveWidget->RemoveFromParent();
    if(bChangedTimeDilation) UGameplayStatics::SetGlobalTimeDilation(this,SavedTimeDilation);
    if(bQualityOverrideActive)
    {
        // Remove only this actor's temporary history entries. Original values,
        // source priorities and other actors' overrides remain in the CVar history.
        IConsoleManager::Get().UnsetAllConsoleVariablesWithTag(QualityOverrideTag,ECVF_SetByTemp);
        bQualityOverrideActive=false;
    }
    if(GetWorld()) for(FTimerHandle& Timer:SmokeTimers) GetWorld()->GetTimerManager().ClearTimer(Timer);
    SmokeTimers.Reset();
    Super::EndPlay(Reason);
}
void AVFXShowcaseController::RefreshCatalog()
{
    const TSoftObjectPtr<UVFXCatalog> DefaultCatalog=GetDefault<UModularGameVFXSettings>()->DefaultCatalog;
    if(Catalog.IsNull()) Catalog=DefaultCatalog;
    LoadedCatalog=Catalog.LoadSynchronous();
    AllEntries.Reset();
    if(!LoadedCatalog){LastMessage=TEXT("Catalog missing. Configure Modular Game VFX DefaultCatalog.");OnChanged.Broadcast();return;}
    LoadedCatalog->BuildRuntimeCache();
    LoadReviews();
    TMap<FGameplayTag,int32> Counts;
    for(const FVFXCatalogEntry& Entry:LoadedCatalog->Entries) Counts.FindOrAdd(Entry.VFXTag)++;
    TMap<FName,FString> VersionFileHashes;
    for(const FVFXCatalogEntry& Entry:LoadedCatalog->Entries)
    {
        FVFXShowcaseEntry Row;
        Row.VFXTag=Entry.VFXTag;Row.DisplayName=Entry.DisplayName;Row.AssetPath=Entry.NiagaraSystem.ToString();Row.AssetVersion=VersionFor(Entry,VersionFileHashes);
        Row.Category=Entry.Category;Row.Element=Entry.Element;Row.Form=Entry.Form;Row.Context=Entry.Context;Row.Lifecycle=Entry.Lifecycle;
        Row.Priority=Entry.PerformanceClass;Row.SpawnMode=Entry.SpawnMode;Row.DefaultParameters=Entry.DefaultParameters;
        Row.Pooling=StaticEnum<ENCPoolMethod>()->GetNameStringByValue(int64(Entry.PoolingMethod));
        Row.PreviewMode=ResolveProfile(Entry).PreviewMode;
        if(!Entry.VFXTag.IsValid()) Row.ValidationIssues.Add(TEXT("Invalid Tag"));
        if(Counts.FindRef(Entry.VFXTag)>1) Row.ValidationIssues.Add(TEXT("Duplicate Tag"));
        if(Entry.Category==EVFXCategory::Unspecified || !StaticEnum<EVFXCategory>()->IsValidEnumValue(int64(Entry.Category))) Row.ValidationIssues.Add(TEXT("Invalid Category"));
        if(!Profile || !Profile->Find(Entry.VFXTag)) Row.ValidationIssues.Add(TEXT("Missing Preview Profile (metadata fallback active)"));
        if(Catalog.ToSoftObjectPath()!=DefaultCatalog.ToSoftObjectPath()) Row.ValidationIssues.Add(TEXT("Catalog differs from Manager DefaultCatalog; playback disabled"));
        if(UNiagaraSystem* System=Entry.NiagaraSystem.LoadSynchronous())
        {
            Row.EmitterCount=System->GetEmitterHandles().Num();
            Row.EffectType=System->GetEffectType()?System->GetEffectType()->GetPathName():TEXT("Missing");
            if(!System->GetEffectType()) Row.ValidationIssues.Add(TEXT("Missing Effect Type"));
            Row.FixedBounds=System->GetFixedBounds().ToString();
            bool bCPU=false,bGPU=false;
            for(const FNiagaraEmitterHandle& Handle:System->GetEmitterHandles())
            {
                if(const FVersionedNiagaraEmitterData* Data=Handle.GetInstance().GetEmitterData())
                {bCPU|=Data->SimTarget==ENiagaraSimTarget::CPUSim;bGPU|=Data->SimTarget==ENiagaraSimTarget::GPUComputeSim;}
            }
            Row.SimulationTarget=bCPU&&bGPU?TEXT("CPU + GPU"):bGPU?TEXT("GPU"):bCPU?TEXT("CPU"):TEXT("Unavailable");
            TArray<FNiagaraVariable> Variables; System->GetExposedParameters().GetParameters(Variables);
            for(const FNiagaraVariable& Variable:Variables) Row.SupportedParameters.Add(Variable.GetName());
        }
        else Row.ValidationIssues.Add(TEXT("Missing Niagara"));
        AllEntries.Add(MoveTemp(Row));
    }
    UpdateReviewRows();
    if(!FindRow(SelectedTag)) SelectedTag=AllEntries.Num()?AllEntries[0].VFXTag:FGameplayTag();
    if(const FVFXShowcaseEntry* Row=FindRow(SelectedTag)) Parameters=Row->DefaultParameters;
    LastMessage=FString::Printf(TEXT("Catalog refreshed: %d entries. Reviews require six evidence-backed approvals."),AllEntries.Num());
    OnChanged.Broadcast();
}
FVFXShowcaseProfileEntry AVFXShowcaseController::ResolveProfile(const FVFXCatalogEntry& Entry) const
{
    if(Profile) if(const FVFXShowcaseProfileEntry* Found=Profile->Find(Entry.VFXTag)) return *Found;
    FVFXShowcaseProfileEntry Out;Out.VFXTag=Entry.VFXTag;
    switch(Entry.Category)
    {
    case EVFXCategory::Cast:case EVFXCategory::Trail:case EVFXCategory::Status:case EVFXCategory::Character:Out.PreviewMode=EVFXPreviewMode::Attached;break;
    case EVFXCategory::Projectile:Out.PreviewMode=EVFXPreviewMode::Projectile;Out.PreviewDistance=2000;break;
    case EVFXCategory::Beam:Out.PreviewMode=EVFXPreviewMode::Beam;break;
    case EVFXCategory::Area:case EVFXCategory::Indicator:Out.PreviewMode=EVFXPreviewMode::Area;break;
    case EVFXCategory::UI:Out.PreviewMode=EVFXPreviewMode::Screen;break;
    case EVFXCategory::World:Out.PreviewMode=EVFXPreviewMode::Environment;break;
    default:Out.PreviewMode=Entry.SpawnMode==EVFXSpawnMode::Attached?EVFXPreviewMode::Attached:EVFXPreviewMode::AtLocation;break;
    }
    return Out;
}
const FVFXShowcaseEntry* AVFXShowcaseController::FindRow(FGameplayTag Tag) const
{return AllEntries.FindByPredicate([Tag](const FVFXShowcaseEntry& Entry){return Entry.VFXTag==Tag;});}
void AVFXShowcaseController::SetFilter(const FVFXShowcaseFilter& NewFilter){Filter=NewFilter;OnChanged.Broadcast();}
TArray<FVFXShowcaseEntry> AVFXShowcaseController::GetEntries() const
{
    TArray<FVFXShowcaseEntry> Out;
    for(const FVFXShowcaseEntry& Entry:AllEntries) if(Entry.Matches(Filter,RecentTags)) Out.Add(Entry);
    const EVFXShowcaseSort Sort=Filter.Sort;
    Out.Sort([Sort](const FVFXShowcaseEntry& A,const FVFXShowcaseEntry& B)
    {
        if(Sort==EVFXShowcaseSort::Category && A.Category!=B.Category) return A.Category<B.Category;
        if((Sort==EVFXShowcaseSort::Element||Sort==EVFXShowcaseSort::Category) && A.Element!=B.Element) return A.Element<B.Element;
        if(Sort==EVFXShowcaseSort::Priority && A.Priority!=B.Priority) return A.Priority<B.Priority;
        if(Sort==EVFXShowcaseSort::ReviewStatus && A.Review.Status!=B.Review.Status) return A.Review.Status<B.Review.Status;
        return A.DisplayName.ToString()<B.DisplayName.ToString();
    });
    return Out;
}
TArray<FVFXShowcaseCategoryProgress> AVFXShowcaseController::GetCategories() const
{
    TArray<FVFXShowcaseCategoryProgress> Out;
    // Read the runtime taxonomy dynamically: future enum members appear without widget edits.
    const UEnum* Enum=StaticEnum<EVFXCategory>();
    for(int32 Index=0;Index<Enum->NumEnums()-1;++Index)
    {FVFXShowcaseCategoryProgress P;P.Category=EVFXCategory(Enum->GetValueByIndex(Index));Out.Add(P);}
    for(const FVFXShowcaseEntry& Entry:AllEntries)
        for(FVFXShowcaseCategoryProgress& P:Out)
            if(P.Category==EVFXCategory::Unspecified||P.Category==Entry.Category)
            {P.Total++;P.Reviewed+=Entry.Review.Status!=EVFXReviewStatus::NotTested;P.Ready+=Entry.Review.Status==EVFXReviewStatus::ProductionReady;P.Failed+=Entry.Review.Status==EVFXReviewStatus::Fail||Entry.Review.Status==EVFXReviewStatus::NeedsOptimization||Entry.Review.Status==EVFXReviewStatus::NeedsVisualRework;}
    return Out;
}
TArray<EVFXElement> AVFXShowcaseController::GetElements() const
{
    TArray<EVFXElement> Out{EVFXElement::None};
    for(const FVFXShowcaseEntry& Entry:AllEntries) Out.AddUnique(Entry.Element);
    Out.Sort([](EVFXElement A,EVFXElement B){return A<B;});return Out;
}
bool AVFXShowcaseController::GetSelectedEntry(FVFXShowcaseEntry& Entry) const
{if(const FVFXShowcaseEntry* Found=FindRow(SelectedTag)){Entry=*Found;return true;}return false;}
bool AVFXShowcaseController::SelectEntry(FGameplayTag Tag,bool bPlay)
{
    if(!FindRow(Tag)){LastMessage=TEXT("Selection rejected: tag not present in this catalog.");return false;}
    ClearPlayback(true);SelectedTag=Tag;CompareTag=FGameplayTag();
    Parameters=FindRow(Tag)->DefaultParameters;
    if(LoadedCatalog) if(const FVFXCatalogEntry* Entry=LoadedCatalog->FindEntryNative(Tag)) PreviewDistance=ResolveProfile(*Entry).PreviewDistance;
    RecentTags.Remove(Tag);RecentTags.Insert(Tag,0);if(RecentTags.Num()>30)RecentTags.SetNum(30);
    SequenceElapsed=0;OnChanged.Broadcast();return !bPlay||Play();
}
bool AVFXShowcaseController::PlayByTag(FGameplayTag Tag){return SelectEntry(Tag,true);}
bool AVFXShowcaseController::Play(){return Replay();}
bool AVFXShowcaseController::Replay()
{
    ClearPlayback(true);SequenceElapsed=0;MotionElapsed=0;TestCount=1;
    if(Environment)Environment->FocusStress(false);
    const bool bSuccess=SpawnPreview(SelectedTag,FVector::ZeroVector,false).IsValid();OnChanged.Broadcast();return bSuccess;
}
void AVFXShowcaseController::ClearPlayback(bool bImmediate)
{
    UModularGameVFXBlueprintLibrary::StopAllVFXForOwner(this,this,bImmediate);
    Handles.Reset();MovingAnchors.Reset();AnchorStartTimes.Reset();AnchorStartLocations.Reset();AnchorTargetLocations.Reset();
    // Attached systems must finish before their anchors are destroyed on graceful stop.
    if(bImmediate){for(USceneComponent* Anchor:PreviewAnchors)if(Anchor)Anchor->DestroyComponent();PreviewAnchors.Reset();}
    TestCount=0;
}
void AVFXShowcaseController::Stop(bool bImmediate)
{bLoop=false;bAutoPreview=false;ClearPlayback(bImmediate);LastMessage=bImmediate?TEXT("Stopped immediately."):TEXT("Stopped; allowing system completion.");OnChanged.Broadcast();}
FVFXHandle AVFXShowcaseController::SpawnPreview(FGameplayTag Tag,const FVector& Offset,bool bStress)
{
    if(!GetWorld()||!GetWorld()->GetGameInstance()||!Environment||!LoadedCatalog)return FVFXHandle();
    if(Catalog.ToSoftObjectPath()!=GetDefault<UModularGameVFXSettings>()->DefaultCatalog.ToSoftObjectPath())
    {LastMessage=TEXT("Playback blocked: Showcase catalog must equal Manager DefaultCatalog.");return FVFXHandle();}
    const FVFXCatalogEntry* Entry=LoadedCatalog->FindEntryNative(Tag);if(!Entry)return FVFXHandle();
    const FVFXShowcaseProfileEntry Preview=ResolveProfile(*Entry);
    FVFXPlayRequest Request;Request.VFXTag=Tag;Request.Owner=this;Request.ParameterOverrides=Tag==SelectedTag?Parameters:Entry->DefaultParameters;
    FVector Source=Environment->GetOriginPoint()+Offset;
    FVector Target=Environment->GetTargetPoint(PreviewDistance)+Offset;
    if(bStress){Source=Environment->GetStressPoint()+Offset;Target=Source+FVector(300,0,100);}
    Request.Transform=FTransform(FRotator::ZeroRotator,Target);
    USceneComponent* Anchor=nullptr;
    switch(Preview.PreviewMode)
    {
    case EVFXPreviewMode::Area:Request.Transform=FTransform(FRotator::ZeroRotator,bStress?Source:Environment->GetGroundPoint()+Offset);break;
    case EVFXPreviewMode::Environment:Request.Transform=FTransform(FRotator::ZeroRotator,bStress?Source:Environment->GetEnvironmentPoint()+Offset);if(!bStress)Environment->FocusEnvironment();break;
    case EVFXPreviewMode::Beam:Request.Transform=FTransform((Target-Source).Rotation(),Source);break;
    case EVFXPreviewMode::Screen:
        Request.SpawnMode=EVFXSpawnMode::Attached;Request.AttachComponent=Environment->ScreenAnchor;Request.Transform=FTransform(FRotator::ZeroRotator,Offset*.02f,FVector(.2));break;
    case EVFXPreviewMode::Attached:
    case EVFXPreviewMode::Projectile:
        Anchor=NewObject<USceneComponent>(this);Anchor->RegisterComponent();Anchor->AttachToComponent(GetRootComponent(),FAttachmentTransformRules::KeepWorldTransform);PreviewAnchors.Add(Anchor);
        if(Preview.PreviewMode==EVFXPreviewMode::Attached&&!bStress&&Environment->Character)
        {
            const FName Name=Entry->Category==EVFXCategory::Trail?FName(TEXT("weapon")):Entry->DefaultSocketName.IsNone()?FName(TEXT("hand_r")):Entry->DefaultSocketName;
            Anchor->AttachToComponent(Environment->Character->GetAnchor(Name),FAttachmentTransformRules::SnapToTargetNotIncludingScale);Anchor->SetRelativeLocation(Offset);
        }
        else Anchor->SetWorldLocation(Source);
        Request.SpawnMode=EVFXSpawnMode::Attached;Request.AttachComponent=Anchor;Request.Transform=FTransform::Identity;break;
    default:break;
    }
    const bool bEndpointMode=Preview.PreviewMode==EVFXPreviewMode::Projectile||Preview.PreviewMode==EVFXPreviewMode::Beam;
    if(bEndpointMode)
    {
        Request.ParameterOverrides.VectorParameters.Add(TEXT("User.SourceLocation"),Source);
        Request.ParameterOverrides.VectorParameters.Add(TEXT("User.TargetLocation"),Target);
        Request.ParameterOverrides.VectorParameters.Add(TEXT("User.Direction"),(Target-Source).GetSafeNormal());
        if(!Request.ParameterOverrides.FloatParameters.Contains(TEXT("User.Length"))
            && !Entry->DefaultParameters.FloatParameters.Contains(TEXT("User.Length")))
            Request.ParameterOverrides.FloatParameters.Add(TEXT("User.Length"),FVector::Distance(Source,Target));
    }
    const bool bSurfaceTarget=Entry->Category==EVFXCategory::Surface||Preview.TargetType==TEXT("Surface")
        ||Preview.TargetType==TEXT("SurfaceWall")||Preview.TargetType==TEXT("Stone")
        ||Preview.TargetType==TEXT("Metal")||Preview.TargetType==TEXT("Wood")||Preview.TargetType==TEXT("Wall");
    if(bSurfaceTarget)
    {
        Request.SpawnMode=EVFXSpawnMode::AtLocation;Request.AttachComponent=nullptr;
        Request.Transform=Environment->GetSurfaceTransform();
        Request.Transform.AddToTranslation(Request.Transform.TransformVectorNoScale(FVector(Offset.X,Offset.Y,0)));
        Request.ParameterOverrides.VectorParameters.Add(TEXT("User.SurfaceNormal"),Request.Transform.GetUnitAxis(EAxis::Z));
        if(Anchor){PreviewAnchors.Remove(Anchor);Anchor->DestroyComponent();Anchor=nullptr;}
    }
    FOnModularVFXReady Callback;Callback.BindDynamic(this,&AVFXShowcaseController::OnVFXReady);
    const FVFXHandle Handle=UModularGameVFXBlueprintLibrary::PlayVFX(this,Request,Callback);
    if(Handle.IsValid())
    {
        Handles.Add(Handle);LastMessage=TEXT("Requested through GameplayTag -> Catalog -> VFX Manager.");
        if(Anchor&&Preview.PreviewMode==EVFXPreviewMode::Projectile){MovingAnchors.Add(Handle.Id,Anchor);AnchorStartTimes.Add(Handle.Id,MotionElapsed);AnchorStartLocations.Add(Handle.Id,Source);AnchorTargetLocations.Add(Handle.Id,Target);}
    }
    return Handle;
}
void AVFXShowcaseController::OnVFXReady(FVFXHandle Handle,bool bSuccess)
{LastMessage=bSuccess?TEXT("VFX Manager confirmed successful spawn."):TEXT("VFX Manager failed to spawn selected tag; inspect catalog/asset validation.");OnChanged.Broadcast();}
void AVFXShowcaseController::SetLoop(bool bEnabled)
{
    bLoop=bEnabled;
    if(LoadedCatalog)if(const FVFXCatalogEntry* Entry=LoadedCatalog->FindEntryNative(SelectedTag))
        if(bLoop&&!ResolveProfile(*Entry).bLoopAllowed){bLoop=false;LastMessage=TEXT("Loop disabled by preview profile.");}
    if(bLoop){bAutoPreview=false;Replay();}OnChanged.Broadcast();
}
void AVFXShowcaseController::SetAutoPreview(bool bEnabled){bAutoPreview=bEnabled;if(bEnabled){bLoop=false;Replay();}OnChanged.Broadcast();}
void AVFXShowcaseController::Advance(int32 Direction)
{
    const TArray<FVFXShowcaseEntry> Rows=GetEntries();if(Rows.IsEmpty())return;
    int32 Index=Rows.IndexOfByPredicate([this](const FVFXShowcaseEntry& E){return E.VFXTag==SelectedTag;});
    Index=Index==INDEX_NONE?(Direction>0?0:Rows.Num()-1):(Index+Direction+Rows.Num())%Rows.Num();SelectEntry(Rows[Index].VFXTag,true);
}
void AVFXShowcaseController::Next(){Advance(1);}
void AVFXShowcaseController::Previous(){Advance(-1);}
void AVFXShowcaseController::SetFloatParameter(FName Name,float Value)
{
    if(!FMath::IsFinite(Value)||!Name.ToString().StartsWith(TEXT("User.")))return;
    if(Name==TEXT("User.Scale")||Name==TEXT("User.Intensity")||Name==TEXT("User.LifetimeScale"))Value=FMath::Clamp(Value,.01f,20.f);
    Parameters.FloatParameters.Add(Name,Value);for(const FVFXHandle& H:Handles)UModularGameVFXBlueprintLibrary::SetVFXFloatParameter(this,H,Name,Value);OnChanged.Broadcast();
}
void AVFXShowcaseController::SetColorParameter(FName Name,FLinearColor Value)
{if(!Name.ToString().StartsWith(TEXT("User.")))return;Parameters.ColorParameters.Add(Name,Value);for(const FVFXHandle& H:Handles)UModularGameVFXBlueprintLibrary::SetVFXColorParameter(this,H,Name,Value);OnChanged.Broadcast();}
void AVFXShowcaseController::ResetParameters(){if(const FVFXShowcaseEntry* Row=FindRow(SelectedTag)){Parameters=Row->DefaultParameters;Replay();}}
void AVFXShowcaseController::SetQuality(EVFXShowcaseQuality NewQuality)
{
    Quality=NewQuality;const int32 Index=QualityIndex(Quality);
    if(QualityOverrideTag.IsNone()) QualityOverrideTag=FName(*(TEXT("VFXShowcaseQuality_")+FGuid::NewGuid().ToString(EGuidFormats::Digits)));
    for(const TCHAR* Name:{TEXT("sg.EffectsQuality"),TEXT("fx.Niagara.QualityLevel")})
        if(IConsoleVariable* CVar=IConsoleManager::Get().FindConsoleVariable(Name))
        {
            CVar->Set(Index,EConsoleVariableFlags(ECVF_SetByTemp|ECVF_Set_ReplaceExistingTag),QualityOverrideTag);
            bQualityOverrideActive=true;
        }
    Replay();
}
void AVFXShowcaseController::SetBackground(EVFXShowcaseBackground NewBackground){Background=NewBackground;if(Environment)Environment->SetBackground(Background);OnChanged.Broadcast();}
void AVFXShowcaseController::SetPlaybackSpeed(float Speed){PlaybackSpeed=FMath::Clamp(Speed,.1f,1.f);UGameplayStatics::SetGlobalTimeDilation(this,PlaybackSpeed);bChangedTimeDilation=true;OnChanged.Broadcast();}
void AVFXShowcaseController::SetPreviewDistance(float DistanceCm){PreviewDistance=FMath::Clamp(DistanceCm,100.f,5000.f);if(Environment&&Environment->Target)Environment->Target->SetActorLocation(Environment->GetTargetPoint(PreviewDistance));Replay();}
void AVFXShowcaseController::SetCameraDistance(float DistanceCm){if(Environment)Environment->SetCameraDistance(DistanceCm);}
void AVFXShowcaseController::SetVRPreview(bool bEnabled){bVRPreview=bEnabled;if(bEnabled)SetQuality(EVFXShowcaseQuality::VRMobile);LastMessage=TEXT("VRMobile budget preview; actual headset, stereo and comfort review still required.");OnChanged.Broadcast();}
void AVFXShowcaseController::SetMotion(EVFXShowcaseMotion Motion){CurrentMotion=Motion;MotionElapsed=0;}
void AVFXShowcaseController::SetSurface(FName Surface){if(Environment){Environment->SetSurface(Surface);Replay();}}
void AVFXShowcaseController::PlayStress(int32 Count)
{
    bLoop=false;bAutoPreview=false;ClearPlayback(true);TestCount=ClampStressCount(Count);MotionElapsed=0;
    const int32 Columns=FMath::CeilToInt(FMath::Sqrt(float(TestCount)));
    const int32 Rows=FMath::DivideAndRoundUp(TestCount,Columns);
    if(Environment)Environment->FrameStressGrid(Columns,Rows,300.f);
    for(int32 I=0;I<TestCount;++I)SpawnPreview(SelectedTag,FVector(I%Columns*300,I/Columns*300,0),true);
    OnChanged.Broadcast();
}
void AVFXShowcaseController::PlayCompare(FGameplayTag OtherTag)
{
    if(!FindRow(OtherTag))return;bLoop=false;bAutoPreview=false;ClearPlayback(true);CompareTag=OtherTag;TestCount=2;
    if(Environment)Environment->FocusStress(false);
    SpawnPreview(SelectedTag,FVector(0,-300,0),false);SpawnPreview(OtherTag,FVector(0,300,0),false);OnChanged.Broadcast();
}
void AVFXShowcaseController::PlayCombatSimulation()
{
    bLoop=false;bAutoPreview=false;ClearPlayback(true);MotionElapsed=0;if(Environment)Environment->FocusStress(true);
    for(EVFXCategory Category:{EVFXCategory::Projectile,EVFXCategory::Impact,EVFXCategory::Trail,EVFXCategory::Status,EVFXCategory::Area})
        if(const FVFXShowcaseEntry* Row=AllEntries.FindByPredicate([Category](const FVFXShowcaseEntry& E){return E.Category==Category;}))
        {SpawnPreview(Row->VFXTag,FVector((TestCount%3)*400,(TestCount/3)*400,0),true);++TestCount;}
    OnChanged.Broadcast();
}
void AVFXShowcaseController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);const double Now=FPlatformTime::Seconds();const double RealDelta=LastTickTime>0?Now-LastTickTime:DeltaSeconds;LastTickTime=Now;
    if(RealDelta>0){Performance.FrameTimeMs=FMath::Lerp(Performance.FrameTimeMs,float(RealDelta*1000),.1f);Performance.FPS=1000.f/FMath::Max(Performance.FrameTimeMs,.01f);}
    UModularGameVFXBlueprintLibrary::GetVFXRuntimeCounts(this,Performance.ManagedVFXCount,Performance.ActiveVFXCount);
    Performance.ActiveNiagaraSystems=0;for(TObjectIterator<UNiagaraComponent> It;It;++It)if(It->GetWorld()==GetWorld()&&It->IsActive())++Performance.ActiveNiagaraSystems;
    Performance.CurrentTestCount=TestCount;Performance.Quality=Quality;
    MotionElapsed+=DeltaSeconds;SequenceElapsed+=DeltaSeconds;
    for(auto& Pair:MovingAnchors)
        if(USceneComponent* Anchor=Pair.Value.Get())
        {
            // One trip per instance, with endpoint retained for impact/beam observation.
            const float Age=MotionElapsed-AnchorStartTimes.FindRef(Pair.Key);
            const FVector Origin=AnchorStartLocations.FindRef(Pair.Key);
            const FVector Target=AnchorTargetLocations.FindRef(Pair.Key);
            Anchor->SetWorldLocation(FMath::Lerp(Origin,Target,FMath::Clamp(Age/2.f,0.f,1.f)));
        }
    if(Environment&&Environment->Character)
    {
        USceneComponent* Weapon=Environment->Character->GetAnchor(TEXT("weapon"));
        const float Phase=MotionElapsed*3;
        FVector Position(0,90,120);FRotator Rotation=FRotator::ZeroRotator;
        switch(CurrentMotion)
        {
        case EVFXShowcaseMotion::SwingHorizontal:Position+=FVector(FMath::Sin(Phase)*120,0,0);break;
        case EVFXShowcaseMotion::SwingVertical:Position+=FVector(0,0,FMath::Sin(Phase)*100);break;
        case EVFXShowcaseMotion::Spin:Position=FVector(FMath::Cos(Phase)*150,FMath::Sin(Phase)*150,120);break;
        case EVFXShowcaseMotion::Dash:Position.X+=FMath::Sin(Phase)*400;break;
        case EVFXShowcaseMotion::Jump:Position.Z+=FMath::Abs(FMath::Sin(Phase))*200;break;
        case EVFXShowcaseMotion::Land:Position.Z-=FMath::Abs(FMath::Sin(Phase))*100;break;
        }
        Weapon->SetRelativeLocation(Position);
    }
    if((bLoop||bAutoPreview)&&LoadedCatalog)
        if(const FVFXCatalogEntry* Entry=LoadedCatalog->FindEntryNative(SelectedTag))
            if(SequenceElapsed>=FMath::Max(.1f,ResolveProfile(*Entry).PreviewDuration)){if(bAutoPreview)Next();else Replay();}
}
FVFXShowcasePerformance AVFXShowcaseController::GetPerformance() const{return Performance;}

FVFXShowcaseReview AVFXShowcaseController::GetReview(FGameplayTag Tag) const
{
    if(const FVFXShowcaseEntry* Row=FindRow(Tag))return Row->Review;
    FVFXShowcaseReview Empty;Empty.VFXTag=Tag;return Empty;
}
FVFXShowcaseReview* AVFXShowcaseController::FindOrAddReview(FGameplayTag Tag)
{
    const FVFXShowcaseEntry* Row=FindRow(Tag);if(!Row)return nullptr;
    FVFXShowcaseReview* Review=Reviews.FindByPredicate([Tag](const FVFXShowcaseReview& Item){return Item.VFXTag==Tag;});
    if(!Review){FVFXShowcaseReview New;New.VFXTag=Tag;New.AssetVersion=Row->AssetVersion;Reviews.Add(New);Review=&Reviews.Last();}
    if(Review->AssetVersion!=Row->AssetVersion)
    {
        // Keep old evidence for audit, but it cannot approve the current version.
        Review->AssetVersion=Row->AssetVersion;Review->Status=EVFXReviewStatus::NotTested;
    }
    return Review;
}
void AVFXShowcaseController::UpdateReviewRows()
{
    for(FVFXShowcaseEntry& Row:AllEntries)
    {
        if(const FVFXShowcaseReview* Saved=Reviews.FindByPredicate([&Row](const FVFXShowcaseReview& Item){return Item.VFXTag==Row.VFXTag;}))
        {
            Row.Review=*Saved;
            if(Row.Review.AssetVersion!=Row.AssetVersion){Row.Review.Status=EVFXReviewStatus::NotTested;Row.Review.AssetVersion=Row.AssetVersion;}
            if(Row.Review.Status==EVFXReviewStatus::ProductionReady&&!Row.Review.IsProductionReady())Row.Review.Status=EVFXReviewStatus::NotTested;
        }
        else {Row.Review=FVFXShowcaseReview();Row.Review.VFXTag=Row.VFXTag;Row.Review.AssetVersion=Row.AssetVersion;}
    }
}
bool AVFXShowcaseController::SetReviewGate(EVFXReviewGate Gate,EVFXGateResult Result,const FString& Evidence,const FString& Reviewer)
{
    if(!StaticEnum<EVFXReviewGate>()->IsValidEnumValue(int64(Gate))||!StaticEnum<EVFXGateResult>()->IsValidEnumValue(int64(Result)))return false;
    if(Result!=EVFXGateResult::NotAssessed&&(Evidence.TrimStartAndEnd().IsEmpty()||Reviewer.TrimStartAndEnd().IsEmpty()))
    {LastMessage=TEXT("Review rejected: a real evidence reference and reviewer/tool identity are required.");OnChanged.Broadcast();return false;}
    FVFXShowcaseReview* Review=FindOrAddReview(SelectedTag);if(!Review)return false;
    FVFXReviewEvidence* Item=Review->Gates.FindByPredicate([Gate](const FVFXReviewEvidence& G){return G.Gate==Gate;});
    if(!Item){FVFXReviewEvidence New;New.Gate=Gate;Review->Gates.Add(New);Item=&Review->Gates.Last();}
    Item->Result=Result;Item->Evidence=Evidence;Item->Reviewer=Reviewer;Item->TimestampUtc=FDateTime::UtcNow().ToIso8601();Item->AssetVersion=Review->AssetVersion;
    Review->TimestampUtc=Item->TimestampUtc;
    if(Result==EVFXGateResult::Failed)Review->Status=EVFXReviewStatus::Fail;
    else if(Review->Status==EVFXReviewStatus::ProductionReady&&!Review->IsProductionReady())Review->Status=EVFXReviewStatus::NotTested;
    UpdateReviewRows();LastMessage=TEXT("Gate recorded in memory. Save Reviews to persist.");OnChanged.Broadcast();return true;
}
bool AVFXShowcaseController::SetReviewStatus(EVFXReviewStatus Status,const FString& Notes,const FString& FailReason)
{
    FVFXShowcaseReview* Review=FindOrAddReview(SelectedTag);if(!Review)return false;
    if(!StaticEnum<EVFXReviewStatus>()->IsValidEnumValue(int64(Status)))return false;
    if(Status==EVFXReviewStatus::ProductionReady&&!Review->IsProductionReady())
    {LastMessage=TEXT("Final PASS blocked: Visual, Gameplay, Performance, Naming, Catalog and Dependency need current-version approval with evidence, reviewer and UTC time.");OnChanged.Broadcast();return false;}
    if(Status==EVFXReviewStatus::Pass)
    {
        for(EVFXReviewGate Gate:{EVFXReviewGate::Visual,EVFXReviewGate::Gameplay,EVFXReviewGate::Performance})
        {
            const FVFXReviewEvidence* Item=Review->Gates.FindByPredicate([Gate](const FVFXReviewEvidence& G){return G.Gate==Gate;});
            if(!Item||!Item->HasApproval(Review->AssetVersion))
            {LastMessage=TEXT("PASS blocked: three human review gates require evidence. Production Ready additionally requires Naming, Catalog and Dependency.");OnChanged.Broadcast();return false;}
        }
    }
    Review->Status=Status;Review->Notes=Notes;Review->FailReason=FailReason;Review->TimestampUtc=FDateTime::UtcNow().ToIso8601();
    if(Status==EVFXReviewStatus::NotTested) for(FVFXReviewEvidence& Gate:Review->Gates) Gate.Result=EVFXGateResult::NotAssessed;
    UpdateReviewRows();LastMessage=TEXT("Review updated. Save Reviews to persist.");OnChanged.Broadcast();return true;
}
bool AVFXShowcaseController::SetReviewNotes(const FString& Notes,const FString& FailReason)
{
    const FVFXShowcaseEntry* Row=FindRow(SelectedTag);if(!Row)return false;
    FVFXShowcaseReview* Review=Reviews.FindByPredicate([this](const FVFXShowcaseReview& Item){return Item.VFXTag==SelectedTag;});
    if(!Review)
    {
        FVFXShowcaseReview New;New.VFXTag=SelectedTag;New.AssetVersion=Row->AssetVersion;
        Reviews.Add(New);Review=&Reviews.Last();
    }
    // Editing prose never grants, clears or refreshes the version of gate evidence.
    Review->Notes=Notes;Review->FailReason=FailReason;Review->TimestampUtc=FDateTime::UtcNow().ToIso8601();
    UpdateReviewRows();LastMessage=TEXT("Notes updated; review status and evidence retained. Save Reviews to persist.");
    OnChanged.Broadcast();return true;
}

void AVFXShowcaseController::ScheduleSmokeAction(float Delay,TFunction<void()> Action)
{
    if(!GetWorld())return;
    FTimerHandle Timer;
    GetWorld()->GetTimerManager().SetTimer(Timer,FTimerDelegate::CreateWeakLambda(this,
        [Action=MoveTemp(Action)](){Action();}),Delay,false);
    SmokeTimers.Add(Timer);
}

void AVFXShowcaseController::CaptureSmokeStage(const FString& Stage,bool bScreenshot)
{
    const FString Directory=FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("ShowcaseSetup"));
    IFileManager::Get().MakeDirectory(*Directory,true);
    const FString Screenshot=FPaths::Combine(Directory,TEXT("Showcase_")+Stage+TEXT(".png"));
    UModularGameVFXBlueprintLibrary::GetVFXRuntimeCounts(this,Performance.ManagedVFXCount,Performance.ActiveVFXCount);
    Performance.CurrentTestCount=TestCount;Performance.Quality=Quality;
    TSharedRef<FJsonObject> Root=MakeShared<FJsonObject>();
    Root->SetStringField(TEXT("stage"),Stage);Root->SetStringField(TEXT("timestampUtc"),FDateTime::UtcNow().ToIso8601());
    Root->SetStringField(TEXT("tag"),SelectedTag.ToString());Root->SetStringField(TEXT("message"),LastMessage);
    Root->SetBoolField(TEXT("screenshotRequested"),bScreenshot);
    Root->SetStringField(TEXT("screenshotPath"),bScreenshot?Screenshot:TEXT(""));
    Root->SetStringField(TEXT("visualReview"),TEXT("NotTested"));Root->SetStringField(TEXT("gpuProfile"),TEXT("NotRun"));
    TSharedRef<FJsonObject> Telemetry=MakeShared<FJsonObject>();
    const FVFXShowcasePerformance Snapshot=GetPerformance();
    FJsonObjectConverter::UStructToJsonObject(FVFXShowcasePerformance::StaticStruct(),&Snapshot,Telemetry,0,0);
    Root->SetObjectField(TEXT("performance"),Telemetry);
    for(const TCHAR* Name:{TEXT("sg.EffectsQuality"),TEXT("fx.Niagara.QualityLevel")})
        if(IConsoleVariable* CVar=IConsoleManager::Get().FindConsoleVariable(Name))Root->SetNumberField(Name,CVar->GetInt());
    FString Json;FJsonSerializer::Serialize(Root,TJsonWriterFactory<>::Create(&Json));
    const FString ReportPath=FPaths::Combine(Directory,TEXT("Showcase_")+Stage+TEXT(".json"));
    if(!FFileHelper::SaveStringToFile(Json,*ReportPath,FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
        UE_LOG(LogTemp,Error,TEXT("SHOWCASE_SMOKE_REPORT_FAILED %s"),*ReportPath);
    if(bScreenshot)FScreenshotRequest::RequestScreenshot(Screenshot,true,false);
    UE_LOG(LogTemp,Display,TEXT("SHOWCASE_SMOKE %s managed=%d active=%d test=%d screenshot_requested=%d"),
        *Stage,Snapshot.ManagedVFXCount,Snapshot.ActiveVFXCount,Snapshot.CurrentTestCount,bScreenshot);
}

void AVFXShowcaseController::StartSmokeTest()
{
    // Explicit command-line opt-in only. Weak delegates are canceled on EndPlay;
    // no review setters or review persistence are invoked by this smoke sequence.
    ScheduleSmokeAction(5.f,[this]()
    {
        const FVFXShowcaseEntry* Impact=AllEntries.FindByPredicate([](const FVFXShowcaseEntry& Row)
            {return Row.Category==EVFXCategory::Impact&&Row.VFXTag.IsValid()&&Row.ValidationIssues.IsEmpty();});
        if(!Impact){LastMessage=TEXT("Smoke test failed: no valid Impact catalog entry.");CaptureSmokeStage(TEXT("MissingImpact"),false);return;}
        bSmokeImpactSelected=SelectEntry(Impact->VFXTag,false);SetQuality(EVFXShowcaseQuality::High);
        ScheduleSmokeAction(.15f,[this](){CaptureSmokeStage(TEXT("High"),true);});
    });
    ScheduleSmokeAction(10.f,[this]()
    {
        if(!bSmokeImpactSelected)return;
        SetQuality(EVFXShowcaseQuality::VRMobile);
        ScheduleSmokeAction(.15f,[this](){CaptureSmokeStage(TEXT("VRMobile"),true);});
    });
    ScheduleSmokeAction(15.f,[this]()
    {
        if(!bSmokeImpactSelected)return;
        SetQuality(EVFXShowcaseQuality::High);PlayStress(10);
        ScheduleSmokeAction(.15f,[this](){CaptureSmokeStage(TEXT("Stress10"),true);});
    });
    ScheduleSmokeAction(20.f,[this](){Stop(true);CaptureSmokeStage(TEXT("Stopped"),false);});
    ScheduleSmokeAction(22.f,[this](){CaptureSmokeStage(TEXT("Exit"),false);FPlatformMisc::RequestExit(false);});
}
void AVFXShowcaseController::ToggleFavorite()
{if(FVFXShowcaseReview* Review=FindOrAddReview(SelectedTag)){Review->bFavorite=!Review->bFavorite;UpdateReviewRows();OnChanged.Broadcast();}}
FString AVFXShowcaseController::GetReviewFilePath() const
{return FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("ModularGameVFXShowcase"),TEXT("Reviews.json"));}
bool AVFXShowcaseController::SaveReviews()
{
    TSharedRef<FJsonObject> Root=MakeShared<FJsonObject>();Root->SetNumberField(TEXT("schemaVersion"),1);Root->SetStringField(TEXT("savedUtc"),FDateTime::UtcNow().ToIso8601());
    TArray<TSharedPtr<FJsonValue>> Values;
    for(FVFXShowcaseReview& Review:Reviews)
    {
        const FVFXShowcaseEntry* Row=FindRow(Review.VFXTag);
        if(Row&&Review.AssetVersion!=Row->AssetVersion){Review.Status=EVFXReviewStatus::NotTested;Review.AssetVersion=Row->AssetVersion;}
        if(Review.Status==EVFXReviewStatus::ProductionReady&&!Review.IsProductionReady())Review.Status=EVFXReviewStatus::NotTested;
        TSharedRef<FJsonObject> Object=MakeShared<FJsonObject>();FJsonObjectConverter::UStructToJsonObject(FVFXShowcaseReview::StaticStruct(),&Review,Object,0,0);Values.Add(MakeShared<FJsonValueObject>(Object));
    }
    Root->SetArrayField(TEXT("reviews"),Values);
    FString Output;const TSharedRef<TJsonWriter<>> Writer=TJsonWriterFactory<>::Create(&Output);
    if(!FJsonSerializer::Serialize(Root,Writer))return false;
    const FString Path=GetReviewFilePath();IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path),true);
    const FString TempPath=Path+TEXT(".tmp");
    if(!FFileHelper::SaveStringToFile(Output,*TempPath,FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM)||!IFileManager::Get().Move(*Path,*TempPath,true,true))
    {LastMessage=TEXT("Could not persist Reviews.json; existing file retained where possible.");return false;}
    UpdateReviewRows();LastMessage=TEXT("Reviews saved: ")+Path;OnChanged.Broadcast();return true;
}
bool AVFXShowcaseController::LoadReviews()
{
    const FString Path=GetReviewFilePath();if(!IFileManager::Get().FileExists(*Path))return true;
    FString Input;TSharedPtr<FJsonObject> Root;
    if(!FFileHelper::LoadFileToString(Input,*Path)||!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Input),Root)||!Root.IsValid())
    {LastMessage=TEXT("Review JSON is invalid; in-memory records retained.");return false;}
    const TArray<TSharedPtr<FJsonValue>>* Values=nullptr;
    if(!Root->TryGetArrayField(TEXT("reviews"),Values))return false;
    TArray<FVFXShowcaseReview> Loaded;
    for(const TSharedPtr<FJsonValue>& Value:*Values)
    {
        FVFXShowcaseReview Item;
        if(!Value.IsValid()||Value->Type!=EJson::Object||!FJsonObjectConverter::JsonObjectToUStruct(Value->AsObject().ToSharedRef(),FVFXShowcaseReview::StaticStruct(),&Item,0,0))
        {LastMessage=TEXT("Invalid review record; in-memory records retained.");return false;}
        if(Item.Status==EVFXReviewStatus::ProductionReady&&!Item.IsProductionReady())Item.Status=EVFXReviewStatus::NotTested;
        if(Loaded.ContainsByPredicate([&Item](const FVFXShowcaseReview& R){return R.VFXTag==Item.VFXTag;}))return false;
        Loaded.Add(MoveTemp(Item));
    }
    Reviews=MoveTemp(Loaded);UpdateReviewRows();OnChanged.Broadcast();return true;
}
