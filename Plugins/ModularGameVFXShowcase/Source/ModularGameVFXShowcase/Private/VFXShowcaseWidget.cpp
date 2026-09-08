#include "VFXShowcaseWidget.h"
#include "VFXShowcaseController.h"
#include "VFXShowcaseLocalization.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/ButtonSlot.h"
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
    const FLinearColor PanelColor(0.008f, 0.016f, 0.028f, 0.08f);
    const FLinearColor Accent(0.32f, 0.78f, 0.94f, 1.f);
    const FLinearColor TextColor(0.88f, 0.95f, 1.f, 1.f);
    using VFXShowcaseUI::Chinese;
    template<typename T> FString EnumName(T Value) { return StaticEnum<T>()->GetNameStringByValue(static_cast<int64>(Value)); }
    template<typename T> FString EnumLabel(T Value) { return Chinese(EnumName(Value)); }
    template<typename T> T EnumValue(const FString& Value) { return static_cast<T>(StaticEnum<T>()->GetValueByNameString(Value)); }
    FSlateFontInfo HudFont(FSlateFontInfo Font)
    {
        Font.Size = VFXShowcaseUI::FontSize;
        Font.OutlineSettings.OutlineSize = 1;
        Font.OutlineSettings.OutlineColor = FLinearColor(0, 0, 0, .8f);
        return Font;
    }
    FSlateBrush HudBrush(float Opacity = .24f, float OutlineOpacity = .45f)
    {
        return FSlateRoundedBoxBrush(FLinearColor(.008f, .025f, .044f, Opacity), 3.f,
            FLinearColor(.25f, .68f, .82f, OutlineOpacity), .7f);
    }
    FButtonStyle HudButtonStyle(FButtonStyle Style)
    {
        Style.Normal = HudBrush(); Style.Hovered = HudBrush(.42f, .85f);
        Style.Pressed = HudBrush(.56f, 1.f); Style.Disabled = HudBrush(.12f, .2f);
        Style.NormalForeground = TextColor; Style.HoveredForeground = TextColor;
        Style.PressedForeground = Accent; Style.DisabledForeground = FLinearColor(.55f, .65f, .7f);
        return Style;
    }
    FEditableTextBoxStyle HudInputStyle(FEditableTextBoxStyle Style)
    {
        Style.TextStyle.Font = HudFont(Style.TextStyle.Font);
        // Slate dims the hint fill independently of its font outline. An opaque dark outline
        // overwhelms that dimmed fill, so editable text uses clean, explicit pale lettering.
        Style.TextStyle.Font.OutlineSettings.OutlineSize = 0;
        Style.TextStyle.Font.OutlineSettings.OutlineColor = FLinearColor::Transparent;
        Style.TextStyle.ColorAndOpacity = FLinearColor(.9f, .94f, .98f, 1.f);
        Style.ForegroundColor = TextColor; Style.FocusedForegroundColor = TextColor;
        Style.ReadOnlyForegroundColor = TextColor;
        Style.BackgroundImageNormal = HudBrush(.3f);
        Style.BackgroundImageHovered = HudBrush(.42f, .8f);
        Style.BackgroundImageFocused = HudBrush(.45f, 1.f);
        Style.BackgroundImageReadOnly = HudBrush(.16f, .2f);
        Style.BackgroundColor = FLinearColor::White;
        Style.Padding = FMargin(7.f, 5.f);
        return Style;
    }
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

UTextBlock* UVFXShowcaseWidget::Label(UVerticalBox* Box, const FString& Text)
{
    UTextBlock* Result = WidgetTree->ConstructWidget<UTextBlock>();
    Result->SetText(FText::FromString(Chinese(Text)));
    Result->SetFont(HudFont(Result->GetFont()));
    Result->SetColorAndOpacity(TextColor);
    Result->SetAutoWrapText(true);
    Result->SetWrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping);
    if (Inspector && Box == Inspector->GetContentBox())
    {
        // Resolve long asset paths before layout; auto-wrap alone learns its width one frame late.
        Result->SetAutoWrapText(false);
        Result->SetWrapTextAt(300.f);
    }
    Box->AddChildToVerticalBox(Result)->SetPadding(FMargin(0, 3));
    return Result;
}

