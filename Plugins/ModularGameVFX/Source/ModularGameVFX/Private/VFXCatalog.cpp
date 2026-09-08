#include "VFXCatalog.h"

#include "Misc/DataValidation.h"

void UVFXCatalog::BuildRuntimeCache() const
{
    EntryIndexByTag.Reset();

    for (int32 Index = 0; Index < Entries.Num(); ++Index)
    {
        const FVFXCatalogEntry& Entry = Entries[Index];
        if (!Entry.VFXTag.IsValid())
        {
            UE_LOG(LogModularGameVFX, Warning, TEXT("[ModularGameVFX] Catalog '%s' contains an invalid VFX Tag at index %d."), *GetName(), Index);
            continue;
        }

        if (EntryIndexByTag.Contains(Entry.VFXTag))
        {
            UE_LOG(LogModularGameVFX, Warning, TEXT("[ModularGameVFX] Duplicate VFX Tag '%s' in Catalog '%s'; the first entry remains active."), *Entry.VFXTag.ToString(), *GetName());
            continue;
        }

        EntryIndexByTag.Add(Entry.VFXTag, Index);
    }
}

const FVFXCatalogEntry* UVFXCatalog::FindEntryNative(const FGameplayTag& VFXTag) const
{
    if (EntryIndexByTag.Num() == 0 && Entries.Num() > 0)
    {
        BuildRuntimeCache();
    }

    const int32* Index = EntryIndexByTag.Find(VFXTag);
    return Index && Entries.IsValidIndex(*Index) ? &Entries[*Index] : nullptr;
}

bool UVFXCatalog::FindEntry(FGameplayTag VFXTag, FVFXCatalogEntry& OutEntry) const
{
    if (const FVFXCatalogEntry* Entry = FindEntryNative(VFXTag))
    {
        OutEntry = *Entry;
        return true;
    }

    return false;
}

void UVFXCatalog::PostLoad()
{
    Super::PostLoad();
    BuildRuntimeCache();
}

#if WITH_EDITOR
EDataValidationResult UVFXCatalog::IsDataValid(FDataValidationContext& Context) const
{
    const EDataValidationResult SuperResult = Super::IsDataValid(Context);
    bool bHasErrors = SuperResult == EDataValidationResult::Invalid;
    TSet<FGameplayTag> SeenTags;

    for (int32 Index = 0; Index < Entries.Num(); ++Index)
    {
        const FVFXCatalogEntry& Entry = Entries[Index];
        if (!Entry.VFXTag.IsValid())
        {
            Context.AddError(FText::Format(NSLOCTEXT("ModularGameVFX", "InvalidTag", "Entry {0} has an invalid Gameplay Tag."), FText::AsNumber(Index)));
            bHasErrors = true;
        }
        else if (SeenTags.Contains(Entry.VFXTag))
        {
            Context.AddError(FText::Format(NSLOCTEXT("ModularGameVFX", "DuplicateTag", "Duplicate VFX Tag: {0}"), FText::FromString(Entry.VFXTag.ToString())));
            bHasErrors = true;
        }
        else
        {
            SeenTags.Add(Entry.VFXTag);
        }

        // Legacy catalogs can acquire metadata incrementally. Typed entries use
        // category-first tags; project character namespaces may encode skill ownership.
        if (Entry.VFXTag.IsValid() && Entry.Category != EVFXCategory::Unspecified)
        {
            const FString TagString = Entry.VFXTag.ToString();
            const UEnum* CategoryEnum = StaticEnum<EVFXCategory>();
            const int64 CategoryValue = static_cast<int64>(Entry.Category);
            const bool bKnownCategory = CategoryEnum->IsValidEnumValue(CategoryValue)
                && CategoryValue < static_cast<int64>(EVFXCategory::UI) + 1;
            const FString CategoryName = CategoryEnum->GetNameStringByValue(CategoryValue);
            const FString ExpectedPrefix = FString(TEXT("VFX.")) + CategoryName + TEXT(".");
            const bool bCategoryTag = TagString.StartsWith(ExpectedPrefix, ESearchCase::IgnoreCase)
                && TagString.Len() > ExpectedPrefix.Len();
            const bool bProjectCharacterTag = TagString.StartsWith(TEXT("VFX.Character."), ESearchCase::IgnoreCase)
                && TagString.Len() > FCString::Strlen(TEXT("VFX.Character."));

            if (!bKnownCategory || (!bCategoryTag && !bProjectCharacterTag))
            {
                Context.AddError(FText::Format(
                    NSLOCTEXT("ModularGameVFX", "CategoryTagMismatch", "Entry {0}: Gameplay Tag '{1}' does not match category '{2}'. Use VFX.<Category>.<Name> or a project VFX.Character.* namespace."),
                    FText::AsNumber(Index), FText::FromString(TagString), FText::FromString(CategoryName)));
                bHasErrors = true;
            }
        }

        if (Entry.NiagaraSystem.IsNull())
        {
            Context.AddError(FText::Format(NSLOCTEXT("ModularGameVFX", "MissingNiagara", "Entry {0} ({1}) has no Niagara System."), FText::AsNumber(Index), FText::FromString(Entry.VFXTag.ToString())));
            bHasErrors = true;
        }
    }

    return bHasErrors ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}

void UVFXCatalog::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    BuildRuntimeCache();
}
#endif
