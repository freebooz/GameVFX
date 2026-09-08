#include "ModularGameVFXSubsystem.h"

#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "ModularGameVFXSettings.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "VFXCatalog.h"

void UModularGameVFXSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    const UModularGameVFXSettings* Settings = GetDefault<UModularGameVFXSettings>();
    DefaultCatalogPath = Settings ? Settings->DefaultCatalog.ToSoftObjectPath() : FSoftObjectPath();
    CleanupTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateUObject(this, &UModularGameVFXSubsystem::TickCleanup), 0.25f);

    if (DefaultCatalogPath.IsNull())
    {
        UE_LOG(LogModularGameVFX, Warning, TEXT("[ModularGameVFX] Default Catalog is not configured in Project Settings."));
        return;
    }

    StartCatalogLoad();
}

void UModularGameVFXSubsystem::Deinitialize()
{
    if (CleanupTickerHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(CleanupTickerHandle);
        CleanupTickerHandle.Reset();
    }

    TArray<FGuid> ActiveHandles;
    RuntimeInstances.GetKeys(ActiveHandles);
    for (const FGuid& HandleId : ActiveHandles)
    {
        StopVFXImmediate(FVFXHandle(HandleId));
    }

    for (TPair<FSoftObjectPath, FPendingNiagaraLoad>& Pair : PendingLoads)
    {
        if (Pair.Value.Handle.IsValid())
        {
            Pair.Value.Handle->CancelHandle();
        }
    }

    for (TPair<FSoftObjectPath, FPreloadedAsset>& Pair : PreloadedAssets)
    {
        if (Pair.Value.Handle.IsValid())
        {
            Pair.Value.Handle->ReleaseHandle();
        }
    }

    if (CatalogLoadHandle.IsValid())
    {
        CatalogLoadHandle->CancelHandle();
    }

    PendingLoads.Reset();
    PreloadedAssets.Reset();
    PreloadPathByTag.Reset();
    PendingPreloadReferences.Reset();
    WaitingForCatalog.Reset();
    CatalogLoadHandle.Reset();
    LoadedCatalog = nullptr;
    Super::Deinitialize();
}

FVFXHandle UModularGameVFXSubsystem::PlayVFX(
    const UObject* WorldContextObject,
    const FVFXPlayRequest& Request,
    const FOnModularVFXReady& OnReady)
{
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
    if (!World)
    {
        UE_LOG(LogModularGameVFX, Warning, TEXT("[ModularGameVFX] PlayVFX rejected: World is invalid."));
        OnReady.ExecuteIfBound(FVFXHandle(), false);
        return FVFXHandle();
    }

    if (World->GetNetMode() == NM_DedicatedServer)
    {
        UE_LOG(LogModularGameVFX, Verbose, TEXT("[ModularGameVFX] PlayVFX ignored on a dedicated server: %s"), *Request.VFXTag.ToString());
        OnReady.ExecuteIfBound(FVFXHandle(), false);
        return FVFXHandle();
    }

    if (!Request.VFXTag.IsValid())
    {
        UE_LOG(LogModularGameVFX, Warning, TEXT("[ModularGameVFX] PlayVFX rejected: VFX Tag is invalid."));
        OnReady.ExecuteIfBound(FVFXHandle(), false);
        return FVFXHandle();
    }

    if (Request.SpawnMode == EVFXSpawnMode::Attached && !IsValid(Request.AttachComponent))
    {
        UE_LOG(LogModularGameVFX, Warning, TEXT("[ModularGameVFX] PlayVFX rejected for '%s': Attach Component is invalid."), *Request.VFXTag.ToString());
        OnReady.ExecuteIfBound(FVFXHandle(), false);
        return FVFXHandle();
    }

    const FVFXHandle Handle(FGuid::NewGuid());
    FVFXRuntimeInstance& Runtime = RuntimeInstances.Add(Handle.Id);
    Runtime.Handle = Handle;
    Runtime.VFXTag = Request.VFXTag;
    Runtime.SpawnMode = Request.SpawnMode;
    Runtime.Transform = Request.Transform;
    Runtime.World = World;
    Runtime.AttachComponent = Request.AttachComponent;
    Runtime.SocketName = Request.SocketName;
    Runtime.bUseCatalogDefaultTransform = Request.bUseCatalogDefaultTransform;
    Runtime.bOverrideAutoDestroy = Request.bOverrideAutoDestroy;
    Runtime.bAutoDestroy = Request.bAutoDestroy;
    Runtime.bOverridePoolingMethod = Request.bOverridePoolingMethod;
    Runtime.PoolingMethod = Request.PoolingMethod;
    Runtime.Parameters = Request.ParameterOverrides;
    Runtime.Completion = OnReady;

    UObject* EffectiveOwner = Request.Owner;
    if (!EffectiveOwner && Request.AttachComponent)
    {
        EffectiveOwner = Request.AttachComponent->GetOwner();
    }
    Runtime.Owner = EffectiveOwner;
    Runtime.bTrackOwner = EffectiveOwner != nullptr;

    HandlesByTag.Add(Runtime.VFXTag, Handle.Id);
    if (Runtime.bTrackOwner)
    {
        HandlesByOwner.Add(Runtime.Owner, Handle.Id);
    }

    if (LoadedCatalog)
    {
        ResolveHandle(Handle.Id);
    }
    else if (DefaultCatalogPath.IsNull())
    {
        UE_LOG(LogModularGameVFX, Warning, TEXT("[ModularGameVFX] Cannot play '%s': Default Catalog is not configured."), *Request.VFXTag.ToString());
        CompleteRequest(Handle.Id, false);
    }
    else
    {
        WaitingForCatalog.AddUnique(Handle.Id);
        StartCatalogLoad();
    }

    return Handle;
}

