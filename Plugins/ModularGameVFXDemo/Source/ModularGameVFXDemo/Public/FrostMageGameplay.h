#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "FrostMageGameplay.generated.h"
class UNiagaraComponent; class UNiagaraSystem; class USpringArmComponent; class UCameraComponent; class UPointLightComponent; class UAnimSequence; class UMaterialInterface;
class AFMFrostMage;

UCLASS()
class MODULARGAMEVFXDEMO_API AFMFrostMonster : public ACharacter {
 GENERATED_BODY()
public:
 AFMFrostMonster(); virtual void BeginPlay() override; virtual void Tick(float Delta) override;
 UFUNCTION(BlueprintCallable) void ReceiveFrost(float Damage, bool bRoot);
 UFUNCTION(BlueprintCallable) void ReceiveFire(float Damage);
 UFUNCTION(BlueprintCallable) void ReceiveArcane(float Damage);
 UFUNCTION(BlueprintCallable) void ReceiveWeaponDamage(float Damage);
 UPROPERTY(BlueprintReadOnly) float Health=100;
 UPROPERTY(BlueprintReadOnly) float FrozenRemaining=0;
 UPROPERTY(BlueprintReadOnly) float SlowRemaining=0;
 UPROPERTY(BlueprintReadOnly) FVector WanderDestination;
 UPROPERTY(BlueprintReadOnly) bool bSelected=false;
 UPROPERTY(EditAnywhere) float WanderRadius=420;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Combat") bool bCombatEnabled=true;
 UPROPERTY(BlueprintReadOnly,Category="Combat") bool bChasing=false;
 UPROPERTY(BlueprintReadOnly,Category="Combat") float CastRemaining=0;
 UPROPERTY(BlueprintReadOnly,Category="Combat") float AttackCooldown=1.5f;
private:
 FVector Home; float ChooseAt=0,WalkPhase=0,RespawnAt=0;
 UPROPERTY() TArray<UStaticMeshComponent*> Stones;
 UPROPERTY() TArray<FVector> StoneOffsets;
 UPROPERTY() UMaterialInterface* StoneMaterial;
 UPROPERTY() UMaterialInterface* FrozenMaterial;
 UPROPERTY() USceneComponent* BodyRoot;
 UPROPERTY() UPointLightComponent* EyesGlow;
 UPROPERTY() UNiagaraSystem* ChargeSystem;
 UPROPERTY() UNiagaraComponent* AttackCharge;
 void ChooseDestination();
 bool ApplySpellDamage(float Damage);
 void StopAttack(); void UpdateCombat(float Delta);
};

UCLASS()
class MODULARGAMEVFXDEMO_API AFMFrostProjectile : public AActor {
 GENERATED_BODY()
public:
 AFMFrostProjectile(); virtual void BeginPlay() override; virtual void Tick(float Delta) override;
 UPROPERTY(BlueprintReadWrite) TObjectPtr<AFMFrostMonster> Target;
 UPROPERTY(BlueprintReadWrite) TObjectPtr<AFMFrostMage> PlayerTarget;
 UPROPERTY(BlueprintReadWrite) float Speed=1250;
 UPROPERTY(BlueprintReadWrite) float Damage=24.f;
private:
 UPROPERTY() UNiagaraComponent* Frost;
 UPROPERTY() UPointLightComponent* Glow;
 UPROPERTY() UNiagaraSystem* Impact;
 float Alive=0; bool bHit=false;
};

UCLASS()
class MODULARGAMEVFXDEMO_API AFMPetalSpell : public AActor {
 GENERATED_BODY()
public:
 AFMPetalSpell(); virtual void BeginPlay() override; virtual void Tick(float Delta) override;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Petal Bloom") float Duration=6.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Petal Bloom") float Intensity=1.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Petal Bloom|Palette") FLinearColor GlowColor=FLinearColor(.025f,1.f,.60f,1.f);
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Petal Bloom|Palette") FLinearColor PetalColor=FLinearColor(.02f,.65f,.28f,1.f);
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Petal Bloom|Palette") FLinearColor CoreColor=FLinearColor(.70f,1.f,.88f,1.f);
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Petal Bloom") UNiagaraComponent* Petals;
 UPROPERTY(BlueprintReadWrite,Category="Petal Bloom") TObjectPtr<AFMFrostMage> FollowTarget;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Petal Bloom") bool bHealing=false;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Petal Bloom",meta=(ClampMin="0")) float HealingPerSecond=12.f;
private:
 UPROPERTY() UPointLightComponent* BloomLight;
 float Age=0;
 void UpdateFollowLocation();
};