UButton* UVFXShowcaseWidget::ActionButton(UPanelWidget* Parent, const FString& Text, const FString& Action)
{
    UVFXShowcaseActionButton* Button = WidgetTree->ConstructWidget<UVFXShowcaseActionButton>();
    Button->Configure(this, Action);
    Button->SetStyle(HudButtonStyle(Button->GetStyle()));
    Button->SetBackgroundColor(FLinearColor::White);
    UTextBlock* Caption = WidgetTree->ConstructWidget<UTextBlock>();
    Caption->SetText(FText::FromString(Chinese(Text)));
    Caption->SetFont(HudFont(Caption->GetFont()));
    Caption->SetColorAndOpacity(TextColor);
    // A control caption must contribute its complete horizontal width to its WrapBox.
    // Only whole buttons wrap to the next row; Chinese captions never collapse to one glyph.
    Caption->SetAutoWrapText(false);
    Caption->SetMargin(FMargin(10.f, 6.f));
    if (Action.StartsWith(TEXT("Entry:")) || Action.StartsWith(TEXT("Category:")))
    {
        Caption->SetAutoWrapText(false);
        Caption->SetWrapTextAt(248.f);
        Caption->SetWrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping);
        Caption->SetMargin(FMargin(5.f, 5.f));
    }
    Button->SetContent(Caption);
    if (Action.StartsWith(TEXT("Entry:")) || Action.StartsWith(TEXT("Category:")))
    {
        UButtonSlot* ContentSlot = CastChecked<UButtonSlot>(Caption->Slot);
        ContentSlot->SetHorizontalAlignment(HAlign_Fill);
        ContentSlot->SetVerticalAlignment(VAlign_Fill);
    }
    if (UWrapBox* Wrap = Cast<UWrapBox>(Parent)) Wrap->AddChildToWrapBox(Button)->SetPadding(FMargin(3));
    else if (UVerticalBox* Vertical = Cast<UVerticalBox>(Parent)) Vertical->AddChildToVerticalBox(Button)->SetPadding(FMargin(0, 3));
    else Parent->AddChild(Button);
    return Button;
}

UComboBoxString* UVFXShowcaseWidget::Choice(UVerticalBox* Box, const FString& Title, const TArray<FString>& Options)
{
    Label(Box, Title);
    UComboBoxString* Combo = WidgetTree->ConstructWidget<UComboBoxString>();
    // UE 5.8 exposes these as construction-only properties with getters and no runtime setters.
PRAGMA_DISABLE_DEPRECATION_WARNINGS
    Combo->Font = HudFont(Combo->GetFont());
    Combo->ForegroundColor = TextColor;
PRAGMA_ENABLE_DEPRECATION_WARNINGS
    // UE invokes this delegate for the selected value and every popup item; stored options remain stable English keys.
    Combo->OnGenerateWidgetEvent.BindDynamic(this, &UVFXShowcaseWidget::GenerateChoiceWidget);
    FComboBoxStyle ComboStyle = Combo->GetWidgetStyle();
    ComboStyle.ComboButtonStyle.ButtonStyle = HudButtonStyle(ComboStyle.ComboButtonStyle.ButtonStyle);
    ComboStyle.ComboButtonStyle.DownArrowImage.TintColor = Accent;
    ComboStyle.ComboButtonStyle.MenuBorderBrush = HudBrush(.78f, .65f);
    ComboStyle.ContentPadding = FMargin(6.f, 4.f);
    ComboStyle.MenuRowPadding = FMargin(5.f, 4.f);
    Combo->SetWidgetStyle(ComboStyle);
    FTableRowStyle Rows = Combo->GetItemStyle();
    Rows.EvenRowBackgroundBrush = HudBrush(0.f, 0.f);
    Rows.OddRowBackgroundBrush = Rows.EvenRowBackgroundBrush;
    Rows.EvenRowBackgroundHoveredBrush = HudBrush(.38f, .7f);
    Rows.OddRowBackgroundHoveredBrush = Rows.EvenRowBackgroundHoveredBrush;
    Rows.ActiveBrush = HudBrush(.46f, .85f); Rows.ActiveHoveredBrush = HudBrush(.55f, 1.f);
    Rows.InactiveBrush = HudBrush(.24f, .5f); Rows.InactiveHoveredBrush = Rows.ActiveBrush;
    Rows.TextColor = TextColor; Rows.SelectedTextColor = TextColor;
    Combo->SetItemStyle(Rows);
    for (const FString& Option : Options) Combo->AddOption(Option);
    if (Options.Num()) Combo->SetSelectedOption(Options[0]);
    Box->AddChildToVerticalBox(Combo)->SetPadding(FMargin(0, 2));
    return Combo;
}