void UModularGameVFXSubsystem::StartCatalogLoad()
{
    if (LoadedCatalog || CatalogLoadHandle.IsValid() || DefaultCatalogPath.IsNull())
    {
        return;
    }

    if (UVFXCatalog* ExistingCatalog = Cast<UVFXCatalog>(DefaultCatalogPath.ResolveObject()))
    {
        LoadedCatalog = ExistingCatalog;
        LoadedCatalog->BuildRuntimeCache();
        HandleCatalogLoaded(nullptr);
        return;
    }

    CatalogLoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
        DefaultCatalogPath,
        FStreamableDelegateWithHandle::CreateUObject(this, &UModularGameVFXSubsystem::HandleCatalogLoaded),
        FStreamableManager::DefaultAsyncLoadPriority,
        false,
        false,
        TEXT("ModularGameVFX_DefaultCatalog"));
}

void UModularGameVFXSubsystem::HandleCatalogLoaded(TSharedPtr<FStreamableHandle> CompletedHandle)
{
    if (!LoadedCatalog)
    {
        LoadedCatalog = Cast<UVFXCatalog>(DefaultCatalogPath.ResolveObject());
    }
    CatalogLoadHandle.Reset();

    if (!LoadedCatalog)
    {
        UE_LOG(LogModularGameVFX, Error, TEXT("[ModularGameVFX] Failed to load Default Catalog: %s"), *DefaultCatalogPath.ToString());
        const TArray<FGuid> FailedHandles = WaitingForCatalog;
        WaitingForCatalog.Reset();
        for (const FGuid& HandleId : FailedHandles)
        {
            CompleteRequest(HandleId, false);
        }
        PendingPreloadReferences.Reset();
        return;
    }

    LoadedCatalog->BuildRuntimeCache();
    const TArray<FGuid> ReadyHandles = WaitingForCatalog;
    WaitingForCatalog.Reset();
    for (const FGuid& HandleId : ReadyHandles)
    {
        ResolveHandle(HandleId);
    }

    TArray<FGameplayTag> PreloadTags;
    PendingPreloadReferences.GetKeys(PreloadTags);
    for (const FGameplayTag& Tag : PreloadTags)
    {
        BeginPreloadForTag(Tag);
    }
}