UCLASS()
class MODULARGAMEVFXDEMO_API AFMFrostMage : public ACharacter {
 GENERATED_BODY()
public:
 AFMFrostMage(); virtual void BeginPlay() override; virtual void Tick(float Delta) override;
 UFUNCTION(BlueprintCallable) void CastFrostbolt();
 UFUNCTION(BlueprintCallable) void CastFrostNova();
 UFUNCTION(BlueprintCallable) void CastPetalBloom();
 UFUNCTION(BlueprintCallable) void CastHealingBloom();
 UFUNCTION(BlueprintCallable) void CastFireball();
 UFUNCTION(BlueprintCallable) void CastFireBlast();
 UFUNCTION(BlueprintCallable) void CastFlamestrike();
 UFUNCTION(BlueprintCallable) void CastArcaneMissiles();
 UFUNCTION(BlueprintCallable) float ApplyHealing(float Amount);
 UFUNCTION(BlueprintCallable) void ReceiveEnemyFrost(float Damage);
 UFUNCTION(BlueprintCallable) void SelectNextTarget();
 UFUNCTION(BlueprintCallable) void SelectPreviousTarget();
 UFUNCTION(BlueprintCallable) void CancelCast();
 UFUNCTION(BlueprintCallable) void SetMovementIntent(float Forward, float Right);
 UPROPERTY(BlueprintReadOnly) TObjectPtr<AFMFrostMonster> Target;
 UPROPERTY(BlueprintReadOnly) float CastRemaining=0;
 UPROPERTY(BlueprintReadOnly) float CastDuration=1.5;
 UPROPERTY(BlueprintReadOnly) float NovaCooldown=0;
 UPROPERTY(BlueprintReadOnly) float BoltCooldown=0;
 UPROPERTY(BlueprintReadOnly) float PetalCooldown=0;
 UPROPERTY(BlueprintReadOnly) float HealingCooldown=0;
 UPROPERTY(BlueprintReadOnly,Category="Fire Mage") float FireballCooldown=0;
 UPROPERTY(BlueprintReadOnly,Category="Fire Mage") float FireBlastCooldown=0;
 UPROPERTY(BlueprintReadOnly,Category="Fire Mage") float FlamestrikeCooldown=0;
 UPROPERTY(BlueprintReadOnly,Category="Arcane Mage") float ArcaneCooldown=0;
 UPROPERTY(BlueprintReadOnly,Category="Arcane Mage") int32 ArcaneShotsFired=0;
 UPROPERTY(BlueprintReadWrite,Category="Combat") bool bAutoAttacking=false;
 UPROPERTY(BlueprintReadOnly) float HealingRemaining=0;
 UPROPERTY(BlueprintReadOnly) float ChilledRemaining=0;
 UPROPERTY(BlueprintReadOnly) float RespawnRemaining=0;
 UPROPERTY(BlueprintReadOnly,Category="Vitals") float Health=100.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Vitals",meta=(ClampMin="1")) float MaxHealth=100.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Petal Bloom|Palette") FLinearColor PetalGlowColor=FLinearColor(1.f,.18f,.48f,1.f);
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Petal Bloom|Palette") FLinearColor PetalSurfaceColor=FLinearColor(1.f,.055f,.20f,1.f);
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Petal Bloom|Palette") FLinearColor PetalCoreColor=FLinearColor(1.f,.74f,.89f,1.f);
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Petal Bloom",meta=(ClampMin="0.1",ClampMax="3")) float PetalIntensity=1.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Healing Bloom|Palette") FLinearColor HealingGlowColor=FLinearColor(.025f,1.f,.60f,1.f);
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Healing Bloom|Palette") FLinearColor HealingPetalColor=FLinearColor(.02f,.65f,.28f,1.f);
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Healing Bloom|Palette") FLinearColor HealingCoreColor=FLinearColor(.70f,1.f,.88f,1.f);
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Healing Bloom",meta=(ClampMin="0.1",ClampMax="3")) float HealingIntensity=1.f;
 UPROPERTY(BlueprintReadOnly) int32 CastingSpell=0;
 UPROPERTY(BlueprintReadWrite,Category="Movement") bool bWalkMode=false;
 UPROPERTY(BlueprintReadOnly,Category="Movement") bool bBackpedaling=false;
 UPROPERTY(BlueprintReadOnly,Category="Movement") UAnimSequence* CurrentLocomotion;
 UPROPERTY(BlueprintReadOnly) FString StatusMessage;
 UPROPERTY(BlueprintReadOnly) float StatusRemaining=0;
 UPROPERTY(VisibleAnywhere) USpringArmComponent* CameraBoom;
 UPROPERTY(VisibleAnywhere) UCameraComponent* FollowCamera;
