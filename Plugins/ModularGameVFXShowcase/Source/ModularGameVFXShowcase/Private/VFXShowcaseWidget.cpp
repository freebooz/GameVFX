#include "VFXShowcaseWidget.h"
#include "VFXShowcaseController.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"
#include "EngineUtils.h"

namespace
{
    const FLinearColor PanelColor(0.018f, 0.025f, 0.040f, 0.95f);
    const FLinearColor Accent(0.32f, 0.78f, 0.94f, 1.f);
    template<typename T> FString EnumName(T Value) { return StaticEnum<T>()->GetNameStringByValue(static_cast<int64>(Value)); }
    template<typename T> T EnumValue(const FString& Value) { return static_cast<T>(StaticEnum<T>()->GetValueByNameString(Value)); }
    UBorder* Frame(UWidgetTree* Tree, UWidget* Child)
    {
        UBorder* Border = Tree->ConstructWidget<UBorder>();
        Border->SetBrushColor(PanelColor);
        Border->SetPadding(FMargin(10.f));
        Border->SetContent(Child);
        return Border;
    }
}

void UVFXShowcaseActionButton::Configure(UVFXShowcaseWidget* InOwner, const FString& InAction)
{
    OwnerWidget = InOwner;
    Action = InAction;
    OnClicked.AddUniqueDynamic(this, &UVFXShowcaseActionButton::Execute);
}

void UVFXShowcaseActionButton::Execute() { if (IsValid(OwnerWidget)) OwnerWidget->ExecuteAction(Action); }
void UVFXShowcaseNumber::Configure(UVFXShowcaseWidget* InOwner, const FString& InKey)
{
    OwnerWidget = InOwner;
    Key = InKey;
    OnValueChanged.AddUniqueDynamic(this, &UVFXShowcaseNumber::Changed);
}
void UVFXShowcaseNumber::Changed(float NewValue) { if (IsValid(OwnerWidget)) OwnerWidget->ChangeNumber(Key, NewValue); }

UVerticalBox* UVFXShowcasePanel::GetContentBox()
{
    if (!ContentBox)
    {
        ContentBox = WidgetTree->ConstructWidget<UVerticalBox>();
        WidgetTree->RootWidget = ContentBox;
    }
    return ContentBox;
}

TSharedRef<SWidget> UVFXShowcasePanel::RebuildWidget()
{
    GetContentBox();
    return Super::RebuildWidget();
}

TSharedRef<SWidget> UVFXShowcaseWidget::RebuildWidget()
{
    if (!Categories) BuildWorkbench();
    return Super::RebuildWidget();
}

UTextBlock* UVFXShowcaseWidget::Label(UVerticalBox* Box, const FString& Text, int32 Size)
{
    UTextBlock* Result = WidgetTree->ConstructWidget<UTextBlock>();
    Result->SetText(FText::FromString(Text));
    FSlateFontInfo Font = Result->GetFont();
    Font.Size = Size;
    Result->SetFont(Font);
    Result->SetColorAndOpacity(FSlateColor(FLinearColor(0.88f, 0.92f, 0.98f)));
    Result->SetAutoWrapText(true);
    Box->AddChildToVerticalBox(Result)->SetPadding(FMargin(0, 3));
    return Result;
}

UButton* UVFXShowcaseWidget::ActionButton(UPanelWidget* Parent, const FString& Text, const FString& Action)
{
    UVFXShowcaseActionButton* Button = WidgetTree->ConstructWidget<UVFXShowcaseActionButton>();
    Button->Configure(this, Action);
    Button->SetBackgroundColor(FLinearColor(0.11f, 0.20f, 0.29f));
    UTextBlock* Caption = WidgetTree->ConstructWidget<UTextBlock>();
    Caption->SetText(FText::FromString(Text));
    FSlateFontInfo Font = Caption->GetFont(); Font.Size = 11; Caption->SetFont(Font);
    Caption->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    Caption->SetAutoWrapText(true);
    Button->SetContent(Caption);
    if (UWrapBox* Wrap = Cast<UWrapBox>(Parent)) Wrap->AddChildToWrapBox(Button)->SetPadding(FMargin(3));
    else if (UVerticalBox* Vertical = Cast<UVerticalBox>(Parent)) Vertical->AddChildToVerticalBox(Button)->SetPadding(FMargin(0, 3));
    else Parent->AddChild(Button);
    return Button;
}

UComboBoxString* UVFXShowcaseWidget::Choice(UVerticalBox* Box, const FString& Title, const TArray<FString>& Options)
{
    Label(Box, Title);
    UComboBoxString* Combo = WidgetTree->ConstructWidget<UComboBoxString>();
    for (const FString& Option : Options) Combo->AddOption(Option);
    if (Options.Num()) Combo->SetSelectedOption(Options[0]);
    Box->AddChildToVerticalBox(Combo)->SetPadding(FMargin(0, 2));
    return Combo;
}

void UVFXShowcaseWidget::Number(UVerticalBox* Box, const FString& Name, const FString& Key, float Value, float Min, float Max)
{
    UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
    UTextBlock* Caption = WidgetTree->ConstructWidget<UTextBlock>();
    Caption->SetText(FText::FromString(Name));
    FSlateFontInfo Font = Caption->GetFont(); Font.Size = 11; Caption->SetFont(Font);
    Row->AddChildToHorizontalBox(Caption)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    UVFXShowcaseNumber* Spin = WidgetTree->ConstructWidget<UVFXShowcaseNumber>();
    Spin->SetMinValue(Min); Spin->SetMaxValue(Max);
    Spin->SetMinSliderValue(Min); Spin->SetMaxSliderValue(Max);
    Spin->SetMinDesiredWidth(110.f);
    Spin->SetDelta(.05f);
    Spin->SetValue(Value);
    Spin->Configure(this, Key);
    Row->AddChildToHorizontalBox(Spin);
    Box->AddChildToVerticalBox(Row)->SetPadding(FMargin(0, 3));
}

void UVFXShowcaseWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (!Controller && GetWorld())
        for (TActorIterator<AVFXShowcaseController> It(GetWorld()); It; ++It) { Controller = *It; break; }
    if (!Categories) BuildWorkbench();
    RefreshCatalog();
}

void UVFXShowcaseWidget::BuildWorkbench()
{
    bUpdating = true;
    UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>();
    WidgetTree->RootWidget = Root;
    UVerticalBox* Header = WidgetTree->ConstructWidget<UVerticalBox>();
    Label(Header, TEXT("VFX SHOWCASE  /  Catalog & acceptance workbench"), 19)->SetColorAndOpacity(Accent);
    StatusText = Label(Header, TEXT("Choose a catalog entry to play through VFX Manager."));
    Root->AddChildToVerticalBox(Frame(WidgetTree, Header));

    UHorizontalBox* Body = WidgetTree->ConstructWidget<UHorizontalBox>();
    Root->AddChildToVerticalBox(Body)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    UVerticalBox* Browser = WidgetTree->ConstructWidget<UVerticalBox>();
    USizeBox* BrowserSize = WidgetTree->ConstructWidget<USizeBox>();
    BrowserSize->SetWidthOverride(310); BrowserSize->SetContent(Frame(WidgetTree, Browser));
    Body->AddChildToHorizontalBox(BrowserSize);
    Search = WidgetTree->ConstructWidget<UEditableTextBox>();
    Search->SetHintText(FText::FromString(TEXT("Search name, tag, element, form")));
    Search->OnTextChanged.AddDynamic(this, &UVFXShowcaseWidget::SearchChanged);
    Browser->AddChildToVerticalBox(Search);

    Categories = CreateWidget<UVFXShowcaseCategoryMenu>(this);
    UScrollBox* CategoryScroll = WidgetTree->ConstructWidget<UScrollBox>();
    CategoryScroll->SetOrientation(Orient_Horizontal);
    CategoryScroll->AddChild(Categories);
    USizeBox* CategorySize = WidgetTree->ConstructWidget<USizeBox>();
    CategorySize->SetHeightOverride(112); CategorySize->SetContent(CategoryScroll);
    Browser->AddChildToVerticalBox(CategorySize);
    ElementFilter = Choice(Browser, TEXT("Element"), {TEXT("All")});
    ReviewFilter = Choice(Browser, TEXT("Review status"), {TEXT("All"), TEXT("NotTested"), TEXT("Pass"), TEXT("Fail"), TEXT("NeedsOptimization"), TEXT("NeedsVisualRework"), TEXT("ProductionReady")});
    SortFilter = Choice(Browser, TEXT("Sort"), {TEXT("Category / Element / Name"), TEXT("Name"), TEXT("Priority"), TEXT("Element"), TEXT("ReviewStatus")});
    for (UComboBoxString* Combo : {ElementFilter.Get(), ReviewFilter.Get(), SortFilter.Get()})
        Combo->OnSelectionChanged.AddDynamic(this, &UVFXShowcaseWidget::FilterChanged);
    EntryCount = Label(Browser, TEXT("0 entries"));
    Entries = CreateWidget<UVFXShowcaseEntryList>(this);
    UScrollBox* EntryScroll = WidgetTree->ConstructWidget<UScrollBox>();
    EntryScroll->AddChild(Entries);
    Browser->AddChildToVerticalBox(EntryScroll)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

    UVerticalBox* Stage = WidgetTree->ConstructWidget<UVerticalBox>();
    Body->AddChildToHorizontalBox(Stage)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    Performance = CreateWidget<UVFXShowcasePerformancePanel>(this);
    Label(Performance->GetContentBox(), TEXT("LIVE PERFORMANCE"), 13)->SetColorAndOpacity(Accent);
    TelemetryText = Label(Performance->GetContentBox(), TEXT("Waiting for controller"));
    Stage->AddChildToVerticalBox(Frame(WidgetTree, Performance));
    USpacer* Space = WidgetTree->ConstructWidget<USpacer>();
    Space->SetVisibility(ESlateVisibility::HitTestInvisible);
    Stage->AddChildToVerticalBox(Space)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    UVerticalBox* StageFooter = WidgetTree->ConstructWidget<UVerticalBox>();
    SelectionText = Label(StageFooter, TEXT("No VFX selected"), 15);
    CompareText = Label(StageFooter, TEXT("Compare A: —  |  B: —"));
    Label(StageFooter, TEXT("Select an entry for Single mode. Stress uses its own test zone.\nRight mouse + WASD: inspect the stage (when viewport has focus)."), 10);
    Stage->AddChildToVerticalBox(Frame(WidgetTree, StageFooter));

    UVerticalBox* Details = WidgetTree->ConstructWidget<UVerticalBox>();
    UScrollBox* DetailScroll = WidgetTree->ConstructWidget<UScrollBox>();
    DetailScroll->AddChild(Details);
    USizeBox* DetailSize = WidgetTree->ConstructWidget<USizeBox>();
    DetailSize->SetWidthOverride(350); DetailSize->SetContent(Frame(WidgetTree, DetailScroll));
    Body->AddChildToHorizontalBox(DetailSize);
    Inspector = CreateWidget<UVFXShowcaseInspector>(this); Details->AddChildToVerticalBox(Inspector);
    Parameters = CreateWidget<UVFXShowcaseParameterPanel>(this); Details->AddChildToVerticalBox(Parameters);
    Label(Details, TEXT("ENVIRONMENT"), 15)->SetColorAndOpacity(Accent);
    BackgroundChoice = Choice(Details, TEXT("Background"), {TEXT("Neutral"), TEXT("Bright"), TEXT("Dark"), TEXT("Complex")});
    QualityChoice = Choice(Details, TEXT("Quality"), {TEXT("Cinematic"), TEXT("High"), TEXT("Medium"), TEXT("Low"), TEXT("VRMobile")});
    QualityChoice->SetSelectedOption(TEXT("High"));
    SpeedChoice = Choice(Details, TEXT("Playback speed"), {TEXT("1.0x"), TEXT("0.5x"), TEXT("0.25x"), TEXT("0.1x")});
    for (UComboBoxString* Combo : {BackgroundChoice.Get(), QualityChoice.Get(), SpeedChoice.Get()})
        Combo->OnSelectionChanged.AddDynamic(this, &UVFXShowcaseWidget::EnvironmentChanged);
    Label(Details, TEXT("Preview / beam distance"));
    UWrapBox* Distances = WidgetTree->ConstructWidget<UWrapBox>(); Details->AddChildToVerticalBox(Distances);
    for (int32 Distance : {3, 5, 10, 20, 30, 50}) ActionButton(Distances, FString::Printf(TEXT("%dm"), Distance), FString::Printf(TEXT("Distance:%d"), Distance));
    Label(Details, TEXT("Camera distance"));
    UWrapBox* Cameras = WidgetTree->ConstructWidget<UWrapBox>(); Details->AddChildToVerticalBox(Cameras);
    ActionButton(Cameras, TEXT("Near 3m"), TEXT("Camera:3"));
    ActionButton(Cameras, TEXT("Medium 10m"), TEXT("Camera:10"));
    ActionButton(Cameras, TEXT("Far 40m"), TEXT("Camera:40"));
    ActionButton(Details, TEXT("VR preview / VRMobile quality"), TEXT("VR"));
    UWrapBox* Motion = WidgetTree->ConstructWidget<UWrapBox>(); Details->AddChildToVerticalBox(Motion);
    for (const FString& Name : {TEXT("Horizontal"), TEXT("Vertical"), TEXT("Spin"), TEXT("Dash"), TEXT("Jump"), TEXT("Land")}) ActionButton(Motion, Name, TEXT("Motion:") + Name);
    UWrapBox* Surfaces = WidgetTree->ConstructWidget<UWrapBox>(); Details->AddChildToVerticalBox(Surfaces);
    for (const FString& Name : {TEXT("Stone"), TEXT("Metal"), TEXT("Wood"), TEXT("Ground"), TEXT("Wall")}) ActionButton(Surfaces, Name, TEXT("Surface:") + Name);

    Review = CreateWidget<UVFXShowcaseReviewPanel>(this); Details->AddChildToVerticalBox(Review);
    UVerticalBox* ReviewBox = Review->GetContentBox();
    Label(ReviewBox, TEXT("MANUAL REVIEW"), 15)->SetColorAndOpacity(Accent);
    ReviewState = Label(ReviewBox, TEXT("No review selected"));
    Label(ReviewBox, TEXT("Check: focus, impact, force, speed, depth, color, silhouette, timing, readability, end feedback and performance."), 10);
    const TArray<FString> Verdicts = {TEXT("NotTested"), TEXT("Pass"), TEXT("Fail")};
    VisualChoice = Choice(ReviewBox, TEXT("Visual"), Verdicts);
    GameplayChoice = Choice(ReviewBox, TEXT("Gameplay"), Verdicts);
    PerformanceChoice = Choice(ReviewBox, TEXT("Performance"), Verdicts);
    NamingChoice = Choice(ReviewBox, TEXT("Naming"), Verdicts);
    CatalogChoice = Choice(ReviewBox, TEXT("Catalog"), Verdicts);
    DependencyChoice = Choice(ReviewBox, TEXT("Dependency"), Verdicts);
    for (UComboBoxString* GateChoice : {VisualChoice.Get(), GameplayChoice.Get(), PerformanceChoice.Get(), NamingChoice.Get(), CatalogChoice.Get(), DependencyChoice.Get()})
        GateChoice->OnSelectionChanged.AddDynamic(this, &UVFXShowcaseWidget::ReviewGateChanged);
    FinalChoice = Choice(ReviewBox, TEXT("Final verdict (KeepCurrent preserves recorded gates)"), {TEXT("KeepCurrent"), TEXT("Pass"), TEXT("Fail"), TEXT("NeedsOptimization"), TEXT("NeedsVisualRework"), TEXT("ProductionReady"), TEXT("ResetAllToNotTested")});
    ReasonChoice = Choice(ReviewBox, TEXT("Failure reason"), {TEXT("None"), TEXT("WeakVisual"), TEXT("WeakImpact"), TEXT("PoorLayering"), TEXT("ColorIssue"), TEXT("ExcessiveBloom"), TEXT("Occlusion"), TEXT("LifetimeIssue"), TEXT("Overdraw"), TEXT("TooManyParticles"), TEXT("HighCost"), TEXT("VRIssue"), TEXT("WrongCategory"), TEXT("CatalogIssue"), TEXT("Other")});
    Notes = WidgetTree->ConstructWidget<UMultiLineEditableTextBox>();
    Reviewer = WidgetTree->ConstructWidget<UEditableTextBox>();
    Reviewer->SetHintText(FText::FromString(TEXT("Reviewer name (required for gate evidence)")));
    ReviewBox->AddChildToVerticalBox(Reviewer);
    Notes->SetHintText(FText::FromString(TEXT("Evidence / observations for the chosen gate verdicts")));
    USizeBox* NoteSize = WidgetTree->ConstructWidget<USizeBox>(); NoteSize->SetHeightOverride(100); NoteSize->SetContent(Notes);
    ReviewBox->AddChildToVerticalBox(NoteSize);
    UWrapBox* ReviewButtons = WidgetTree->ConstructWidget<UWrapBox>(); ReviewBox->AddChildToVerticalBox(ReviewButtons);
    ActionButton(ReviewButtons, TEXT("Save review"), TEXT("SaveReview"));
    ActionButton(ReviewButtons, TEXT("Save + Next"), TEXT("SaveNext"));
    ActionButton(ReviewButtons, TEXT("Final PASS"), TEXT("FinalPass"));
    ActionButton(ReviewButtons, TEXT("FAIL"), TEXT("FinalFail"));
    ActionButton(ReviewButtons, TEXT("Toggle Favorite"), TEXT("Favorite"));
    Label(ReviewBox, TEXT("Production Ready requires all six gates approved with reviewer + evidence for this asset version. Saves review data only."), 10);

    UVerticalBox* Controls = WidgetTree->ConstructWidget<UVerticalBox>();
    UWrapBox* Playback = WidgetTree->ConstructWidget<UWrapBox>(); Controls->AddChildToVerticalBox(Playback);
    for (const FString& Name : {TEXT("Previous"), TEXT("Play"), TEXT("Replay"), TEXT("Stop"), TEXT("Stop Immediate"), TEXT("Next")}) ActionButton(Playback, Name, Name);
    LoopLabel = Cast<UTextBlock>(ActionButton(Playback, TEXT("Loop: OFF"), TEXT("Loop"))->GetContent());
    AutoLabel = Cast<UTextBlock>(ActionButton(Playback, TEXT("Auto Preview: OFF"), TEXT("Auto"))->GetContent());
    ActionButton(Playback, TEXT("Set A"), TEXT("SetA")); ActionButton(Playback, TEXT("Set B"), TEXT("SetB"));
    ActionButton(Playback, TEXT("Compare A / B"), TEXT("Compare"));
    ActionButton(Playback, TEXT("Combat simulation"), TEXT("Combat"));
    ActionButton(Playback, TEXT("Refresh / Validate"), TEXT("Refresh"));
    UWrapBox* Stress = WidgetTree->ConstructWidget<UWrapBox>(); Controls->AddChildToVerticalBox(Stress);
    for (int32 Count : {1, 5, 10, 20, 50, 100}) ActionButton(Stress, FString::Printf(TEXT("Stress %d"), Count), FString::Printf(TEXT("Stress:%d"), Count));
    Root->AddChildToVerticalBox(Frame(WidgetTree, Controls));
    bUpdating = false;
}

