#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VFXTypes.h"
#include "ModularGameVFXBlueprintLibrary.generated.h"

class UModularGameVFXSubsystem;

UCLASS()
class MODULARGAMEVFX_API UModularGameVFXBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Modular Game VFX", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "OnReady"))
    static FVFXHandle PlayVFX(const UObject* WorldContextObject, const FVFXPlayRequest& Request, const FOnModularVFXReady& OnReady);

    UFUNCTION(BlueprintCallable, Category = "Modular Game VFX", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "Parameters,OnReady"))
    static FVFXHandle PlayVFXAtLocation(const UObject* WorldContextObject, FGameplayTag VFXTag, FTransform Transform, UObject* Owner, bool bUseCatalogDefaultTransform, const FVFXParameterOverrides& Parameters, const FOnModularVFXReady& OnReady);

    UFUNCTION(BlueprintCallable, Category = "Modular Game VFX", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "Parameters,OnReady"))
    static FVFXHandle PlayVFXAttached(const UObject* WorldContextObject, FGameplayTag VFXTag, USceneComponent* AttachComponent, FName SocketName, FTransform RelativeTransform, UObject* Owner, bool bUseCatalogDefaultTransform, const FVFXParameterOverrides& Parameters, const FOnModularVFXReady& OnReady);

    UFUNCTION(BlueprintCallable, Category = "Modular Game VFX", meta = (WorldContext = "WorldContextObject"))
    static bool StopVFX(const UObject* WorldContextObject, FVFXHandle Handle);

    UFUNCTION(BlueprintCallable, Category = "Modular Game VFX", meta = (WorldContext = "WorldContextObject"))
    static bool StopVFXImmediate(const UObject* WorldContextObject, FVFXHandle Handle);

    UFUNCTION(BlueprintCallable, Category = "Modular Game VFX", meta = (WorldContext = "WorldContextObject"))
    static int32 StopAllVFXForOwner(const UObject* WorldContextObject, UObject* Owner, bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Modular Game VFX", meta = (WorldContext = "WorldContextObject"))
    static int32 StopVFXByTag(const UObject* WorldContextObject, FGameplayTag VFXTag, bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Modular Game VFX", meta = (WorldContext = "WorldContextObject"))
    static bool SetVFXFloatParameter(const UObject* WorldContextObject, FVFXHandle Handle, FName ParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category = "Modular Game VFX", meta = (WorldContext = "WorldContextObject"))
    static bool SetVFXVectorParameter(const UObject* WorldContextObject, FVFXHandle Handle, FName ParameterName, FVector Value);

    UFUNCTION(BlueprintCallable, Category = "Modular Game VFX", meta = (WorldContext = "WorldContextObject"))
    static bool SetVFXColorParameter(const UObject* WorldContextObject, FVFXHandle Handle, FName ParameterName, FLinearColor Value);

    UFUNCTION(BlueprintCallable, Category = "Modular Game VFX", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "Parameters"))
    static bool SetVFXParameters(const UObject* WorldContextObject, FVFXHandle Handle, const FVFXParameterOverrides& Parameters);

    UFUNCTION(BlueprintCallable, Category = "Modular Game VFX", meta = (WorldContext = "WorldContextObject"))
    static bool PreloadVFX(const UObject* WorldContextObject, FGameplayTag VFXTag);

    UFUNCTION(BlueprintCallable, Category = "Modular Game VFX", meta = (WorldContext = "WorldContextObject"))
    static int32 PreloadVFXTags(const UObject* WorldContextObject, const FGameplayTagContainer& VFXTags);

    UFUNCTION(BlueprintCallable, Category = "Modular Game VFX", meta = (WorldContext = "WorldContextObject"))
    static bool ReleasePreloadedVFX(const UObject* WorldContextObject, FGameplayTag VFXTag);

    UFUNCTION(BlueprintPure, Category = "Modular Game VFX", meta = (WorldContext = "WorldContextObject"))
    static bool IsVFXHandleValid(const UObject* WorldContextObject, FVFXHandle Handle);

    UFUNCTION(BlueprintPure, Category = "Modular Game VFX", meta = (WorldContext = "WorldContextObject"))
    static bool FindVFXEntry(const UObject* WorldContextObject, FGameplayTag VFXTag, FVFXCatalogEntry& OutEntry);

    /** Counts requests owned by this world, including pending loads, and their active components. */
    UFUNCTION(BlueprintPure, Category = "Modular Game VFX|Diagnostics", meta = (WorldContext = "WorldContextObject"))
    static void GetVFXRuntimeCounts(const UObject* WorldContextObject, int32& Managed, int32& Active);

private:
    static UModularGameVFXSubsystem* GetSubsystem(const UObject* WorldContextObject);
};