void UModularGameVFXSubsystem::ResolveHandle(const FGuid& HandleId)
{
    FVFXRuntimeInstance* Runtime = RuntimeInstances.Find(HandleId);
    if (!Runtime || Runtime->bStopRequested)
    {
        return;
    }

    if (!LoadedCatalog)
    {
        WaitingForCatalog.AddUnique(HandleId);
        StartCatalogLoad();
        return;
    }

    const FVFXCatalogEntry* Entry = LoadedCatalog->FindEntryNative(Runtime->VFXTag);
    if (!Entry)
    {
        UE_LOG(LogModularGameVFX, Warning, TEXT("[ModularGameVFX] VFX Tag not found: %s"), *Runtime->VFXTag.ToString());
        CompleteRequest(HandleId, false);
        return;
    }

    if (Entry->NiagaraSystem.IsNull())
    {
        UE_LOG(LogModularGameVFX, Warning, TEXT("[ModularGameVFX] Niagara reference is empty for VFX Tag: %s"), *Runtime->VFXTag.ToString());
        CompleteRequest(HandleId, false);
        return;
    }

    Runtime->CatalogEntry = *Entry;
    Runtime->bEntryResolved = true;
    const FSoftObjectPath AssetPath = Entry->NiagaraSystem.ToSoftObjectPath();
    if (UNiagaraSystem* NiagaraSystem = Entry->NiagaraSystem.Get())
    {
        if (!SpawnResolvedVFX(*Runtime, NiagaraSystem))
        {
            CompleteRequest(HandleId, false);
        }
        return;
    }

    BeginNiagaraLoad(AssetPath, HandleId);
}

void UModularGameVFXSubsystem::BeginNiagaraLoad(const FSoftObjectPath& AssetPath, const FGuid& HandleId)
{
    FPendingNiagaraLoad& PendingLoad = PendingLoads.FindOrAdd(AssetPath);
    PendingLoad.WaitingHandles.AddUnique(HandleId);
    if (PendingLoad.Handle.IsValid())
    {
        return;
    }

    PendingLoad.Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
        AssetPath,
        FStreamableDelegateWithHandle::CreateUObject(this, &UModularGameVFXSubsystem::HandleNiagaraLoaded, AssetPath),
        FStreamableManager::DefaultAsyncLoadPriority,
        false,
        false,
        FString::Printf(TEXT("ModularGameVFX_%s"), *AssetPath.GetAssetName()));
}

void UModularGameVFXSubsystem::HandleNiagaraLoaded(TSharedPtr<FStreamableHandle> CompletedHandle, FSoftObjectPath AssetPath)
{
    FPendingNiagaraLoad PendingLoad;
    if (!PendingLoads.RemoveAndCopyValue(AssetPath, PendingLoad))
    {
        return;
    }

    UNiagaraSystem* NiagaraSystem = Cast<UNiagaraSystem>(AssetPath.ResolveObject());
    if (!NiagaraSystem)
    {
        UE_LOG(LogModularGameVFX, Error, TEXT("[ModularGameVFX] Failed to load Niagara System: %s"), *AssetPath.ToString());
    }

    for (const FGuid& HandleId : PendingLoad.WaitingHandles)
    {
        FVFXRuntimeInstance* Runtime = RuntimeInstances.Find(HandleId);
        if (!Runtime || Runtime->bStopRequested)
        {
            continue;
        }

        if (!NiagaraSystem || !SpawnResolvedVFX(*Runtime, NiagaraSystem))
        {
            CompleteRequest(HandleId, false);
        }
    }
}

