#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VFXShowcaseTypes.h"
#include "VFXShowcaseProfile.generated.h"

USTRUCT(BlueprintType)
struct MODULARGAMEVFXSHOWCASE_API FVFXShowcaseProfileEntry
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTag VFXTag;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EVFXPreviewMode PreviewMode = EVFXPreviewMode::AtLocation;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.1")) float PreviewDuration = 3;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Units="cm", ClampMin="100")) float PreviewDistance = 1000;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName TargetType = TEXT("Target");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName DefaultCamera = TEXT("Medium");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bLoopAllowed = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EVFXShowcaseBackground RecommendedBackground = EVFXShowcaseBackground::Neutral;
};

UCLASS(BlueprintType)
class MODULARGAMEVFXSHOWCASE_API UVFXShowcaseProfile : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Showcase", meta=(TitleProperty="VFXTag")) TArray<FVFXShowcaseProfileEntry> Entries;
    const FVFXShowcaseProfileEntry* Find(FGameplayTag Tag) const;
};
