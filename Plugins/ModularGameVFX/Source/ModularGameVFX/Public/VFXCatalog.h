#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VFXTypes.h"
#include "VFXCatalog.generated.h"

UCLASS(BlueprintType)
class MODULARGAMEVFX_API UVFXCatalog : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Catalog", meta = (TitleProperty = "VFXTag"))
    TArray<FVFXCatalogEntry> Entries;

    UFUNCTION(BlueprintPure, Category = "VFX Catalog")
    bool FindEntry(FGameplayTag VFXTag, FVFXCatalogEntry& OutEntry) const;

    const FVFXCatalogEntry* FindEntryNative(const FGameplayTag& VFXTag) const;
    void BuildRuntimeCache() const;

    virtual void PostLoad() override;

#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
    UPROPERTY(Transient)
    mutable TMap<FGameplayTag, int32> EntryIndexByTag;
};