UNiagaraComponent* UModularGameVFXSubsystem::SpawnResolvedVFX(FVFXRuntimeInstance& Runtime, UNiagaraSystem* NiagaraSystem)
{
    UWorld* World = Runtime.World.Get();
    if (!World || World->GetNetMode() == NM_DedicatedServer || !NiagaraSystem || Runtime.bStopRequested)
    {
        return nullptr;
    }

    FTransform SpawnTransform = Runtime.Transform;
    if (Runtime.bUseCatalogDefaultTransform)
    {
        const FQuat BaseRotation = Runtime.Transform.GetRotation();
        const FQuat Rotation = BaseRotation * Runtime.CatalogEntry.RotationOffset.Quaternion();
        const FVector Location = Runtime.Transform.GetLocation() + BaseRotation.RotateVector(Runtime.CatalogEntry.LocationOffset);
        const FVector Scale = Runtime.Transform.GetScale3D() * Runtime.CatalogEntry.DefaultScale;
        SpawnTransform = FTransform(Rotation, Location, Scale);
    }
    const bool bAutoDestroy = Runtime.bOverrideAutoDestroy ? Runtime.bAutoDestroy : Runtime.CatalogEntry.bAutoDestroy;
    const ENCPoolMethod PoolingMethod = Runtime.bOverridePoolingMethod ? Runtime.PoolingMethod : Runtime.CatalogEntry.PoolingMethod;
    UNiagaraComponent* Component = nullptr;

    if (Runtime.SpawnMode == EVFXSpawnMode::Attached)
    {
        USceneComponent* AttachComponent = Runtime.AttachComponent.Get();
        if (!IsValid(AttachComponent))
        {
            UE_LOG(LogModularGameVFX, Warning, TEXT("[ModularGameVFX] Attach Component expired before '%s' could spawn."), *Runtime.VFXTag.ToString());
            return nullptr;
        }

        const FName SocketName = Runtime.SocketName.IsNone() ? Runtime.CatalogEntry.DefaultSocketName : Runtime.SocketName;
        Component = UNiagaraFunctionLibrary::SpawnSystemAttached(
            NiagaraSystem,
            AttachComponent,
            SocketName,
            SpawnTransform.GetLocation(),
            SpawnTransform.Rotator(),
            SpawnTransform.GetScale3D(),
            EAttachLocation::KeepRelativeOffset,
            bAutoDestroy,
            PoolingMethod,
            false,
            true);
    }
    else
    {
        Component = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            World,
            NiagaraSystem,
            SpawnTransform.GetLocation(),
            SpawnTransform.Rotator(),
            SpawnTransform.GetScale3D(),
            bAutoDestroy,
            false,
            PoolingMethod,
            true);
    }

    if (!Component)
    {
        UE_LOG(LogModularGameVFX, Warning, TEXT("[ModularGameVFX] Niagara spawn failed for VFX Tag: %s"), *Runtime.VFXTag.ToString());
        return nullptr;
    }

    FVFXParameterOverrides EffectiveParameters = Runtime.CatalogEntry.DefaultParameters;
    EffectiveParameters.Append(Runtime.Parameters);
    ApplyParameters(*Component, EffectiveParameters);

    Runtime.Component = Component;
    Runtime.PoolingMethod = PoolingMethod;
    Runtime.bSpawned = true;
    HandlesByComponent.Add(Component, Runtime.Handle.Id);
    Component->OnSystemFinished.AddDynamic(this, &UModularGameVFXSubsystem::HandleSystemFinished);
    Component->Activate(true);
    CompleteRequest(Runtime.Handle.Id, true);
    return Component;
}

void UModularGameVFXSubsystem::ApplyParameters(UNiagaraComponent& Component, const FVFXParameterOverrides& Parameters) const
{
    for (const TPair<FName, float>& Pair : Parameters.FloatParameters)
    {
        Component.SetVariableFloat(Pair.Key, Pair.Value);
    }
    for (const TPair<FName, FVector>& Pair : Parameters.VectorParameters)
    {
        Component.SetVariableVec3(Pair.Key, Pair.Value);
    }
    for (const TPair<FName, FLinearColor>& Pair : Parameters.ColorParameters)
    {
        Component.SetVariableLinearColor(Pair.Key, Pair.Value);
    }
}

void UModularGameVFXSubsystem::CompleteRequest(const FGuid& HandleId, bool bSuccess)
{
    FVFXRuntimeInstance* Runtime = RuntimeInstances.Find(HandleId);
    if (!Runtime)
    {
        return;
    }

    const FOnModularVFXReady Completion = Runtime->Completion;
    Runtime->Completion.Unbind();
    Completion.ExecuteIfBound(Runtime->Handle, bSuccess);
    if (!bSuccess)
    {
        RemoveRuntimeInstance(HandleId);
    }
}

void UModularGameVFXSubsystem::GetVFXRuntimeCounts(const UWorld* World, int32& Managed, int32& Active) const
{
    Managed = Active = 0;
    if (!World) return;
    for (const auto& Pair : RuntimeInstances)
    {
        const FVFXRuntimeInstance& Instance = Pair.Value;
        if (Instance.World.Get() != World) continue;
        ++Managed;
        const UNiagaraComponent* Component = Instance.Component.Get();
        if (Instance.bSpawned && Component && Component->IsActive()) ++Active;
    }
}

