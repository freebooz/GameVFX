#include "ModularGameVFXSettings.h"

UModularGameVFXSettings::UModularGameVFXSettings()
{
    CategoryName = TEXT("Plugins");
    SectionName = TEXT("ModularGameVFX");
    DefaultCatalog = TSoftObjectPtr<UVFXCatalog>(FSoftObjectPath(TEXT("/ModularGameVFX/ModularGameVFXLibrary/00_Core/Catalog/DA_VFXCatalog_Default.DA_VFXCatalog_Default")));
}

FName UModularGameVFXSettings::GetCategoryName() const
{
    return TEXT("Plugins");
}

FName UModularGameVFXSettings::GetSectionName() const
{
    return TEXT("ModularGameVFX");
}

FText UModularGameVFXSettings::GetSectionText() const
{
    return NSLOCTEXT("ModularGameVFX", "SettingsSection", "Modular Game VFX");
}