void UVFXShowcaseWidget::RefreshCatalog()
{
    if (!Categories) return;
    if (!IsValid(Controller))
    {
        StatusText->SetText(FText::FromString(TEXT("No Showcase Controller in this world. Place BP_VFX_ShowcaseController in the level.")));
        return;
    }
    Controller->RefreshCatalog();
    bUpdating = true;
    const FString PreviousElement = ElementFilter->GetSelectedOption();
    ElementFilter->ClearOptions(); ElementFilter->AddOption(TEXT("All"));
    for (EVFXElement Element : Controller->GetElements()) ElementFilter->AddOption(EnumName(Element));
    ElementFilter->SetSelectedOption(ElementFilter->FindOptionIndex(PreviousElement) == INDEX_NONE ? TEXT("All") : PreviousElement);
    bUpdating = false;
    RebuildCategories(); RebuildEntries(); RefreshSelection(); UpdateTelemetry();
}

void UVFXShowcaseWidget::RebuildCategories()
{
    UVerticalBox* Box = Categories->GetContentBox(); Box->ClearChildren();
    UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(); Box->AddChildToVerticalBox(Row);
    for (const FString& Name : {TEXT("All"), TEXT("Favorites"), TEXT("Recent"), TEXT("Failed"), TEXT("Unreviewed")})
    {
        UButton* Button = ActionButton(Row, Name, TEXT("Category:") + Name);
        if (SelectedCategory == Name) Button->SetBackgroundColor(Accent);
    }
    UHorizontalBox* CatalogRow = WidgetTree->ConstructWidget<UHorizontalBox>(); Box->AddChildToVerticalBox(CatalogRow);
    for (const FVFXShowcaseCategoryProgress& Progress : Controller->GetCategories())
    {
        const FString Name = EnumName(Progress.Category);
        const FString State = Progress.Failed > 0 ? TEXT("Failed") : (Progress.Total > 0 && Progress.Ready == Progress.Total ? TEXT("Ready") : TEXT("Needs review"));
        UButton* Button = ActionButton(CatalogRow, FString::Printf(TEXT("%s\n%d / %d reviewed\n%s"), *Name, Progress.Reviewed, Progress.Total, *State), TEXT("Category:") + Name);
        if (SelectedCategory == Name) Button->SetBackgroundColor(Accent);
    }
}