UWidget* UVFXShowcaseWidget::GenerateChoiceWidget(FString Option)
{
    UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>();
    Text->SetText(FText::FromString(Chinese(Option)));
    Text->SetFont(HudFont(Text->GetFont()));
    Text->SetColorAndOpacity(TextColor);
    Text->SetWrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping);
    Text->SetWrapTextAt(245.f);
    return Text;
}

void UVFXShowcaseWidget::Number(UVerticalBox* Box, const FString& Name, const FString& Key, float Value, float Min, float Max)
{
    UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
    UTextBlock* Caption = WidgetTree->ConstructWidget<UTextBlock>();
    Caption->SetText(FText::FromString(Chinese(Name)));
    Caption->SetFont(HudFont(Caption->GetFont()));
    Caption->SetColorAndOpacity(TextColor);
    Row->AddChildToHorizontalBox(Caption)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    UVFXShowcaseNumber* Spin = WidgetTree->ConstructWidget<UVFXShowcaseNumber>();
    Spin->SetFont(HudFont(Spin->GetFont()));
    FSpinBoxStyle SpinStyle = Spin->GetWidgetStyle();
    SpinStyle.BackgroundBrush = HudBrush(.3f); SpinStyle.ActiveBackgroundBrush = HudBrush(.45f, 1.f);
    SpinStyle.HoveredBackgroundBrush = HudBrush(.4f, .8f);
    SpinStyle.InactiveFillBrush = HudBrush(.08f, 0.f); SpinStyle.ActiveFillBrush = HudBrush(.25f, 0.f);
    SpinStyle.HoveredFillBrush = HudBrush(.16f, 0.f); SpinStyle.ArrowsImage.TintColor = Accent;
    SpinStyle.ForegroundColor = TextColor;
    Spin->SetWidgetStyle(SpinStyle); Spin->SetForegroundColor(TextColor);
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
    Label(Header, TEXT("VFX SHOWCASE  /  Catalog & acceptance workbench"))->SetColorAndOpacity(Accent);
    StatusText = Label(Header, TEXT("Choose a catalog entry to play through VFX Manager."));
    Root->AddChildToVerticalBox(Frame(WidgetTree, Header));

    UHorizontalBox* Body = WidgetTree->ConstructWidget<UHorizontalBox>();
    Root->AddChildToVerticalBox(Body)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    UVerticalBox* Browser = WidgetTree->ConstructWidget<UVerticalBox>();
    USizeBox* BrowserSize = WidgetTree->ConstructWidget<USizeBox>();
    BrowserSize->SetWidthOverride(310); BrowserSize->SetContent(Frame(WidgetTree, Browser));
    Body->AddChildToHorizontalBox(BrowserSize);
    Search = WidgetTree->ConstructWidget<UEditableTextBox>();
    Search->SetHintText(FText::FromString(TEXT("搜索名称／标签")));
    Search->SetWidgetStyle(HudInputStyle(Search->GetWidgetStyle()));
    Search->SetForegroundColor(TextColor);
    Search->OnTextChanged.AddDynamic(this, &UVFXShowcaseWidget::SearchChanged);
    Browser->AddChildToVerticalBox(Search);

    Categories = CreateWidget<UVFXShowcaseCategoryMenu>(this);
    UScrollBox* CategoryScroll = WidgetTree->ConstructWidget<UScrollBox>();
    CategoryScroll->SetOrientation(Orient_Vertical);
    CategoryScroll->AddChild(Categories);
    USizeBox* CategorySize = WidgetTree->ConstructWidget<USizeBox>();
    CategorySize->SetHeightOverride(172); CategorySize->SetContent(CategoryScroll);
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
    Label(Performance->GetContentBox(), TEXT("LIVE PERFORMANCE"))->SetColorAndOpacity(Accent);
    TelemetryText = Label(Performance->GetContentBox(), TEXT("Waiting for controller"));
    Stage->AddChildToVerticalBox(Frame(WidgetTree, Performance));
    USpacer* Space = WidgetTree->ConstructWidget<USpacer>();
    Space->SetVisibility(ESlateVisibility::HitTestInvisible);
    Stage->AddChildToVerticalBox(Space)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    UVerticalBox* StageFooter = WidgetTree->ConstructWidget<UVerticalBox>();
    SelectionText = Label(StageFooter, TEXT("No VFX selected"));
    CompareText = Label(StageFooter, TEXT("Compare A: —  |  B: —"));
    Label(StageFooter, TEXT("Select an entry for Single mode. Stress uses its own test zone.\nUse the Camera distance controls to inspect near and far readability."));
    Stage->AddChildToVerticalBox(Frame(WidgetTree, StageFooter));

    UVerticalBox* Details = WidgetTree->ConstructWidget<UVerticalBox>();
    UScrollBox* DetailScroll = WidgetTree->ConstructWidget<UScrollBox>();
    DetailScroll->AddChild(Details);
    USizeBox* DetailSize = WidgetTree->ConstructWidget<USizeBox>();
    DetailSize->SetWidthOverride(350); DetailSize->SetContent(Frame(WidgetTree, DetailScroll));
    Body->AddChildToHorizontalBox(DetailSize);
    Inspector = CreateWidget<UVFXShowcaseInspector>(this); Details->AddChildToVerticalBox(Inspector);
    Parameters = CreateWidget<UVFXShowcaseParameterPanel>(this); Details->AddChildToVerticalBox(Parameters);
    Label(Details, TEXT("ENVIRONMENT"))->SetColorAndOpacity(Accent);
    BackgroundChoice = Choice(Details, TEXT("Background"), {TEXT("Neutral"), TEXT("Bright"), TEXT("Dark"), TEXT("Complex")});
    QualityChoice = Choice(Details, TEXT("Quality"), {TEXT("Cinematic"), TEXT("High"), TEXT("Medium"), TEXT("Low"), TEXT("VRMobile")});
    QualityChoice->SetSelectedOption(TEXT("High"));
    SpeedChoice = Choice(Details, TEXT("Playback speed"), {TEXT("1.0x"), TEXT("0.5x"), TEXT("0.25x"), TEXT("0.1x")});
    for (UComboBoxString* Combo : {BackgroundChoice.Get(), QualityChoice.Get(), SpeedChoice.Get()})
        Combo->OnSelectionChanged.AddDynamic(this, &UVFXShowcaseWidget::EnvironmentChanged);
    Label(Details, TEXT("Preview / beam distance"));
    UWrapBox* Distances = WidgetTree->ConstructWidget<UWrapBox>(); Details->AddChildToVerticalBox(Distances);
    for (int32 Distance : {3, 5, 10, 20, 30, 50}) ActionButton(Distances, FString::Printf(TEXT("%d米"), Distance), FString::Printf(TEXT("Distance:%d"), Distance));
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
    Label(ReviewBox, TEXT("MANUAL REVIEW"))->SetColorAndOpacity(Accent);
    ReviewState = Label(ReviewBox, TEXT("No review selected"));
    Label(ReviewBox, TEXT("Check: focus, impact, force, speed, depth, color, silhouette, timing, readability, end feedback and performance."));
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
    Reviewer->SetWidgetStyle(HudInputStyle(Reviewer->GetWidgetStyle()));
    Reviewer->SetForegroundColor(TextColor);
    Reviewer->SetHintText(FText::FromString(TEXT("评审人姓名（验收证据必填）")));
    ReviewBox->AddChildToVerticalBox(Reviewer);
    Notes->WidgetStyle = HudInputStyle(Notes->WidgetStyle);
    Notes->SetForegroundColor(TextColor);
    Notes->SetHintText(FText::FromString(TEXT("填写本次验收项的证据与观察记录")));
    USizeBox* NoteSize = WidgetTree->ConstructWidget<USizeBox>(); NoteSize->SetHeightOverride(100); NoteSize->SetContent(Notes);
    ReviewBox->AddChildToVerticalBox(NoteSize);
    UWrapBox* ReviewButtons = WidgetTree->ConstructWidget<UWrapBox>(); ReviewBox->AddChildToVerticalBox(ReviewButtons);
    ActionButton(ReviewButtons, TEXT("Save review"), TEXT("SaveReview"));
    ActionButton(ReviewButtons, TEXT("Save + Next"), TEXT("SaveNext"));
    ActionButton(ReviewButtons, TEXT("Final PASS"), TEXT("FinalPass"));
    ActionButton(ReviewButtons, TEXT("FAIL"), TEXT("FinalFail"));
    ActionButton(ReviewButtons, TEXT("Toggle Favorite"), TEXT("Favorite"));
    Label(ReviewBox, TEXT("Production Ready requires all six gates approved with reviewer + evidence for this asset version. Saves review data only."));

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
    for (int32 Count : {1, 5, 10, 20, 50, 100}) ActionButton(Stress, FString::Printf(TEXT("压力测试：%d个"), Count), FString::Printf(TEXT("Stress:%d"), Count));
    Root->AddChildToVerticalBox(Frame(WidgetTree, Controls));
    bUpdating = false;
}

