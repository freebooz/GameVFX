#include "VFXShowcaseTypes.h"
#include "VFXShowcaseProfile.h"
#include "VFXShowcaseLocalization.h"
bool FVFXReviewEvidence::HasApproval(const FString& Version) const
{
    return Result == EVFXGateResult::Approved && !Evidence.TrimStartAndEnd().IsEmpty() && !Reviewer.TrimStartAndEnd().IsEmpty()
        && !TimestampUtc.IsEmpty() && !Version.IsEmpty() && AssetVersion == Version;
}
bool FVFXShowcaseReview::IsProductionReady() const
{
    for (uint8 Index = 0; Index < 6; ++Index)
    {
        const FVFXReviewEvidence* Item = Gates.FindByPredicate([Index](const FVFXReviewEvidence& Gate) { return uint8(Gate.Gate) == Index; });
        if (!Item || !Item->HasApproval(AssetVersion)) return false;
    }
    return true;
}
bool FVFXShowcaseEntry::Matches(const FVFXShowcaseFilter& Filter, const TArray<FGameplayTag>& Recent) const
{
    if (Filter.Category != EVFXCategory::Unspecified && Category != Filter.Category) return false;
    if (Filter.Element != EVFXElement::None && Element != Filter.Element) return false;
    if (Filter.bFilterReviewStatus && Review.Status != Filter.ReviewStatus) return false;
    if (Filter.bFavoritesOnly && !Review.bFavorite) return false;
    if (Filter.bRecentOnly && !Recent.Contains(VFXTag)) return false;
    const FString Search = Filter.Search.TrimStartAndEnd();
    return Search.IsEmpty() || DisplayName.ToString().Contains(Search) || VFXTag.ToString().Contains(Search)
        || StaticEnum<EVFXElement>()->GetNameStringByValue(int64(Element)).Contains(Search)
        || StaticEnum<EVFXForm>()->GetNameStringByValue(int64(Form)).Contains(Search)
        || VFXShowcaseUI::Chinese(StaticEnum<EVFXElement>()->GetNameStringByValue(int64(Element))).Contains(Search)
        || VFXShowcaseUI::Chinese(StaticEnum<EVFXForm>()->GetNameStringByValue(int64(Form))).Contains(Search)
        || (Form == EVFXForm::Burst && FString(TEXT("爆发")).Contains(Search));
}
const FVFXShowcaseProfileEntry* UVFXShowcaseProfile::Find(FGameplayTag Tag) const
{
    return Entries.FindByPredicate([Tag](const FVFXShowcaseProfileEntry& Entry) { return Entry.VFXTag == Tag; });
}