void UVFXShowcaseWidget::RebuildEntries()
{
    if (!Controller) return;
    UVerticalBox* Box = Entries->GetContentBox(); Box->ClearChildren(); VisibleTags.Reset();
    for (const FVFXShowcaseEntry& Entry : Controller->GetEntries())
    {
        VisibleTags.Add(Entry.VFXTag);
        UVFXShowcaseEntryItem* Item = CreateWidget<UVFXShowcaseEntryItem>(this);
        const FString Caption = FString::Printf(TEXT("%s%s\n%s\n%s / %s  |  %s  |  %s"),
            Entry.Review.bFavorite ? TEXT("★ ") : TEXT(""), *Entry.DisplayName.ToString(), *Entry.VFXTag.ToString(),
            *EnumName(Entry.Element), *EnumName(Entry.Form), *EnumName(Entry.Priority), *EnumName(Entry.Review.Status));
        UButton* Button = ActionButton(Item->GetContentBox(), Caption, TEXT("Entry:") + Entry.VFXTag.ToString());
        if (Entry.VFXTag == Controller->SelectedTag) Button->SetBackgroundColor(FLinearColor(.12f, .43f, .57f));
        Box->AddChildToVerticalBox(Item)->SetPadding(FMargin(0, 2));
    }
    EntryCount->SetText(FText::FromString(FString::Printf(TEXT("%d entries · %s"), VisibleTags.Num(), *SelectedCategory)));
    if (VisibleTags.IsEmpty()) Label(Box, TEXT("No matching entries. Clear filters or register a valid Catalog entry."));
}

