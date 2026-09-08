#include "ArcaneMageGameplay.h"
#include "FrostMageGameplay.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Components/CapsuleComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

AFMArcaneMissile::AFMArcaneMissile(){
 PrimaryActorTick.bCanEverTick=true;RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
 Energy=CreateDefaultSubobject<UNiagaraComponent>(TEXT("ArcaneMissile"));Energy->SetupAttachment(RootComponent);Energy->SetCastShadow(false);
 ConstructorHelpers::FObjectFinder<UNiagaraSystem> Head(TEXT("/ModularGameVFX/ModularGameVFXLibrary/02_Projectile/NS_Arcane_Missiles_Projectile"));Energy->SetAsset(Head.Object);
 ConstructorHelpers::FObjectFinder<UNiagaraSystem> Hit(TEXT("/ModularGameVFX/ModularGameVFXLibrary/05_Impact/NS_Arcane_Missiles_Impact"));Impact=Hit.Object;
 // Missiles are emissive only; the character's shared spell light supplies bounce.
}
void AFMArcaneMissile::BeginPlay(){
 Super::BeginPlay();Start=LastPosition=GetActorLocation();
 const float Side=VolleyIndex%2==0?-1.f:1.f;
 ControlA=Start+GetActorForwardVector()*125+GetActorRightVector()*Side*(135+VolleyIndex*16)+FVector(0,0,110+VolleyIndex*13);
 FlightDuration=FMath::Clamp(IsValid(Target)?GetDistanceTo(Target)/1450.f:.7f,.5f,1.65f);SetLifeSpan(FlightDuration+.65f);
 UE_LOG(LogTemp,Display,TEXT("AM_MISSILE_SPAWN Index=%d"),VolleyIndex);
}
void AFMArcaneMissile::ImpactAt(const FVector& Location,AFMFrostMonster* Victim){
 if(bImpacted)return;bImpacted=true;
 if(IsValid(Victim)&&Victim->Health>0)Victim->ReceiveArcane(Damage);
 UNiagaraFunctionLibrary::SpawnSystemAtLocation(this,Impact,Location,FRotator::ZeroRotator,FVector(1),true,true,ENCPoolMethod::AutoRelease);
 Energy->SetVariableFloat(TEXT("User.HeadAlpha"),0);Energy->Deactivate();SetLifeSpan(.5f);
 UE_LOG(LogTemp,Display,TEXT("AM_MISSILE_HIT Index=%d Damage=%.1f"),VolleyIndex,Victim?Damage:0.f);
}
void AFMArcaneMissile::Tick(float D){
 Super::Tick(D);if(bImpacted)return;
 if(!IsValid(Target)||Target->Health<=0){Energy->DeactivateImmediate();Destroy();return;}
 Age+=D;const float T=FMath::Clamp(Age/FlightDuration,0.f,1.f),Inv=1-T;
 const FVector End=Target->GetActorLocation()+FVector(0,0,20);
 const FVector Axis=(End-Start).GetSafeNormal();const FVector Side=FVector::CrossProduct(Axis,FVector::UpVector).GetSafeNormal();
 const FVector ControlB=End-Axis*110+Side*(VolleyIndex%2?45:-45)+FVector(0,0,45);
 const FVector Next=Inv*Inv*Inv*Start+3*Inv*Inv*T*ControlA+3*Inv*T*T*ControlB+T*T*T*End;
 FHitResult Hit;FCollisionQueryParams Query(SCENE_QUERY_STAT(ArcaneSweep),false,this);Query.AddIgnoredActor(GetOwner());
 if(GetWorld()->SweepSingleByChannel(Hit,LastPosition,Next,FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(9),Query)){
  SetActorLocation(Hit.Location);ImpactAt(Hit.ImpactPoint,Cast<AFMFrostMonster>(Hit.GetActor()));return;
 }
 SetActorLocationAndRotation(Next,(Next-LastPosition).Rotation());LastPosition=Next;
 if(T>=1)ImpactAt(Next,Target);
}
void AFMFrostMage::CastArcaneMissiles(){
 if(Health<=0||CastingSpell||ArcaneCooldown>0||!ValidateFireTarget())return;
 ArcaneTarget=Target;ArcaneShotsFired=0;ArcaneCooldown=4;
 GetCharacterMovement()->StopMovementImmediately();SetActorRotation((Target->GetActorLocation()-GetActorLocation()).GetSafeNormal2D().Rotation());
 CastingSpell=8;CastDuration=CastRemaining=2.5f;Recovery=0;SpellLight->SetLightColor(FLinearColor(.42f,.12f,1));PlayPose(BoltAnimation,true);
 Charge=UNiagaraFunctionLibrary::SpawnSystemAttached(ArcaneChannelSystem,RootComponent,NAME_None,FVector(0,0,-96),FRotator::ZeroRotator,EAttachLocation::KeepRelativeOffset,true,true,ENCPoolMethod::AutoRelease);
 TickArcaneChannel(0);UE_LOG(LogTemp,Display,TEXT("AM_CHANNEL_BEGIN"));
}
void AFMFrostMage::TickArcaneChannel(float D){
 if(!IsValid(ArcaneTarget)||ArcaneTarget->Health<=0||GetDistanceTo(ArcaneTarget)>2400||!Controller||!Controller->LineOfSightTo(ArcaneTarget)){CancelCast();Message(TEXT("Arcane channel interrupted: target lost"));return;}
 const int32 Due=FMath::Min(5,FMath::FloorToInt((2.5f-CastRemaining)/.5f)+1);
 while(ArcaneShotsFired<Due){
  const FVector Feet=GetActorLocation()-FVector(0,0,GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
  const FVector Origin=Feet+GetActorForwardVector()*60+GetActorRightVector()*(ArcaneShotsFired%2?18:-18)+FVector(0,0,136);
  const FTransform Transform((ArcaneTarget->GetActorLocation()-Origin).Rotation(),Origin);
  auto* Missile=GetWorld()->SpawnActorDeferred<AFMArcaneMissile>(AFMArcaneMissile::StaticClass(),Transform,this,this,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
  if(Missile){Missile->Target=ArcaneTarget;Missile->VolleyIndex=ArcaneShotsFired;UGameplayStatics::FinishSpawningActor(Missile,Transform);}
  ArcaneShotsFired++;
 }
}
void AFMPlayerController::FMTestArcane(){if(auto* P=Cast<AFMFrostMage>(GetPawn()))P->CastArcaneMissiles();}
