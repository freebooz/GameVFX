#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFXDemoMigrationDefaultsTest,
    "ModularGameVFXDemo.Migration.NativeDefaultReferences",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FVFXDemoMigrationDefaultsTest::RunTest(const FString&)
{
    struct FFixture { const TCHAR* Class; const TCHAR* Property; bool bComponent; const TCHAR* Suffix; };
    const FFixture Fixtures[] = {
        {TEXT("FMFrostMage"),TEXT("ChargeSystem"),false,TEXT("01_Cast/NS_Ice_Frostbolt_Charge")},
        {TEXT("FMFrostMage"),TEXT("NovaSystem"),false,TEXT("06_Area/NS_Ice_FrostNova_Burst")},
        {TEXT("FMFrostMage"),TEXT("FireChargeSystem"),false,TEXT("01_Cast/NS_Fire_Fireball_Charge")},
        {TEXT("FMFrostMage"),TEXT("FireImpactSystem"),false,TEXT("05_Impact/NS_Fire_Impact_Burst")},
        {TEXT("FMFrostMage"),TEXT("FireTelegraphSystem"),false,TEXT("14_Indicator/NS_Fire_Flamestrike_Warning")},
        {TEXT("FMFrostMage"),TEXT("ArcaneChannelSystem"),false,TEXT("01_Cast/NS_Arcane_Missiles_Channel")},
        {TEXT("FMFrostMonster"),TEXT("ChargeSystem"),false,TEXT("01_Cast/NS_Ice_Frostbolt_Charge")},
        {TEXT("FMFrostProjectile"),TEXT("Impact"),false,TEXT("05_Impact/NS_Ice_Frostbolt_Impact")},
        {TEXT("FMFrostProjectile"),TEXT("Frost"),true,TEXT("02_Projectile/NS_Ice_Frostbolt_Projectile")},
        {TEXT("FMFireProjectile"),TEXT("Impact"),false,TEXT("05_Impact/NS_Fire_Impact_Burst")},
        {TEXT("FMFireProjectile"),TEXT("Flames"),true,TEXT("02_Projectile/NS_Fire_Fireball_Projectile")},
        {TEXT("FMFireArea"),TEXT("Flames"),true,TEXT("06_Area/NS_Fire_Flamestrike_Area")},
        {TEXT("FMArcaneMissile"),TEXT("Impact"),false,TEXT("05_Impact/NS_Arcane_Missiles_Impact")},
        {TEXT("FMArcaneMissile"),TEXT("Energy"),true,TEXT("02_Projectile/NS_Arcane_Missiles_Projectile")},
        {TEXT("FMPetalSpell"),TEXT("Petals"),true,TEXT("07_Status/NS_Nature_PetalBloom_Aura")}
    };
    FString Records; bool bPassed=true; int32 Checked=0;
    for (const FFixture& Fixture: Fixtures)
    {
        const FString ClassPath=FString(TEXT("/Script/ModularGameVFXDemo."))+Fixture.Class;
        UClass* Class=LoadObject<UClass>(nullptr,*ClassPath);
        const FObjectPropertyBase* Property=Class?FindFProperty<FObjectPropertyBase>(Class,Fixture.Property):nullptr;
        UObject* Value=Property?Property->GetObjectPropertyValue_InContainer(Class->GetDefaultObject()):nullptr;
        if (Fixture.bComponent)
        {
            const UNiagaraComponent* Component=Cast<UNiagaraComponent>(Value);
            Value=Component?Component->GetAsset():nullptr;
        }
        const FString Expected=FString(TEXT("/ModularGameVFX/ModularGameVFXLibrary/"))+Fixture.Suffix;
        const FString Actual=Value?Value->GetOutermost()->GetName():TEXT("");
        const bool bMatch=TestEqual(ClassPath+TEXT(".")+Fixture.Property,Actual,Expected);
        bPassed=bMatch&&bPassed;
        if(Checked++)Records+=TEXT(",");
        // Fixture and UObject path names contain no JSON control characters.
        Records+=FString::Printf(TEXT("{\"class_path\":\"%s\",\"property\":\"%s\",\"actual\":\"%s\",\"expected\":\"%s\"}"),*ClassPath,Fixture.Property,*Actual,*Expected);
    }
    const FString Output=FString::Printf(TEXT("{\"passed\":%s,\"checked_count\":%d,\"timestamp_utc\":\"%s\",\"checked\":[%s]}"),bPassed?TEXT("true"):TEXT("false"),Checked,*FDateTime::UtcNow().ToIso8601(),*Records);
    const FString Directory=FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("TaxonomyMigration"));
    IFileManager::Get().MakeDirectory(*Directory,true);
    TestTrue(TEXT("Native default verification evidence saved"),FFileHelper::SaveStringToFile(Output,*FPaths::Combine(Directory,TEXT("demo_defaults.json")),FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM));
    return bPassed;
}
#endif
