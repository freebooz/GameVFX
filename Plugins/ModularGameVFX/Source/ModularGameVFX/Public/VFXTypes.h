#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NiagaraComponentPoolMethodEnum.h"
#include "UObject/SoftObjectPtr.h"
#include "VFXTaxonomy.h"
#include "VFXTypes.generated.h"

class UNiagaraSystem;
class USceneComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogModularGameVFX, Log, All);

UENUM(BlueprintType)
enum class EVFXSpawnMode : uint8
{
    AtLocation UMETA(DisplayName = "At Location"),
    Attached UMETA(DisplayName = "Attached")
};

UENUM(BlueprintType)
enum class EVFXPriority : uint8
{
    Critical UMETA(DisplayName = "Critical"),
    Combat UMETA(DisplayName = "Combat"),
    Cosmetic UMETA(DisplayName = "Cosmetic"),
    Ambient UMETA(DisplayName = "Ambient")
};

USTRUCT(BlueprintType)
struct MODULARGAMEVFX_API FVFXHandle
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
    FGuid Id;

    FVFXHandle() = default;
    explicit FVFXHandle(const FGuid& InId) : Id(InId) {}

    bool IsValid() const { return Id.IsValid(); }

    bool operator==(const FVFXHandle& Other) const { return Id == Other.Id; }
    bool operator!=(const FVFXHandle& Other) const { return !(*this == Other); }
};

FORCEINLINE uint32 GetTypeHash(const FVFXHandle& Handle)
{
    return GetTypeHash(Handle.Id);
}

USTRUCT(BlueprintType)
struct MODULARGAMEVFX_API FVFXParameterOverrides
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Parameters")
    TMap<FName, float> FloatParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Parameters")
    TMap<FName, FVector> VectorParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Parameters")
    TMap<FName, FLinearColor> ColorParameters;

    void Append(const FVFXParameterOverrides& Overrides)
    {
        FloatParameters.Append(Overrides.FloatParameters);
        VectorParameters.Append(Overrides.VectorParameters);
        ColorParameters.Append(Overrides.ColorParameters);
    }
};

USTRUCT(BlueprintType)
struct MODULARGAMEVFX_API FVFXCatalogEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
    FGameplayTag VFXTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX", meta = (MultiLine = true))
    FText Description;

    // Unspecified keeps existing serialized catalogs compatible until classified.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Taxonomy")
    EVFXCategory Category = EVFXCategory::Unspecified;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Taxonomy")
    EVFXElement Element = EVFXElement::None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Taxonomy")
    EVFXForm Form = EVFXForm::Unspecified;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Taxonomy")
    EVFXContext Context = EVFXContext::Combat;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Taxonomy")
    EVFXLifecycle Lifecycle = EVFXLifecycle::OneShot;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Spawn")
    EVFXSpawnMode SpawnMode = EVFXSpawnMode::AtLocation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Spawn")
    FVector DefaultScale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Spawn")
    FVector LocationOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Spawn")
    FRotator RotationOffset = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Spawn")
    FName DefaultSocketName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Lifecycle")
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Lifecycle")
    ENCPoolMethod PoolingMethod = ENCPoolMethod::AutoRelease;

    // Preserve the serialized property name while exposing its semantic role in the editor.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Performance", meta = (DisplayName = "Priority"))
    EVFXPriority PerformanceClass = EVFXPriority::Combat;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Parameters")
    FVFXParameterOverrides DefaultParameters;
};

USTRUCT(BlueprintType)
struct MODULARGAMEVFX_API FVFXPlayRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FGameplayTag VFXTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFXSpawnMode SpawnMode = EVFXSpawnMode::AtLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FTransform Transform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TObjectPtr<USceneComponent> AttachComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FName SocketName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TObjectPtr<UObject> Owner = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bUseCatalogDefaultTransform = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bOverrideAutoDestroy = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (EditCondition = "bOverrideAutoDestroy"))
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bOverridePoolingMethod = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (EditCondition = "bOverridePoolingMethod"))
    ENCPoolMethod PoolingMethod = ENCPoolMethod::AutoRelease;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVFXParameterOverrides ParameterOverrides;
};

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnModularVFXReady, FVFXHandle, Handle, bool, bSuccess);
