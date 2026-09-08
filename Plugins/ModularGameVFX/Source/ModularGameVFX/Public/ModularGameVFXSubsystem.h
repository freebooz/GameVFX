#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Engine/StreamableManager.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "VFXTypes.h"
#include "ModularGameVFXSubsystem.generated.h"

class UModularGameVFXBlueprintLibrary;
class UNiagaraComponent;
class UNiagaraSystem;
class UVFXCatalog;

UCLASS()
class MODULARGAMEVFX_API UModularGameVFXSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

private:
    friend class UModularGameVFXBlueprintLibrary;

    struct FVFXRuntimeInstance
    {
        FVFXHandle Handle;
        FGameplayTag VFXTag;
        EVFXSpawnMode SpawnMode = EVFXSpawnMode::AtLocation;
        FTransform Transform = FTransform::Identity;
        TWeakObjectPtr<UWorld> World;
        TWeakObjectPtr<USceneComponent> AttachComponent;
        FName SocketName = NAME_None;
        TWeakObjectPtr<UObject> Owner;
        bool bTrackOwner = false;
        bool bUseCatalogDefaultTransform = true;
        bool bOverrideAutoDestroy = false;
        bool bAutoDestroy = true;
        bool bOverridePoolingMethod = false;
        ENCPoolMethod PoolingMethod = ENCPoolMethod::AutoRelease;
        FVFXParameterOverrides Parameters;
        FVFXCatalogEntry CatalogEntry;
        bool bEntryResolved = false;
        bool bSpawned = false;
        bool bStopRequested = false;
        TWeakObjectPtr<UNiagaraComponent> Component;
        FOnModularVFXReady Completion;
    };

    struct FPendingNiagaraLoad
    {
        TSharedPtr<FStreamableHandle> Handle;
        TArray<FGuid> WaitingHandles;
    };

    struct FPreloadedAsset
    {
        TSharedPtr<FStreamableHandle> Handle;
        int32 ReferenceCount = 0;
    };

    FVFXHandle PlayVFX(const UObject* WorldContextObject, const FVFXPlayRequest& Request, const FOnModularVFXReady& OnReady);
    bool StopVFX(FVFXHandle Handle);
    bool StopVFXImmediate(FVFXHandle Handle);
    int32 StopAllVFXForOwner(const UObject* Owner, bool bImmediate);
    int32 StopVFXByTag(FGameplayTag VFXTag, bool bImmediate);
    bool SetVFXFloatParameter(FVFXHandle Handle, FName ParameterName, float Value);
    bool SetVFXVectorParameter(FVFXHandle Handle, FName ParameterName, FVector Value);
    bool SetVFXColorParameter(FVFXHandle Handle, FName ParameterName, FLinearColor Value);
    bool SetVFXParameters(FVFXHandle Handle, const FVFXParameterOverrides& Parameters);
    bool PreloadVFX(const UObject* WorldContextObject, FGameplayTag VFXTag);
    int32 PreloadVFXTags(const UObject* WorldContextObject, const FGameplayTagContainer& VFXTags);
    bool ReleasePreloadedVFX(FGameplayTag VFXTag);
    bool IsVFXHandleValid(FVFXHandle Handle) const;
    bool FindVFXEntry(FGameplayTag VFXTag, FVFXCatalogEntry& OutEntry) const;
    void GetVFXRuntimeCounts(const UWorld* World, int32& Managed, int32& Active) const;

    void StartCatalogLoad();
    void HandleCatalogLoaded(TSharedPtr<FStreamableHandle> CompletedHandle);
    void ResolveHandle(const FGuid& HandleId);
    void BeginNiagaraLoad(const FSoftObjectPath& AssetPath, const FGuid& HandleId);
    void HandleNiagaraLoaded(TSharedPtr<FStreamableHandle> CompletedHandle, FSoftObjectPath AssetPath);
    UNiagaraComponent* SpawnResolvedVFX(FVFXRuntimeInstance& RuntimeInstance, UNiagaraSystem* NiagaraSystem);
    void ApplyParameters(UNiagaraComponent& Component, const FVFXParameterOverrides& Parameters) const;
    void CompleteRequest(const FGuid& HandleId, bool bSuccess);
    void RemoveRuntimeInstance(const FGuid& HandleId);
    void BeginPreloadForTag(FGameplayTag VFXTag);
    bool TickCleanup(float DeltaTime);
    static bool IsStandardUserParameter(FName ParameterName);

    UFUNCTION()
    void HandleSystemFinished(UNiagaraComponent* FinishedComponent);

    UPROPERTY(Transient)
    TObjectPtr<UVFXCatalog> LoadedCatalog = nullptr;

    FSoftObjectPath DefaultCatalogPath;
    TSharedPtr<FStreamableHandle> CatalogLoadHandle;
    TArray<FGuid> WaitingForCatalog;
    TMap<FGuid, FVFXRuntimeInstance> RuntimeInstances;
    TMultiMap<TWeakObjectPtr<UObject>, FGuid> HandlesByOwner;
    TMultiMap<FGameplayTag, FGuid> HandlesByTag;
    TMap<TWeakObjectPtr<UNiagaraComponent>, FGuid> HandlesByComponent;
    TMap<FSoftObjectPath, FPendingNiagaraLoad> PendingLoads;
    TMap<FGameplayTag, int32> PendingPreloadReferences;
    TMap<FGameplayTag, FSoftObjectPath> PreloadPathByTag;
    TMap<FSoftObjectPath, FPreloadedAsset> PreloadedAssets;
    FTSTicker::FDelegateHandle CleanupTickerHandle;
};
