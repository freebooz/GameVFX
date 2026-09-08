#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UObject/SoftObjectPtr.h"
#include "ModularGameVFXSettings.generated.h"

class UVFXCatalog;

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Modular Game VFX"))
class MODULARGAMEVFX_API UModularGameVFXSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UModularGameVFXSettings();

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Catalog", meta = (AllowedClasses = "/Script/ModularGameVFX.VFXCatalog"))
    TSoftObjectPtr<UVFXCatalog> DefaultCatalog;

    virtual FName GetCategoryName() const override;
    virtual FName GetSectionName() const override;
    virtual FText GetSectionText() const override;
};