void UVFXShowcaseWidget::RefreshCatalog()
{
    if (!Categories) return;
    if (!IsValid(Controller))
    {
        StatusText->SetText(FText::FromString(TEXT("当前关卡没有测试控制器，请放置 BP_VFX_ShowcaseController。")));
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
    for (const FVFXShowcaseCategoryProgress& Progress : Controller->GetCategories())
    {
        // The controller's Unspecified row is the aggregate, not a second category.
        const FString Name = Progress.Category == EVFXCategory::Unspecified ? TEXT("All") : EnumName(Progress.Category);
        const FString State = Progress.Failed > 0 ? TEXT("存在未通过项") : (Progress.Total > 0 && Progress.Ready == Progress.Total ? TEXT("可用于生产") : TEXT("待验收"));
        UButton* Button = ActionButton(Box, FString::Printf(TEXT("%s · %d项\n已验收%d／%d · %s"), *Chinese(Name), Progress.Total, Progress.Reviewed, Progress.Total, *State), TEXT("Category:") + Name);
        if (SelectedCategory == Name) Button->SetBackgroundColor(Accent);
    }
    for (const FString& Name : {TEXT("Favorites"), TEXT("Recent"), TEXT("Failed"), TEXT("Unreviewed")})
    {
        UButton* Button = ActionButton(Box, Name, TEXT("Category:") + Name);
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
            *EnumLabel(Entry.Element), *EnumLabel(Entry.Form), *EnumLabel(Entry.Priority), *EnumLabel(Entry.Review.Status));
        UButton* Button = ActionButton(Item->GetContentBox(), Caption, TEXT("Entry:") + Entry.VFXTag.ToString());
        if (Entry.VFXTag == Controller->SelectedTag) Button->SetBackgroundColor(FLinearColor(.12f, .43f, .57f));
        Box->AddChildToVerticalBox(Item)->SetPadding(FMargin(0, 2));
    }
    EntryCount->SetText(FText::FromString(FString::Printf(TEXT("共%d项特效 · %s"), VisibleTags.Num(), *Chinese(SelectedCategory))));
    if (VisibleTags.IsEmpty()) Label(Box, TEXT("没有匹配的特效，请清除筛选条件或登记有效的目录条目。"));
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
    Label(Box, TEXT("INSPECTOR"))->SetColorAndOpacity(Accent);
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
    Label(Box, Entry.DisplayName.ToString());
    Label(Box, Entry.VFXTag.ToString());
    Label(Box, FString::Printf(TEXT("分类：%s\n元素：%s　形态：%s\n场景：%s　生命周期：%s\n优先级：%s　对象池：%s\n生成方式：%s　预览方式：%s\n验收状态：%s"),
        *EnumLabel(Entry.Category), *EnumLabel(Entry.Element), *EnumLabel(Entry.Form), *EnumLabel(Entry.Context), *EnumLabel(Entry.Lifecycle), *EnumLabel(Entry.Priority),
        *Chinese(Entry.Pooling), *EnumLabel(Entry.SpawnMode), *EnumLabel(Entry.PreviewMode), *EnumLabel(Entry.Review.Status)));
    Label(Box, FString::Printf(TEXT("系统资源路径：%s\n效果类型：%s\n发射器数量：%d\n模拟目标：%s\n固定边界：%s"), *Entry.AssetPath, *Chinese(Entry.EffectType), Entry.EmitterCount, *Chinese(Entry.SimulationTarget), *Entry.FixedBounds));
    for (const FString& Issue : Entry.ValidationIssues) Label(Box, TEXT("校验：") + Issue)->SetColorAndOpacity(FLinearColor(1.f, .55f, .25f));
    BuildParameters();
    const FVFXShowcaseReview Record = Controller->GetReview(CachedTag);
    for (const FVFXReviewEvidence& Gate : Record.Gates)
        if (Gate.Result != EVFXGateResult::NotAssessed)
            Label(Box, FString::Printf(TEXT("%s：%s%s\n资源版本：%s\n评审人：%s · %s\n验收证据：%s"), *EnumLabel(Gate.Gate), *EnumLabel(Gate.Result), Gate.AssetVersion == Entry.AssetVersion ? TEXT("") : TEXT("〔历史版本，已过期，只读〕"), *Gate.AssetVersion, *Gate.Reviewer, *Gate.TimestampUtc, *Gate.Evidence));
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
    ReviewState->SetText(FText::FromString(FString::Printf(TEXT("%s · %s\n资源版本：%s"), *EnumLabel(Record.Status), Record.bFavorite ? TEXT("已收藏") : TEXT("未收藏"), *Entry.AssetVersion)));
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
    Label(Box, TEXT("PARAMETERS"))->SetColorAndOpacity(Accent);
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
        Label(Box, Chinese(ColorName) + TEXT(" · 线性颜色与透明度"));
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
        Controller->LastMessage = TEXT("所选特效或资源版本已变化，验收表单已刷新。请重新评估并明确选择要提交的验收结果。");
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
        if (!CompareA.IsValid() || !CompareB.IsValid()) { StatusText->SetText(FText::FromString(TEXT("请先选择特效并分别设为对比甲、对比乙，再开始并排对比。"))); return; }
        Controller->SelectEntry(CompareA, false); Controller->PlayCompare(CompareB);
    }
    else if (Action == TEXT("Combat")) Controller->PlayCombatSimulation();
    else if (Action == TEXT("Refresh")) RefreshCatalog();
    else if (Action == TEXT("Favorite")) { Controller->ToggleFavorite(); RebuildCategories(); RebuildEntries(); RefreshSelection(); }
    else if (Action == TEXT("SaveReview")) SaveReview();
    else if (Action == TEXT("SaveNext")) { if (SaveReview()) Controller->Next(); }
    else if (Action == TEXT("FinalPass")) { FinalChoice->SetSelectedOption(TEXT("ProductionReady")); SaveReview(); }
    else if (Action == TEXT("FinalFail")) { FinalChoice->SetSelectedOption(TEXT("Fail")); SaveReview(); }
    CompareText->SetText(FText::FromString(FString::Printf(TEXT("对比甲：%s\n对比乙：%s"), CompareA.IsValid() ? *CompareA.ToString() : TEXT("未选择"), CompareB.IsValid() ? *CompareB.ToString() : TEXT("未选择"))));
    UpdateTelemetry();
}

void UVFXShowcaseWidget::UpdateTelemetry()
{
    if (!Controller || !TelemetryText) return;
    const FVFXShowcasePerformance Data = Controller->GetPerformance();
    TelemetryText->SetText(FText::FromString(FString::Printf(TEXT("每秒%.1f帧 · 每帧%.2f毫秒\n活动特效%d个 · 受管特效%d个 · 活动粒子系统%d个\n质量：%s · 测试实例%d个\n处理器耗时：暂无 · 图形处理器耗时：暂无 · 粒子数：暂无\n%s"),
        Data.FPS, Data.FrameTimeMs, Data.ActiveVFXCount, Data.ManagedVFXCount, Data.ActiveNiagaraSystems, *EnumLabel(Data.Quality), Data.CurrentTestCount, *Data.ProfilingNote)));
    StatusText->SetText(FText::FromString(Controller->LastMessage));
    LoopLabel->SetText(FText::FromString(Controller->bLoop ? TEXT("循环：开启") : TEXT("循环：关闭")));
    AutoLabel->SetText(FText::FromString(Controller->bAutoPreview ? TEXT("自动预览：开启") : TEXT("自动预览：关闭")));
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
