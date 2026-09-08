#pragma once

#include "CoreMinimal.h"
#include "VFXTaxonomy.generated.h"

/** Primary gameplay responsibility. Core building blocks are not catalog effects. */
UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    Unspecified,
    Cast,
    Projectile,
    Trail,
    Beam,
    Impact,
    Area,
    Status,
    Space,
    Lifecycle,
    Character,
    World,
    Surface,
    Destruction,
    Indicator,
    UI
};

UENUM(BlueprintType)
enum class EVFXElement : uint8
{
    None,
    Energy,
    Fire,
    Water,
    Ice,
    Lightning,
    Wind,
    Earth,
    Nature,
    Poison,
    Light,
    Dark,
    Arcane,
    Tech,
    Physical
};

UENUM(BlueprintType)
enum class EVFXForm : uint8
{
    Unspecified,
    Flash,
    Explosion,
    Shockwave,
    Ring,
    Orb,
    Beam,
    Slash,
    Trail,
    Column,
    Cloud,
    Burst,
    Wave,
    Field,
    Circle,
    Portal,
    Spark,
    Smoke,
    Debris
};

UENUM(BlueprintType)
enum class EVFXContext : uint8
{
    Combat,
    Character,
    Weapon,
    World,
    UI,
    Cinematic,
    Interaction
};

/** Descriptive lifecycle metadata; actual spawning remains controlled by spawn options. */
UENUM(BlueprintType)
enum class EVFXLifecycle : uint8
{
    OneShot UMETA(DisplayName = "One Shot"),
    Loop,
    Persistent,
    Attached
};
