#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "VFXShowcaseProfile.h"
#include "VFXShowcaseController.generated.h"

class UVFXCatalog;
class AVFXShowcaseEnvironment;
class UUserWidget;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVFXShowcaseChanged);

UCLASS(Blueprintable)
class MODULARGAMEVFXSHOWCASE_API AVFXShowcaseController : public AActor
{
    GENERATED_BODY()
public:
    AVFXShowcaseController();
    virtual void Tick(float DeltaSeconds) override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    /** Reuse the host combat world, pawn and camera without spawning a showcase stage. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Showcase") bool bUseExistingCombatWorld = false;
    UPROPERTY(BlueprintReadOnly, Category="Showcase") bool bHUDInteractive = false;
    UFUNCTION(BlueprintCallable, Category="Showcase") void ToggleHUDInteraction();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Showcase") TSoftObjectPtr<UVFXCatalog> Catalog;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Showcase") TObjectPtr<UVFXShowcaseProfile> Profile;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Showcase") TObjectPtr<AVFXShowcaseEnvironment> Environment;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Showcase") TSubclassOf<UUserWidget> WidgetClass;
    UPROPERTY(BlueprintReadOnly, Category="Showcase") FGameplayTag SelectedTag;
    UPROPERTY(BlueprintReadOnly, Category="Showcase") FGameplayTag CompareTag;
    UPROPERTY(BlueprintReadOnly, Category="Showcase") FVFXShowcaseFilter Filter;
    UPROPERTY(BlueprintReadOnly, Category="Showcase") FVFXParameterOverrides Parameters;
    UPROPERTY(BlueprintReadOnly, Category="Showcase") EVFXShowcaseQuality Quality = EVFXShowcaseQuality::High;
    UPROPERTY(BlueprintReadOnly, Category="Showcase") EVFXShowcaseBackground Background = EVFXShowcaseBackground::Neutral;
    UPROPERTY(BlueprintReadOnly, Category="Showcase") bool bLoop = false;
    UPROPERTY(BlueprintReadOnly, Category="Showcase") bool bAutoPreview = false;
    UPROPERTY(BlueprintReadOnly, Category="Showcase") bool bVRPreview = false;
    UPROPERTY(BlueprintReadOnly, Category="Showcase") float PlaybackSpeed = 1;
    UPROPERTY(BlueprintReadOnly, Category="Showcase") float PreviewDistance = 1000;
    UPROPERTY(BlueprintReadOnly, Category="Showcase") FString LastMessage;
    UPROPERTY(BlueprintAssignable, Category="Showcase") FVFXShowcaseChanged OnChanged;
    UFUNCTION(BlueprintCallable, Category="Showcase") void RefreshCatalog();
    UFUNCTION(BlueprintCallable, Category="Showcase") void SetFilter(const FVFXShowcaseFilter& NewFilter);
    UFUNCTION(BlueprintPure, Category="Showcase") TArray<FVFXShowcaseEntry> GetEntries() const;
    UFUNCTION(BlueprintPure, Category="Showcase") TArray<FVFXShowcaseCategoryProgress> GetCategories() const;
    UFUNCTION(BlueprintPure, Category="Showcase") TArray<EVFXElement> GetElements() const;
    UFUNCTION(BlueprintPure, Category="Showcase") bool GetSelectedEntry(FVFXShowcaseEntry& Entry) const;
    UFUNCTION(BlueprintCallable, Category="Showcase") bool SelectEntry(FGameplayTag Tag, bool bPlay = true);
    UFUNCTION(BlueprintCallable, Category="Showcase") bool Play();
    UFUNCTION(BlueprintCallable, Category="Showcase") bool PlayByTag(FGameplayTag Tag);
    UFUNCTION(BlueprintCallable, Category="Showcase") bool Replay();
    UFUNCTION(BlueprintCallable, Category="Showcase") void Stop(bool bImmediate = false);
    UFUNCTION(BlueprintCallable, Category="Showcase") void SetLoop(bool bEnabled);
    UFUNCTION(BlueprintCallable, Category="Showcase") void SetAutoPreview(bool bEnabled);
    UFUNCTION(BlueprintCallable, Category="Showcase") void Next();
    UFUNCTION(BlueprintCallable, Category="Showcase") void Previous();
    UFUNCTION(BlueprintCallable, Category="Showcase") void SetFloatParameter(FName Name, float Value);
    UFUNCTION(BlueprintCallable, Category="Showcase") void SetColorParameter(FName Name, FLinearColor Value);
    UFUNCTION(BlueprintCallable, Category="Showcase") void ResetParameters();
    UFUNCTION(BlueprintCallable, Category="Showcase") void SetQuality(EVFXShowcaseQuality NewQuality);
    UFUNCTION(BlueprintCallable, Category="Showcase") void SetBackground(EVFXShowcaseBackground NewBackground);
    UFUNCTION(BlueprintCallable, Category="Showcase") void SetPlaybackSpeed(float Speed);
    UFUNCTION(BlueprintCallable, Category="Showcase") void SetPreviewDistance(float DistanceCm);
    UFUNCTION(BlueprintCallable, Category="Showcase") void SetCameraDistance(float DistanceCm);
    UFUNCTION(BlueprintCallable, Category="Showcase") void SetVRPreview(bool bEnabled);
    UFUNCTION(BlueprintCallable, Category="Showcase") void SetMotion(EVFXShowcaseMotion Motion);
    UFUNCTION(BlueprintCallable, Category="Showcase") void SetSurface(FName Surface);
    UFUNCTION(BlueprintCallable, Category="Showcase") void PlayStress(int32 Count);
    UFUNCTION(BlueprintCallable, Category="Showcase") void PlayCompare(FGameplayTag OtherTag);
    UFUNCTION(BlueprintCallable, Category="Showcase") void PlayCombatSimulation();
    UFUNCTION(BlueprintPure, Category="Showcase") FVFXShowcasePerformance GetPerformance() const;
    UFUNCTION(BlueprintPure, Category="Showcase") FVFXShowcaseReview GetReview(FGameplayTag Tag) const;
    UFUNCTION(BlueprintCallable, Category="Showcase") bool SetReviewGate(EVFXReviewGate Gate, EVFXGateResult Result, const FString& Evidence, const FString& Reviewer);
    UFUNCTION(BlueprintCallable, Category="Showcase") bool SetReviewStatus(EVFXReviewStatus Status, const FString& Notes, const FString& FailReason);
    UFUNCTION(BlueprintCallable, Category="Showcase") bool SetReviewNotes(const FString& Notes, const FString& FailReason);
    UFUNCTION(BlueprintCallable, Category="Showcase") void ToggleFavorite();
    UFUNCTION(BlueprintCallable, Category="Showcase") bool SaveReviews();
    UFUNCTION(BlueprintCallable, Category="Showcase") bool LoadReviews();
    UFUNCTION(BlueprintPure, Category="Showcase") FString GetReviewFilePath() const;
    static int32 ClampStressCount(int32 Count) { return FMath::Clamp(Count, 1, 100); }
private:
    UPROPERTY(Transient) TObjectPtr<UVFXCatalog> LoadedCatalog;
    UPROPERTY(Transient) TObjectPtr<UUserWidget> LiveWidget;
    UPROPERTY(Transient) TArray<FVFXShowcaseEntry> AllEntries;
    UPROPERTY(Transient) TArray<FVFXShowcaseReview> Reviews;
    UPROPERTY(Transient) TArray<FGameplayTag> RecentTags;
    UPROPERTY(Transient) TArray<FVFXHandle> Handles;
    UPROPERTY(Transient) TArray<TObjectPtr<USceneComponent>> PreviewAnchors;
    TArray<TWeakObjectPtr<USceneComponent>> CombatScreenAnchors;
    FVFXShowcasePerformance Performance;
    double LastTickTime = 0;
    float SequenceElapsed = 0;
    float MotionElapsed = 0;
    int32 TestCount = 0;
    FName QualityOverrideTag;
    bool bQualityOverrideActive = false;
    float SavedTimeDilation = 1;
    bool bChangedTimeDilation = false;
    EVFXShowcaseMotion CurrentMotion = EVFXShowcaseMotion::SwingHorizontal;
    TMap<FGuid, TWeakObjectPtr<USceneComponent>> MovingAnchors;
    TMap<FGuid, float> AnchorStartTimes;
    TMap<FGuid, FVector> AnchorStartLocations;
    TMap<FGuid, FVector> AnchorTargetLocations;
    TArray<FTimerHandle> SmokeTimers;
    bool bSmokeImpactSelected = false;
    void ScheduleSmokeAction(float Delay, TFunction<void()> Action);
    void StartSmokeTest();
    void CaptureSmokeStage(const FString& Stage, bool bScreenshot);
    FVFXHandle SpawnPreview(FGameplayTag Tag, const FVector& Offset, bool bStress);
    void ClearPlayback(bool bImmediate);
    void ApplyHUDInteraction();
    bool RejectCombatWorldControl();
    void Advance(int32 Direction);
    void UpdateReviewRows();
    FVFXShowcaseReview* FindOrAddReview(FGameplayTag Tag);
    const FVFXShowcaseEntry* FindRow(FGameplayTag Tag) const;
    FVFXShowcaseProfileEntry ResolveProfile(const FVFXCatalogEntry& Entry) const;
    UFUNCTION() void OnVFXReady(FVFXHandle Handle, bool bSuccess);
};