bool UModularGameVFXSubsystem::StopVFX(FVFXHandle Handle)
{
    FVFXRuntimeInstance* Runtime = RuntimeInstances.Find(Handle.Id);
    if (!Runtime)
    {
        return false;
    }

    Runtime->bStopRequested = true;
    if (UNiagaraComponent* Component = Runtime->Component.Get())
    {
        Component->Deactivate();
    }
    else
    {
        CompleteRequest(Handle.Id, false);
    }
    return true;
}

bool UModularGameVFXSubsystem::StopVFXImmediate(FVFXHandle Handle)
{
    FVFXRuntimeInstance* Runtime = RuntimeInstances.Find(Handle.Id);
    if (!Runtime)
    {
        return false;
    }

    Runtime->bStopRequested = true;
    if (UNiagaraComponent* Component = Runtime->Component.Get())
    {
        Component->OnSystemFinished.RemoveDynamic(this, &UModularGameVFXSubsystem::HandleSystemFinished);
        Component->DeactivateImmediate();
        if (Runtime->PoolingMethod == ENCPoolMethod::ManualRelease || Runtime->PoolingMethod == ENCPoolMethod::ManualRelease_OnComplete)
        {
            Component->ReleaseToPool();
        }
    }
    else
    {
        const FOnModularVFXReady Completion = Runtime->Completion;
        Runtime->Completion.Unbind();
        Completion.ExecuteIfBound(Runtime->Handle, false);
    }

    RemoveRuntimeInstance(Handle.Id);
    return true;
}

int32 UModularGameVFXSubsystem::StopAllVFXForOwner(const UObject* Owner, bool bImmediate)
{
    if (!Owner)
    {
        return 0;
    }

    TArray<FGuid> Handles;
    HandlesByOwner.MultiFind(TWeakObjectPtr<UObject>(const_cast<UObject*>(Owner)), Handles);
    int32 StoppedCount = 0;
    for (const FGuid& HandleId : Handles)
    {
        StoppedCount += bImmediate ? (StopVFXImmediate(FVFXHandle(HandleId)) ? 1 : 0) : (StopVFX(FVFXHandle(HandleId)) ? 1 : 0);
    }
    return StoppedCount;
}

int32 UModularGameVFXSubsystem::StopVFXByTag(FGameplayTag VFXTag, bool bImmediate)
{
    TArray<FGuid> Handles;
    HandlesByTag.MultiFind(VFXTag, Handles);
    int32 StoppedCount = 0;
    for (const FGuid& HandleId : Handles)
    {
        StoppedCount += bImmediate ? (StopVFXImmediate(FVFXHandle(HandleId)) ? 1 : 0) : (StopVFX(FVFXHandle(HandleId)) ? 1 : 0);
    }
    return StoppedCount;
}

bool UModularGameVFXSubsystem::IsStandardUserParameter(FName ParameterName)
{
    return ParameterName.ToString().StartsWith(TEXT("User."));
}

bool UModularGameVFXSubsystem::SetVFXFloatParameter(FVFXHandle Handle, FName ParameterName, float Value)
{
    FVFXRuntimeInstance* Runtime = RuntimeInstances.Find(Handle.Id);
    if (!Runtime || ParameterName.IsNone())
    {
        return false;
    }
    if (!IsStandardUserParameter(ParameterName))
    {
        UE_LOG(LogModularGameVFX, Warning, TEXT("[ModularGameVFX] Non-standard Niagara parameter '%s'; use the User.* namespace."), *ParameterName.ToString());
    }
    Runtime->Parameters.FloatParameters.Add(ParameterName, Value);
    if (UNiagaraComponent* Component = Runtime->Component.Get())
    {
        Component->SetVariableFloat(ParameterName, Value);
    }
    return true;
}