void UVFXShowcaseWidget::FilterChanged(FString Selection, ESelectInfo::Type Type)
{
    if (bUpdating || !Controller) return;
    FVFXShowcaseFilter Filter;
    Filter.Search = Search->GetText().ToString();
    if (SelectedCategory == TEXT("Favorites")) Filter.bFavoritesOnly = true;
    else if (SelectedCategory == TEXT("Recent")) Filter.bRecentOnly = true;
    else if (SelectedCategory == TEXT("Failed")) { Filter.bFilterReviewStatus = true; Filter.ReviewStatus = EVFXReviewStatus::Fail; }
    else if (SelectedCategory == TEXT("Unreviewed")) { Filter.bFilterReviewStatus = true; Filter.ReviewStatus = EVFXReviewStatus::NotTested; }
    else if (SelectedCategory != TEXT("All")) Filter.Category = EnumValue<EVFXCategory>(SelectedCategory);
    if (ElementFilter->GetSelectedOption() != TEXT("All")) Filter.Element = EnumValue<EVFXElement>(ElementFilter->GetSelectedOption());
    if (ReviewFilter->GetSelectedOption() != TEXT("All"))
    {
        Filter.bFilterReviewStatus = true;
        Filter.ReviewStatus = EnumValue<EVFXReviewStatus>(ReviewFilter->GetSelectedOption());
    }
    Filter.Sort = SortFilter->GetSelectedIndex() == 0 ? EVFXShowcaseSort::Category : EnumValue<EVFXShowcaseSort>(SortFilter->GetSelectedOption());
    Controller->SetFilter(Filter);
    RebuildCategories(); RebuildEntries();
}

void UVFXShowcaseWidget::SearchChanged(const FText& Text) { FilterChanged(FString(), ESelectInfo::Direct); }