private:
 UPROPERTY() UNiagaraSystem* ChargeSystem;
 UPROPERTY() UNiagaraSystem* NovaSystem;
 UPROPERTY() UNiagaraComponent* Charge;
 UPROPERTY() UNiagaraSystem* FireChargeSystem;
 UPROPERTY() UNiagaraSystem* FireImpactSystem;
 UPROPERTY() UNiagaraSystem* FireTelegraphSystem;
 UPROPERTY() UNiagaraComponent* GroundPreview;
 UPROPERTY() TObjectPtr<AFMFrostMonster> FireCastTarget;
 FVector FireGroundPoint;
 UPROPERTY() UNiagaraSystem* ArcaneChannelSystem;
 UPROPERTY() TObjectPtr<AFMFrostMonster> ArcaneTarget;
 UPROPERTY() UAnimSequence* BoltAnimation;
 UPROPERTY() UAnimSequence* NovaAnimation;
 UPROPERTY() UAnimSequence* IdleAnimation;
 UPROPERTY() UAnimSequence* WalkAnimation;
 UPROPERTY() TArray<UAnimSequence*> JogDirections;
 UPROPERTY() TArray<UAnimSequence*> WalkDirections;
 UPROPERTY() UPointLightComponent* SpellLight;
 float Recovery=0,LightLife=0; bool bWalking=false;
 float WeaponSwingRemaining=0;
 FVector SpawnLocation;
 void ReleaseSpell(); void PlayPose(UAnimSequence* Anim,bool Loop); void Message(const FString& Text);
 void UpdateLocomotion(); void CycleTarget(int Direction);
 bool ValidateFireTarget(); void ReleaseFireSpell(int32 Spell); void ClearFirePreview();
 void TickArcaneChannel(float Delta);
};

UCLASS()
class MODULARGAMEVFXDEMO_API AFMPlayerController : public APlayerController {
 GENERATED_BODY()
public:
 AFMPlayerController(); virtual void BeginPlay() override; virtual void SetupInputComponent() override; virtual void PlayerTick(float Delta) override;
 virtual void FlushPressedKeys() override;
 UFUNCTION(Exec) void FMTestBolt(); UFUNCTION(Exec) void FMTestNova(); UFUNCTION(Exec) void FMTestSelect();
 UFUNCTION(Exec) void FMTestPetal();
 UFUNCTION(Exec) void FMTestHeal();
 UFUNCTION(Exec) void FMTestFireball();
 UFUNCTION(Exec) void FMTestFireBlast();
 UFUNCTION(Exec) void FMTestFlamestrike();
 UFUNCTION(Exec) void FMTestArcane();
 UFUNCTION(BlueprintCallable,Category="FrostMage|Verification") void SimulateKey(FName KeyName,bool bPressed);
 UFUNCTION(BlueprintCallable,Category="FrostMage|Verification") void SimulateMouse(float X,float Y);
 UFUNCTION(BlueprintCallable,Category="FrostMage|Targeting") void SelectUnderCursor();
 UPROPERTY(BlueprintReadOnly,Category="FrostMage|Camera") bool bOrbiting=false;
 UPROPERTY(BlueprintReadOnly,Category="FrostMage|Camera") bool bLeftMouseHeld=false;
 UPROPERTY(BlueprintReadOnly,Category="FrostMage|Camera") bool bRightMouseHeld=false;
 UPROPERTY(BlueprintReadOnly,Category="FrostMage|Movement") bool bAutoRun=false;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="WoW Mouse") float CameraYawMoveSpeed=180.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="WoW Mouse") float CameraPitchMoveSpeed=90.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="WoW Mouse") float MouseSensitivityMultiplier=1.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="WoW Mouse") float RawDegreesPerCount=.10f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="WoW Mouse") bool bInvertMousePitch=false;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="WoW Mouse") bool bSmartCameraFollow=true;
 UPROPERTY(BlueprintReadOnly,Category="WoW Mouse") float ZoomGoal=690.f;
private:
 void JumpStart(); void JumpStop(); void ZoomIn(); void ZoomOut(); void Cancel();
 void BeginOrbit(); void EndOrbit(); void SetCursorMode();
 void BeginLeftMouse(); void EndLeftMouse(); void RestoreCursor();
 void ToggleAutoRun(); void ToggleWalk();
 float MouseRestoreX=0,MouseRestoreY=0;
 float LeftMouseTravel=0;
 float RightMouseTravel=0,CameraFollowDelay=0;
 bool bRightMouseDragged=false;
 bool bLeftMouseDragged=false,bSkipFirstOrbitDelta=false;
};

UCLASS()
class MODULARGAMEVFXDEMO_API AFMFrostHUD : public AHUD {
 GENERATED_BODY()
public: virtual void DrawHUD() override;
};

UCLASS()
class MODULARGAMEVFXDEMO_API AFMFrostGameMode : public AGameModeBase {
 GENERATED_BODY()
public: AFMFrostGameMode(); virtual void BeginPlay() override;
};