bool UModularGameVFXSubsystem::SetVFXVectorParameter(FVFXHandle Handle, FName ParameterName, FVector Value)
{
    FVFXRuntimeInstance* Runtime = RuntimeInstances.Find(Handle.Id);
    if (!Runtime || ParameterName.IsNone())
    {
        return false;
    }
    if (!IsStandardUserParameter(ParameterName))
    {
        UE_LOG(LogModularGameVFX, Warning, TEXT("[ModularGameVFX] Non-standard Niagara parameter '%s'; use the User.* namespace."), *ParameterName.ToString());
    }
    Runtime->Parameters.VectorParameters.Add(ParameterName, Value);
    if (UNiagaraComponent* Component = Runtime->Component.Get())
    {
        Component->SetVariableVec3(ParameterName, Value);
    }
    return true;
}

bool UModularGameVFXSubsystem::SetVFXColorParameter(FVFXHandle Handle, FName ParameterName, FLinearColor Value)
{
    FVFXRuntimeInstance* Runtime = RuntimeInstances.Find(Handle.Id);
    if (!Runtime || ParameterName.IsNone())
    {
        return false;
    }
    if (!IsStandardUserParameter(ParameterName))
    {
        UE_LOG(LogModularGameVFX, Warning, TEXT("[ModularGameVFX] Non-standard Niagara parameter '%s'; use the User.* namespace."), *ParameterName.ToString());
    }
    Runtime->Parameters.ColorParameters.Add(ParameterName, Value);
    if (UNiagaraComponent* Component = Runtime->Component.Get())
    {
        Component->SetVariableLinearColor(ParameterName, Value);
    }
    return true;
}

bool UModularGameVFXSubsystem::SetVFXParameters(FVFXHandle Handle, const FVFXParameterOverrides& Parameters)
{
    FVFXRuntimeInstance* Runtime = RuntimeInstances.Find(Handle.Id);
    if (!Runtime)
    {
        return false;
    }
    Runtime->Parameters.Append(Parameters);
    if (UNiagaraComponent* Component = Runtime->Component.Get())
    {
        ApplyParameters(*Component, Parameters);
    }
    return true;
}

bool UModularGameVFXSubsystem::PreloadVFX(const UObject* WorldContextObject, FGameplayTag VFXTag)
{
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    if (!World || World->GetNetMode() == NM_DedicatedServer || !VFXTag.IsValid())
    {
        return false;
    }

    PendingPreloadReferences.FindOrAdd(VFXTag)++;
    if (LoadedCatalog)
    {
        if (const FSoftObjectPath* ExistingPath = PreloadPathByTag.Find(VFXTag))
        {
            if (FPreloadedAsset* Asset = PreloadedAssets.Find(*ExistingPath))
            {
                ++Asset->ReferenceCount;
            }
        }
        else
        {
            BeginPreloadForTag(VFXTag);
        }
    }
    else
    {
        StartCatalogLoad();
    }
    return true;
}

int32 UModularGameVFXSubsystem::PreloadVFXTags(const UObject* WorldContextObject, const FGameplayTagContainer& VFXTags)
{
    int32 AcceptedCount = 0;
    for (const FGameplayTag& Tag : VFXTags)
    {
        AcceptedCount += PreloadVFX(WorldContextObject, Tag) ? 1 : 0;
    }
    return AcceptedCount;
}

void UModularGameVFXSubsystem::BeginPreloadForTag(FGameplayTag VFXTag)
{
    if (!LoadedCatalog)
    {
        return;
    }

    const int32 RequestedReferences = PendingPreloadReferences.FindRef(VFXTag);
    if (RequestedReferences <= 0 || PreloadPathByTag.Contains(VFXTag))
    {
        return;
    }

    const FVFXCatalogEntry* Entry = LoadedCatalog->FindEntryNative(VFXTag);
    if (!Entry || Entry->NiagaraSystem.IsNull())
    {
        UE_LOG(LogModularGameVFX, Warning, TEXT("[ModularGameVFX] Cannot preload missing VFX Tag: %s"), *VFXTag.ToString());
        PendingPreloadReferences.Remove(VFXTag);
        return;
    }

    const FSoftObjectPath AssetPath = Entry->NiagaraSystem.ToSoftObjectPath();
    PreloadPathByTag.Add(VFXTag, AssetPath);
    FPreloadedAsset& Asset = PreloadedAssets.FindOrAdd(AssetPath);
    Asset.ReferenceCount += RequestedReferences;
    if (!Asset.Handle.IsValid())
    {
        Asset.Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
            AssetPath,
            FStreamableDelegateWithHandle(),
            FStreamableManager::DefaultAsyncLoadPriority,
            true,
            false,
            FString::Printf(TEXT("ModularGameVFX_Preload_%s"), *AssetPath.GetAssetName()));
    }
}