void UVFXShowcaseWidget::RefreshSelection()
{
    DirtyReviewGates.Reset();
    LastGateSelections.Reset();
    DisplayedAssetVersion.Reset();
    if (!Controller) return;
    CachedTag = Controller->SelectedTag;
    UVerticalBox* Box = Inspector->GetContentBox(); Box->ClearChildren();
    Label(Box, TEXT("INSPECTOR"), 15)->SetColorAndOpacity(Accent);
    FVFXShowcaseEntry Entry;
    const bool bSelected = Controller->GetSelectedEntry(Entry);
    DisplayedAssetVersion = bSelected ? Entry.AssetVersion : FString();
    Parameters->SetIsEnabled(bSelected); Review->SetIsEnabled(bSelected);
    if (!bSelected)
    {
        Label(Box, TEXT("Select an entry from the Catalog."));
        return;
    }
    SelectionText->SetText(FText::FromString(Entry.DisplayName.ToString() + TEXT("\n") + Entry.VFXTag.ToString()));
    Label(Box, Entry.DisplayName.ToString(), 14);
    Label(Box, Entry.VFXTag.ToString(), 10);
    Label(Box, FString::Printf(TEXT("Category: %s\nElement: %s   Form: %s\nContext: %s   Lifecycle: %s\nPriority: %s   Pooling: %s\nSpawn: %s   Preview: %s\nReview: %s"),
        *EnumName(Entry.Category), *EnumName(Entry.Element), *EnumName(Entry.Form), *EnumName(Entry.Context), *EnumName(Entry.Lifecycle), *EnumName(Entry.Priority),
        *Entry.Pooling, *EnumName(Entry.SpawnMode), *EnumName(Entry.PreviewMode), *EnumName(Entry.Review.Status)));
    Label(Box, FString::Printf(TEXT("System path: %s\nEffect Type: %s\nEmitters: %d\nSimulation: %s\nFixed Bounds: %s"), *Entry.AssetPath, *Entry.EffectType, Entry.EmitterCount, *Entry.SimulationTarget, *Entry.FixedBounds), 10);
    for (const FString& Issue : Entry.ValidationIssues) Label(Box, TEXT("Validation: ") + Issue, 10)->SetColorAndOpacity(FLinearColor(1.f, .55f, .25f));
    BuildParameters();
    const FVFXShowcaseReview Record = Controller->GetReview(CachedTag);
    for (const FVFXReviewEvidence& Gate : Record.Gates)
        if (Gate.Result != EVFXGateResult::NotAssessed)
            Label(Box, FString::Printf(TEXT("%s: %s%s\nVersion: %s\nReviewer: %s · %s\nEvidence: %s"), *EnumName(Gate.Gate), *EnumName(Gate.Result), Gate.AssetVersion == Entry.AssetVersion ? TEXT("") : TEXT(" [HISTORICAL / expired — read only]"), *Gate.AssetVersion, *Gate.Reviewer, *Gate.TimestampUtc, *Gate.Evidence), 10);
    bUpdating = true;
    const TArray<UComboBoxString*> Choices = {VisualChoice, GameplayChoice, PerformanceChoice, NamingChoice, CatalogChoice, DependencyChoice};
    for (int32 Index = 0; Index < Choices.Num(); ++Index)
    {
        Choices[Index]->SetSelectedOption(TEXT("NotTested"));
        for (const FVFXReviewEvidence& Gate : Record.Gates)
            if (static_cast<int32>(Gate.Gate) == Index && Gate.AssetVersion == Entry.AssetVersion)
                Choices[Index]->SetSelectedOption(Gate.Result == EVFXGateResult::Approved ? TEXT("Pass") : Gate.Result == EVFXGateResult::Failed ? TEXT("Fail") : TEXT("NotTested"));
        LastGateSelections.Add(Choices[Index]->GetSelectedOption());
    }
    FinalChoice->SetSelectedOption(TEXT("KeepCurrent"));
    ReasonChoice->SetSelectedOption(Record.FailReason.IsEmpty() ? TEXT("None") : Record.FailReason);
    Notes->SetText(FText::FromString(Record.Notes));
    ReviewState->SetText(FText::FromString(FString::Printf(TEXT("%s  ·  %s\nVersion: %s"), *EnumName(Record.Status), Record.bFavorite ? TEXT("Favorite") : TEXT("Not favorited"), *Entry.AssetVersion)));
    bUpdating = false;
}

void UVFXShowcaseWidget::ReviewGateChanged(FString Selection, ESelectInfo::Type Type)
{
    if (bUpdating) return;
    const TArray<UComboBoxString*> Choices = {VisualChoice, GameplayChoice, PerformanceChoice, NamingChoice, CatalogChoice, DependencyChoice};
    if (LastGateSelections.Num() != Choices.Num()) return;
    for (int32 Index = 0; Index < Choices.Num(); ++Index)
    {
        const FString CurrentSelection = Choices[Index]->GetSelectedOption();
        if (CurrentSelection != LastGateSelections[Index])
        {
            DirtyReviewGates.Add(Index);
            LastGateSelections[Index] = CurrentSelection;
        }
    }
}

void UVFXShowcaseWidget::BuildParameters()
{
    UVerticalBox* Box = Parameters->GetContentBox(); Box->ClearChildren();
    Label(Box, TEXT("PARAMETERS"), 15)->SetColorAndOpacity(Accent);
    FVFXShowcaseEntry Entry;
    if (!Controller->GetSelectedEntry(Entry)) return;
    for (const FString& Name : {TEXT("Intensity"), TEXT("Scale"), TEXT("LifetimeScale"), TEXT("Radius"), TEXT("Width"), TEXT("Length"), TEXT("Progress")})
    {
        const FName Key(*(TEXT("User.") + Name));
        if (!Entry.SupportedParameters.Contains(Key)) continue;
        const float* Value = Controller->Parameters.FloatParameters.Find(Key);
        const float* Default = Entry.DefaultParameters.FloatParameters.Find(Key);
        Number(Box, Name, Key.ToString(), Value ? *Value : Default ? *Default : 1.f, Name == TEXT("Progress") ? 0.f : .01f,
            Name == TEXT("Progress") ? 1.f : (Name == TEXT("Radius") || Name == TEXT("Length") ? 5000.f : 20.f));
    }
    for (const FString& ColorName : {TEXT("PrimaryColor"), TEXT("SecondaryColor")})
    {
        const FName Key(*(TEXT("User.") + ColorName));
        if (!Entry.SupportedParameters.Contains(Key)) continue;
        const FLinearColor* Value = Controller->Parameters.ColorParameters.Find(Key);
        const FLinearColor* Default = Entry.DefaultParameters.ColorParameters.Find(Key);
        const FLinearColor Color = Value ? *Value : Default ? *Default : FLinearColor::White;
        Label(Box, ColorName + TEXT(" · linear RGBA"));
        Number(Box, TEXT("R"), Key.ToString() + TEXT(":R"), Color.R, 0, 10);
        Number(Box, TEXT("G"), Key.ToString() + TEXT(":G"), Color.G, 0, 10);
        Number(Box, TEXT("B"), Key.ToString() + TEXT(":B"), Color.B, 0, 10);
        Number(Box, TEXT("A"), Key.ToString() + TEXT(":A"), Color.A, 0, 1);
    }
    if (Entry.SupportedParameters.IsEmpty()) Label(Box, TEXT("This resource exposes no supported user parameters."));
    ActionButton(Box, TEXT("Reset to Catalog defaults"), TEXT("Reset"));
}

