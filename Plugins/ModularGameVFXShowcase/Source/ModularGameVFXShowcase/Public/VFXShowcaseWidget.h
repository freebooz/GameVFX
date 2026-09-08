#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/SpinBox.h"
#include "GameplayTagContainer.h"
#include "VFXShowcaseWidget.generated.h"

class AVFXShowcaseController;
class UVerticalBox;
class UWrapBox;
class UTextBlock;
class UComboBoxString;
class UEditableTextBox;
class UMultiLineEditableTextBox;
class UVFXShowcaseWidget;

/** A dynamically generated action. Its payload is a tag or a control key, never an asset. */
UCLASS()
class MODULARGAMEVFXSHOWCASE_API UVFXShowcaseActionButton : public UButton
{
    GENERATED_BODY()
public:
    void Configure(UVFXShowcaseWidget* InOwner, const FString& InAction);
private:
    UPROPERTY() TObjectPtr<UVFXShowcaseWidget> OwnerWidget;
    FString Action;
    UFUNCTION() void Execute();
};

UCLASS()
class MODULARGAMEVFXSHOWCASE_API UVFXShowcaseNumber : public USpinBox
{
    GENERATED_BODY()
public:
    void Configure(UVFXShowcaseWidget* InOwner, const FString& InKey);
private:
    UPROPERTY() TObjectPtr<UVFXShowcaseWidget> OwnerWidget;
    FString Key;
    UFUNCTION() void Changed(float NewValue);
};

/** Native content supports an empty Widget Blueprint wrapper or Blueprint extensions. */
UCLASS(Blueprintable)
class MODULARGAMEVFXSHOWCASE_API UVFXShowcasePanel : public UUserWidget
{
    GENERATED_BODY()
public:
    UVerticalBox* GetContentBox();
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    UPROPERTY(Transient) TObjectPtr<UVerticalBox> ContentBox;
};

UCLASS(Blueprintable)
class MODULARGAMEVFXSHOWCASE_API UVFXShowcaseCategoryMenu : public UVFXShowcasePanel { GENERATED_BODY() };
UCLASS(Blueprintable)
class MODULARGAMEVFXSHOWCASE_API UVFXShowcaseEntryList : public UVFXShowcasePanel { GENERATED_BODY() };
UCLASS(Blueprintable)
class MODULARGAMEVFXSHOWCASE_API UVFXShowcaseEntryItem : public UVFXShowcasePanel { GENERATED_BODY() };
UCLASS(Blueprintable)
class MODULARGAMEVFXSHOWCASE_API UVFXShowcaseInspector : public UVFXShowcasePanel { GENERATED_BODY() };
UCLASS(Blueprintable)
class MODULARGAMEVFXSHOWCASE_API UVFXShowcaseParameterPanel : public UVFXShowcasePanel { GENERATED_BODY() };
UCLASS(Blueprintable)
class MODULARGAMEVFXSHOWCASE_API UVFXShowcasePerformancePanel : public UVFXShowcasePanel { GENERATED_BODY() };
UCLASS(Blueprintable)
class MODULARGAMEVFXSHOWCASE_API UVFXShowcaseReviewPanel : public UVFXShowcasePanel { GENERATED_BODY() };

/** Catalog-driven runtime QA workbench. All playback and mutation go through the controller. */
UCLASS(Blueprintable)
class MODULARGAMEVFXSHOWCASE_API UVFXShowcaseWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Showcase", meta=(ExposeOnSpawn=true))
    TObjectPtr<AVFXShowcaseController> Controller;

    UFUNCTION(BlueprintCallable, Category="Showcase") void RefreshCatalog();
    UFUNCTION(BlueprintCallable, Category="Showcase") void ExecuteAction(const FString& Action);
    void ChangeNumber(const FString& Key, float Value);

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;

private:
    void BuildWorkbench();
    void RebuildCategories();
    void RebuildEntries();
    void RefreshSelection();
    void BuildParameters();
    void UpdateTelemetry();
    bool SaveReview();
    UTextBlock* Label(UVerticalBox* Box, const FString& Text, int32 Size=12);
    UButton* ActionButton(UPanelWidget* Parent, const FString& Text, const FString& Action);
    UComboBoxString* Choice(UVerticalBox* Box, const FString& Title, const TArray<FString>& Options);
    void Number(UVerticalBox* Box, const FString& Name, const FString& Key, float Value, float Min, float Max);
    UFUNCTION() void FilterChanged(FString Selection, ESelectInfo::Type Type);
    UFUNCTION() void SearchChanged(const FText& Text);
    UFUNCTION() void EnvironmentChanged(FString Selection, ESelectInfo::Type Type);
    UFUNCTION() void ReviewGateChanged(FString Selection, ESelectInfo::Type Type);

    UPROPERTY(Transient) TObjectPtr<UVFXShowcaseCategoryMenu> Categories;
    UPROPERTY(Transient) TObjectPtr<UVFXShowcaseEntryList> Entries;
    UPROPERTY(Transient) TObjectPtr<UVFXShowcaseInspector> Inspector;
    UPROPERTY(Transient) TObjectPtr<UVFXShowcaseParameterPanel> Parameters;
    UPROPERTY(Transient) TObjectPtr<UVFXShowcasePerformancePanel> Performance;
    UPROPERTY(Transient) TObjectPtr<UVFXShowcaseReviewPanel> Review;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> StatusText;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> TelemetryText;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> SelectionText;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> CompareText;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> EntryCount;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> ReviewState;
    UPROPERTY(Transient) TObjectPtr<UEditableTextBox> Search;
    UPROPERTY(Transient) TObjectPtr<UComboBoxString> ElementFilter;
    UPROPERTY(Transient) TObjectPtr<UComboBoxString> ReviewFilter;
    UPROPERTY(Transient) TObjectPtr<UComboBoxString> SortFilter;
    UPROPERTY(Transient) TObjectPtr<UComboBoxString> QualityChoice;
    UPROPERTY(Transient) TObjectPtr<UComboBoxString> BackgroundChoice;
    UPROPERTY(Transient) TObjectPtr<UComboBoxString> SpeedChoice;
    UPROPERTY(Transient) TObjectPtr<UComboBoxString> VisualChoice;
    UPROPERTY(Transient) TObjectPtr<UComboBoxString> GameplayChoice;
    UPROPERTY(Transient) TObjectPtr<UComboBoxString> PerformanceChoice;
    UPROPERTY(Transient) TObjectPtr<UComboBoxString> NamingChoice;
    UPROPERTY(Transient) TObjectPtr<UComboBoxString> CatalogChoice;
    UPROPERTY(Transient) TObjectPtr<UComboBoxString> DependencyChoice;
    UPROPERTY(Transient) TObjectPtr<UEditableTextBox> Reviewer;
    UPROPERTY(Transient) TObjectPtr<UComboBoxString> FinalChoice;
    UPROPERTY(Transient) TObjectPtr<UComboBoxString> ReasonChoice;
    UPROPERTY(Transient) TObjectPtr<UMultiLineEditableTextBox> Notes;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> LoopLabel;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> AutoLabel;
    TArray<FGameplayTag> VisibleTags;
    FGameplayTag CachedTag;
    FGameplayTag CompareA;
    FGameplayTag CompareB;
    FString SelectedCategory = TEXT("All");
    FString DisplayedAssetVersion;
    TArray<FString> LastGateSelections;
    TSet<int32> DirtyReviewGates;
    bool bUpdating = false;
    double LastTelemetryTime = 0;
};
