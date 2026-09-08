#include "ModularGameVFXBlueprintLibrary.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "ModularGameVFXSubsystem.h"

UModularGameVFXSubsystem* UModularGameVFXBlueprintLibrary::GetSubsystem(const UObject* WorldContextObject)
{
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
    UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
    return GameInstance ? GameInstance->GetSubsystem<UModularGameVFXSubsystem>() : nullptr;
}

FVFXHandle UModularGameVFXBlueprintLibrary::PlayVFX(const UObject* WorldContextObject, const FVFXPlayRequest& Request, const FOnModularVFXReady& OnReady)
{
    if (UModularGameVFXSubsystem* Subsystem = GetSubsystem(WorldContextObject))
    {
        return Subsystem->PlayVFX(WorldContextObject, Request, OnReady);
    }
    OnReady.ExecuteIfBound(FVFXHandle(), false);
    return FVFXHandle();
}

FVFXHandle UModularGameVFXBlueprintLibrary::PlayVFXAtLocation(const UObject* WorldContextObject, FGameplayTag VFXTag, FTransform Transform, UObject* Owner, bool bUseCatalogDefaultTransform, const FVFXParameterOverrides& Parameters, const FOnModularVFXReady& OnReady)
{
    FVFXPlayRequest Request;
    Request.VFXTag = VFXTag;
    Request.SpawnMode = EVFXSpawnMode::AtLocation;
    Request.Transform = Transform;
    Request.Owner = Owner;
    Request.bUseCatalogDefaultTransform = bUseCatalogDefaultTransform;
    Request.ParameterOverrides = Parameters;
    return PlayVFX(WorldContextObject, Request, OnReady);
}

FVFXHandle UModularGameVFXBlueprintLibrary::PlayVFXAttached(const UObject* WorldContextObject, FGameplayTag VFXTag, USceneComponent* AttachComponent, FName SocketName, FTransform RelativeTransform, UObject* Owner, bool bUseCatalogDefaultTransform, const FVFXParameterOverrides& Parameters, const FOnModularVFXReady& OnReady)
{
    FVFXPlayRequest Request;
    Request.VFXTag = VFXTag;
    Request.SpawnMode = EVFXSpawnMode::Attached;
    Request.Transform = RelativeTransform;
    Request.AttachComponent = AttachComponent;
    Request.SocketName = SocketName;
    Request.Owner = Owner;
    Request.bUseCatalogDefaultTransform = bUseCatalogDefaultTransform;
    Request.ParameterOverrides = Parameters;
    return PlayVFX(WorldContextObject, Request, OnReady);
}

bool UModularGameVFXBlueprintLibrary::StopVFX(const UObject* WorldContextObject, FVFXHandle Handle)
{
    UModularGameVFXSubsystem* Subsystem = GetSubsystem(WorldContextObject);
    return Subsystem && Subsystem->StopVFX(Handle);
}

bool UModularGameVFXBlueprintLibrary::StopVFXImmediate(const UObject* WorldContextObject, FVFXHandle Handle)
{
    UModularGameVFXSubsystem* Subsystem = GetSubsystem(WorldContextObject);
    return Subsystem && Subsystem->StopVFXImmediate(Handle);
}

int32 UModularGameVFXBlueprintLibrary::StopAllVFXForOwner(const UObject* WorldContextObject, UObject* Owner, bool bImmediate)
{
    UModularGameVFXSubsystem* Subsystem = GetSubsystem(WorldContextObject);
    return Subsystem ? Subsystem->StopAllVFXForOwner(Owner, bImmediate) : 0;
}

int32 UModularGameVFXBlueprintLibrary::StopVFXByTag(const UObject* WorldContextObject, FGameplayTag VFXTag, bool bImmediate)
{
    UModularGameVFXSubsystem* Subsystem = GetSubsystem(WorldContextObject);
    return Subsystem ? Subsystem->StopVFXByTag(VFXTag, bImmediate) : 0;
}

bool UModularGameVFXBlueprintLibrary::SetVFXFloatParameter(const UObject* WorldContextObject, FVFXHandle Handle, FName ParameterName, float Value)
{
    UModularGameVFXSubsystem* Subsystem = GetSubsystem(WorldContextObject);
    return Subsystem && Subsystem->SetVFXFloatParameter(Handle, ParameterName, Value);
}

bool UModularGameVFXBlueprintLibrary::SetVFXVectorParameter(const UObject* WorldContextObject, FVFXHandle Handle, FName ParameterName, FVector Value)
{
    UModularGameVFXSubsystem* Subsystem = GetSubsystem(WorldContextObject);
    return Subsystem && Subsystem->SetVFXVectorParameter(Handle, ParameterName, Value);
}

bool UModularGameVFXBlueprintLibrary::SetVFXColorParameter(const UObject* WorldContextObject, FVFXHandle Handle, FName ParameterName, FLinearColor Value)
{
    UModularGameVFXSubsystem* Subsystem = GetSubsystem(WorldContextObject);
    return Subsystem && Subsystem->SetVFXColorParameter(Handle, ParameterName, Value);
}

bool UModularGameVFXBlueprintLibrary::SetVFXParameters(const UObject* WorldContextObject, FVFXHandle Handle, const FVFXParameterOverrides& Parameters)
{
    UModularGameVFXSubsystem* Subsystem = GetSubsystem(WorldContextObject);
    return Subsystem && Subsystem->SetVFXParameters(Handle, Parameters);
}

bool UModularGameVFXBlueprintLibrary::PreloadVFX(const UObject* WorldContextObject, FGameplayTag VFXTag)
{
    UModularGameVFXSubsystem* Subsystem = GetSubsystem(WorldContextObject);
    return Subsystem && Subsystem->PreloadVFX(WorldContextObject, VFXTag);
}

int32 UModularGameVFXBlueprintLibrary::PreloadVFXTags(const UObject* WorldContextObject, const FGameplayTagContainer& VFXTags)
{
    UModularGameVFXSubsystem* Subsystem = GetSubsystem(WorldContextObject);
    return Subsystem ? Subsystem->PreloadVFXTags(WorldContextObject, VFXTags) : 0;
}

bool UModularGameVFXBlueprintLibrary::ReleasePreloadedVFX(const UObject* WorldContextObject, FGameplayTag VFXTag)
{
    UModularGameVFXSubsystem* Subsystem = GetSubsystem(WorldContextObject);
    return Subsystem && Subsystem->ReleasePreloadedVFX(VFXTag);
}

bool UModularGameVFXBlueprintLibrary::IsVFXHandleValid(const UObject* WorldContextObject, FVFXHandle Handle)
{
    const UModularGameVFXSubsystem* Subsystem = GetSubsystem(WorldContextObject);
    return Subsystem && Subsystem->IsVFXHandleValid(Handle);
}

bool UModularGameVFXBlueprintLibrary::FindVFXEntry(const UObject* WorldContextObject, FGameplayTag VFXTag, FVFXCatalogEntry& OutEntry)
{
    const UModularGameVFXSubsystem* Subsystem = GetSubsystem(WorldContextObject);
    return Subsystem && Subsystem->FindVFXEntry(VFXTag, OutEntry);
}

void UModularGameVFXBlueprintLibrary::GetVFXRuntimeCounts(const UObject* WorldContextObject, int32& Managed, int32& Active)
{
    Managed = Active = 0;
    const UModularGameVFXSubsystem* Subsystem = GetSubsystem(WorldContextObject);
    if (Subsystem && WorldContextObject)
    {
        Subsystem->GetVFXRuntimeCounts(WorldContextObject->GetWorld(), Managed, Active);
    }
}