void UVFXShowcaseWidget::ChangeNumber(const FString& Key, float Value)
{
    if (bUpdating || !Controller) return;
    FString ColorName, Channel;
    if (Key.Split(TEXT(":"), &ColorName, &Channel))
    {
        const FName Name(*ColorName);
        FVFXShowcaseEntry Entry; Controller->GetSelectedEntry(Entry);
        const FLinearColor* Existing = Controller->Parameters.ColorParameters.Find(Name);
        const FLinearColor* Default = Entry.DefaultParameters.ColorParameters.Find(Name);
        FLinearColor Color = Existing ? *Existing : Default ? *Default : FLinearColor::White;
        if (Channel == TEXT("R")) Color.R = Value;
        else if (Channel == TEXT("G")) Color.G = Value;
        else if (Channel == TEXT("B")) Color.B = Value;
        else Color.A = Value;
        Controller->SetColorParameter(Name, Color);
    }
    else Controller->SetFloatParameter(FName(*Key), Value);
}

void UVFXShowcaseWidget::EnvironmentChanged(FString Selection, ESelectInfo::Type Type)
{
    if (bUpdating || !Controller) return;
    const EVFXShowcaseBackground Background = EnumValue<EVFXShowcaseBackground>(BackgroundChoice->GetSelectedOption());
    const EVFXShowcaseQuality Quality = EnumValue<EVFXShowcaseQuality>(QualityChoice->GetSelectedOption());
    const float Speed = FCString::Atof(*SpeedChoice->GetSelectedOption());
    if (Controller->Background != Background) Controller->SetBackground(Background);
    if (Controller->Quality != Quality) Controller->SetQuality(Quality);
    if (!FMath::IsNearlyEqual(Controller->PlaybackSpeed, Speed)) Controller->SetPlaybackSpeed(Speed);
}

bool UVFXShowcaseWidget::SaveReview()
{
    if (!Controller || !Controller->SelectedTag.IsValid()) return false;
    FVFXShowcaseEntry SelectedEntry;
    if (!Controller->GetSelectedEntry(SelectedEntry) || CachedTag != Controller->SelectedTag || DisplayedAssetVersion != SelectedEntry.AssetVersion)
    {
        RefreshSelection();
        Controller->LastMessage = TEXT("Review form refreshed: selection or asset version changed. Reassess and explicitly select the intended gate verdicts.");
        StatusText->SetText(FText::FromString(Controller->LastMessage));
        return false;
    }
    const FString Evidence = Notes->GetText().ToString();
    const FString ReviewerName = Reviewer->GetText().ToString();
    const TArray<UComboBoxString*> Choices = {VisualChoice, GameplayChoice, PerformanceChoice, NamingChoice, CatalogChoice, DependencyChoice};
    const FString FinalSelection = FinalChoice->GetSelectedOption();
    const bool bResetAllGates = FinalSelection == TEXT("ResetAllToNotTested");
    for (int32 Index = 0; Index < Choices.Num(); ++Index)
    {
        // Never re-stamp a displayed or historical verdict unless this form explicitly edited it.
        if (bResetAllGates || !DirtyReviewGates.Contains(Index)) continue;
        const FString Verdict = Choices[Index]->GetSelectedOption();
        const EVFXGateResult Result = Verdict == TEXT("Pass") ? EVFXGateResult::Approved : Verdict == TEXT("Fail") ? EVFXGateResult::Failed : EVFXGateResult::NotAssessed;
        if (!Controller->SetReviewGate(static_cast<EVFXReviewGate>(Index), Result, Evidence, ReviewerName))
        {
            StatusText->SetText(FText::FromString(Controller->LastMessage));
            return false;
        }
    }
    if (FinalSelection != TEXT("KeepCurrent") && !Controller->SetReviewStatus(bResetAllGates ? EVFXReviewStatus::NotTested : EnumValue<EVFXReviewStatus>(FinalSelection), Evidence, ReasonChoice->GetSelectedOption() == TEXT("None") ? FString() : ReasonChoice->GetSelectedOption()))
    {
        StatusText->SetText(FText::FromString(Controller->LastMessage)); return false;
    }
    if (FinalSelection == TEXT("KeepCurrent"))
        Controller->SetReviewNotes(Evidence, ReasonChoice->GetSelectedOption() == TEXT("None") ? FString() : ReasonChoice->GetSelectedOption());
    if (!Controller->SaveReviews()) return false;
    RebuildCategories(); RebuildEntries(); RefreshSelection();
    return true;
}