bool UModularGameVFXSubsystem::ReleasePreloadedVFX(FGameplayTag VFXTag)
{
    int32* TagReferences = PendingPreloadReferences.Find(VFXTag);
    if (!TagReferences || *TagReferences <= 0)
    {
        return false;
    }

    --(*TagReferences);
    const FSoftObjectPath* AssetPath = PreloadPathByTag.Find(VFXTag);
    if (AssetPath)
    {
        if (FPreloadedAsset* Asset = PreloadedAssets.Find(*AssetPath))
        {
            Asset->ReferenceCount = FMath::Max(0, Asset->ReferenceCount - 1);
            if (Asset->ReferenceCount == 0)
            {
                if (Asset->Handle.IsValid())
                {
                    Asset->Handle->ReleaseHandle();
                }
                PreloadedAssets.Remove(*AssetPath);
            }
        }
    }

    if (*TagReferences == 0)
    {
        PendingPreloadReferences.Remove(VFXTag);
        PreloadPathByTag.Remove(VFXTag);
    }
    return true;
}

bool UModularGameVFXSubsystem::IsVFXHandleValid(FVFXHandle Handle) const
{
    return Handle.IsValid() && RuntimeInstances.Contains(Handle.Id);
}

bool UModularGameVFXSubsystem::FindVFXEntry(FGameplayTag VFXTag, FVFXCatalogEntry& OutEntry) const
{
    return LoadedCatalog && LoadedCatalog->FindEntry(VFXTag, OutEntry);
}

void UModularGameVFXSubsystem::HandleSystemFinished(UNiagaraComponent* FinishedComponent)
{
    if (!FinishedComponent)
    {
        return;
    }

    FGuid HandleId;
    if (!HandlesByComponent.RemoveAndCopyValue(FinishedComponent, HandleId))
    {
        return;
    }

    if (FVFXRuntimeInstance* Runtime = RuntimeInstances.Find(HandleId))
    {
        FinishedComponent->OnSystemFinished.RemoveDynamic(this, &UModularGameVFXSubsystem::HandleSystemFinished);
        if (Runtime->PoolingMethod == ENCPoolMethod::ManualRelease || Runtime->PoolingMethod == ENCPoolMethod::ManualRelease_OnComplete)
        {
            FinishedComponent->ReleaseToPool();
        }
    }
    RemoveRuntimeInstance(HandleId);
}

void UModularGameVFXSubsystem::RemoveRuntimeInstance(const FGuid& HandleId)
{
    FVFXRuntimeInstance Runtime;
    if (!RuntimeInstances.RemoveAndCopyValue(HandleId, Runtime))
    {
        return;
    }

    WaitingForCatalog.Remove(HandleId);
    HandlesByTag.RemoveSingle(Runtime.VFXTag, HandleId);
    if (Runtime.bTrackOwner)
    {
        HandlesByOwner.RemoveSingle(Runtime.Owner, HandleId);
    }
    if (UNiagaraComponent* Component = Runtime.Component.Get())
    {
        HandlesByComponent.Remove(Component);
    }
}

bool UModularGameVFXSubsystem::TickCleanup(float DeltaTime)
{
    TArray<FGuid> HandlesToStop;
    for (const TPair<FGuid, FVFXRuntimeInstance>& Pair : RuntimeInstances)
    {
        const FVFXRuntimeInstance& Runtime = Pair.Value;
        if ((Runtime.bTrackOwner && !Runtime.Owner.IsValid()) || !Runtime.World.IsValid() || (Runtime.bSpawned && !Runtime.Component.IsValid()))
        {
            HandlesToStop.Add(Pair.Key);
        }
    }

    for (const FGuid& HandleId : HandlesToStop)
    {
        StopVFXImmediate(FVFXHandle(HandleId));
    }
    return true;
}
