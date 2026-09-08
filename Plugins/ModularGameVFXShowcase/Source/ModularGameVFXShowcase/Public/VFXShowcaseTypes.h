#pragma once
#include "CoreMinimal.h"
#include "VFXTypes.h"
#include "VFXShowcaseTypes.generated.h"

UENUM(BlueprintType)
enum class EVFXPreviewMode : uint8 { AtLocation, Attached, Projectile, Beam, Area, Screen, Environment };
UENUM(BlueprintType)
enum class EVFXShowcaseBackground : uint8 { Neutral, Bright, Dark, Complex };
UENUM(BlueprintType)
enum class EVFXShowcaseQuality : uint8 { Cinematic, High, Medium, Low, VRMobile };
UENUM(BlueprintType)
enum class EVFXReviewStatus : uint8 { NotTested, Pass, Fail, NeedsOptimization, NeedsVisualRework, ProductionReady };
UENUM(BlueprintType)
enum class EVFXReviewGate : uint8 { Visual, Gameplay, Performance, Naming, Catalog, Dependency };
UENUM(BlueprintType)
enum class EVFXGateResult : uint8 { NotAssessed, Approved, Failed };
UENUM(BlueprintType)
enum class EVFXShowcaseSort : uint8 { Category, Name, Priority, Element, ReviewStatus };
UENUM(BlueprintType)
enum class EVFXShowcaseMotion : uint8 { SwingHorizontal, SwingVertical, Spin, Dash, Jump, Land };

USTRUCT(BlueprintType)
struct MODULARGAMEVFXSHOWCASE_API FVFXReviewEvidence
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EVFXReviewGate Gate = EVFXReviewGate::Visual;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EVFXGateResult Result = EVFXGateResult::NotAssessed;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Evidence;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Reviewer;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FString TimestampUtc;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FString AssetVersion;
    bool HasApproval(const FString& Version) const;
};

USTRUCT(BlueprintType)
struct MODULARGAMEVFXSHOWCASE_API FVFXShowcaseReview
{
    GENERATED_BODY()
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FGameplayTag VFXTag;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FString AssetVersion;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EVFXReviewStatus Status = EVFXReviewStatus::NotTested;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FVFXReviewEvidence> Gates;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Notes;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString FailReason;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FString TimestampUtc;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bFavorite = false;
    bool IsProductionReady() const;
};

USTRUCT(BlueprintType)
struct MODULARGAMEVFXSHOWCASE_API FVFXShowcaseFilter
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Search;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EVFXCategory Category = EVFXCategory::Unspecified;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EVFXElement Element = EVFXElement::None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bFilterReviewStatus = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EVFXReviewStatus ReviewStatus = EVFXReviewStatus::NotTested;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bFavoritesOnly = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bRecentOnly = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EVFXShowcaseSort Sort = EVFXShowcaseSort::Category;
};

/** Metadata-only view: widgets cannot spawn a Niagara asset from this DTO. */
USTRUCT(BlueprintType)
struct MODULARGAMEVFXSHOWCASE_API FVFXShowcaseEntry
{
    GENERATED_BODY()
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FGameplayTag VFXTag;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FText DisplayName;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FString AssetPath;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FString AssetVersion;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) EVFXCategory Category = EVFXCategory::Unspecified;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) EVFXElement Element = EVFXElement::None;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) EVFXForm Form = EVFXForm::Unspecified;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) EVFXContext Context = EVFXContext::Combat;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) EVFXLifecycle Lifecycle = EVFXLifecycle::OneShot;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) EVFXPriority Priority = EVFXPriority::Combat;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) EVFXSpawnMode SpawnMode = EVFXSpawnMode::AtLocation;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FString Pooling;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FString EffectType;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 EmitterCount = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FString SimulationTarget;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FString FixedBounds;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TArray<FName> SupportedParameters;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FVFXParameterOverrides DefaultParameters;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) EVFXPreviewMode PreviewMode = EVFXPreviewMode::AtLocation;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FVFXShowcaseReview Review;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TArray<FString> ValidationIssues;
    bool Matches(const FVFXShowcaseFilter& Filter, const TArray<FGameplayTag>& Recent) const;
};

USTRUCT(BlueprintType)
struct MODULARGAMEVFXSHOWCASE_API FVFXShowcasePerformance
{
    GENERATED_BODY()
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float FPS = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float FrameTimeMs = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 ManagedVFXCount = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 ActiveVFXCount = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 ActiveNiagaraSystems = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 CurrentTestCount = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) EVFXShowcaseQuality Quality = EVFXShowcaseQuality::High;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FString ProfilingNote = TEXT("处理器、图形处理器耗时与粒子数尚未测量，请在目标设备进行性能分析。");
};

USTRUCT(BlueprintType)
struct MODULARGAMEVFXSHOWCASE_API FVFXShowcaseCategoryProgress
{
    GENERATED_BODY()
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) EVFXCategory Category = EVFXCategory::Unspecified;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 Total = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 Reviewed = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 Ready = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 Failed = 0;
};