void UVFXShowcaseWidget::ExecuteAction(const FString& Action)
{
    if (!Controller) return;
    FString Kind, Value;
    if (Action.Split(TEXT(":"), &Kind, &Value))
    {
        if (Kind == TEXT("Category"))
        {
            SelectedCategory = Value;
            if (Value == TEXT("Failed") || Value == TEXT("Unreviewed"))
            {
                bUpdating = true; ReviewFilter->SetSelectedOption(TEXT("All")); bUpdating = false;
            }
            FilterChanged(FString(), ESelectInfo::Direct);
        }
        else if (Kind == TEXT("Entry")) { Controller->PlayByTag(FGameplayTag::RequestGameplayTag(FName(*Value), false)); RefreshSelection(); RebuildEntries(); }
        else if (Kind == TEXT("Stress")) Controller->PlayStress(FCString::Atoi(*Value));
        else if (Kind == TEXT("Distance")) Controller->SetPreviewDistance(FCString::Atof(*Value) * 100.f);
        else if (Kind == TEXT("Camera")) Controller->SetCameraDistance(FCString::Atof(*Value) * 100.f);
        else if (Kind == TEXT("Motion")) Controller->SetMotion(EnumValue<EVFXShowcaseMotion>(Value == TEXT("Horizontal") ? TEXT("SwingHorizontal") : Value == TEXT("Vertical") ? TEXT("SwingVertical") : Value));
        else if (Kind == TEXT("Surface")) Controller->SetSurface(FName(*Value));
    }
    else if (Action == TEXT("Play")) Controller->Play();
    else if (Action == TEXT("Replay")) Controller->Replay();
    else if (Action == TEXT("Stop") || Action == TEXT("Stop Immediate")) Controller->Stop(Action == TEXT("Stop Immediate"));
    else if (Action == TEXT("Previous")) Controller->Previous();
    else if (Action == TEXT("Next")) Controller->Next();
    else if (Action == TEXT("Loop")) Controller->SetLoop(!Controller->bLoop);
    else if (Action == TEXT("Auto")) Controller->SetAutoPreview(!Controller->bAutoPreview);
    else if (Action == TEXT("Reset")) { Controller->ResetParameters(); BuildParameters(); }
    else if (Action == TEXT("VR")) { Controller->SetVRPreview(!Controller->bVRPreview); bUpdating = true; QualityChoice->SetSelectedOption(EnumName(Controller->Quality)); bUpdating = false; }
    else if (Action == TEXT("SetA")) CompareA = Controller->SelectedTag;
    else if (Action == TEXT("SetB")) CompareB = Controller->SelectedTag;
    else if (Action == TEXT("Compare"))
    {
        if (!CompareA.IsValid() || !CompareB.IsValid()) { StatusText->SetText(FText::FromString(TEXT("Choose entries and use Set A / Set B before comparing."))); return; }
        Controller->SelectEntry(CompareA, false); Controller->PlayCompare(CompareB);
    }
    else if (Action == TEXT("Combat")) Controller->PlayCombatSimulation();
    else if (Action == TEXT("Refresh")) RefreshCatalog();
    else if (Action == TEXT("Favorite")) { Controller->ToggleFavorite(); RebuildCategories(); RebuildEntries(); RefreshSelection(); }
    else if (Action == TEXT("SaveReview")) SaveReview();
    else if (Action == TEXT("SaveNext")) { if (SaveReview()) Controller->Next(); }
    else if (Action == TEXT("FinalPass")) { FinalChoice->SetSelectedOption(TEXT("ProductionReady")); SaveReview(); }
    else if (Action == TEXT("FinalFail")) { FinalChoice->SetSelectedOption(TEXT("Fail")); SaveReview(); }
    CompareText->SetText(FText::FromString(FString::Printf(TEXT("Compare A: %s\nCompare B: %s"), CompareA.IsValid() ? *CompareA.ToString() : TEXT("—"), CompareB.IsValid() ? *CompareB.ToString() : TEXT("—"))));
    UpdateTelemetry();
}

void UVFXShowcaseWidget::UpdateTelemetry()
{
    if (!Controller || !TelemetryText) return;
    const FVFXShowcasePerformance Data = Controller->GetPerformance();
    TelemetryText->SetText(FText::FromString(FString::Printf(TEXT("%.1f FPS  ·  %.2f ms / frame\nActive VFX %d  ·  Managed %d  ·  Active Niagara %d\nQuality %s  ·  Test count %d\nCPU Niagara: N/A  |  GPU Niagara: N/A  |  Particles: N/A\n%s"),
        Data.FPS, Data.FrameTimeMs, Data.ActiveVFXCount, Data.ManagedVFXCount, Data.ActiveNiagaraSystems, *EnumName(Data.Quality), Data.CurrentTestCount, *Data.ProfilingNote)));
    StatusText->SetText(FText::FromString(Controller->LastMessage));
    LoopLabel->SetText(FText::FromString(Controller->bLoop ? TEXT("Loop: ON") : TEXT("Loop: OFF")));
    AutoLabel->SetText(FText::FromString(Controller->bAutoPreview ? TEXT("Auto Preview: ON") : TEXT("Auto Preview: OFF")));
    bUpdating = true;
    if (QualityChoice->GetSelectedOption() != EnumName(Controller->Quality)) QualityChoice->SetSelectedOption(EnumName(Controller->Quality));
    if (BackgroundChoice->GetSelectedOption() != EnumName(Controller->Background)) BackgroundChoice->SetSelectedOption(EnumName(Controller->Background));
    bUpdating = false;
}

void UVFXShowcaseWidget::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
    Super::NativeTick(Geometry, DeltaTime);
    if (!IsValid(Controller)) return;
    if (CachedTag != Controller->SelectedTag) { RefreshSelection(); RebuildEntries(); }
    const double Now = FPlatformTime::Seconds();
    if (Now - LastTelemetryTime >= .25) { LastTelemetryTime = Now; UpdateTelemetry(); }
}
