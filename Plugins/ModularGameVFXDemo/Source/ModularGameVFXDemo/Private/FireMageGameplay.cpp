#include "FireMageGameplay.h"
#include "FrostMageGameplay.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Components/CapsuleComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"

namespace FireMage {
template<class T> T* Asset(const TCHAR* Path){ConstructorHelpers::FObjectFinder<T> F(Path);return F.Object;}
FVector Feet(const ACharacter* C){return C->GetActorLocation()-FVector(0,0,C->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());}
FVector Ground(UWorld* World,const FVector& Point,const AActor* Ignore){
 FHitResult Hit;FCollisionQueryParams Query(SCENE_QUERY_STAT(FireGround),false,Ignore);
 if(World->LineTraceSingleByObjectType(Hit,Point+FVector(0,0,250),Point-FVector(0,0,1500),FCollisionObjectQueryParams(ECC_WorldStatic),Query))return Hit.ImpactPoint+FVector(0,0,4);
 return Point;
}
}

AFMFireProjectile::AFMFireProjectile(){
 PrimaryActorTick.bCanEverTick=true;RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
 Flames=CreateDefaultSubobject<UNiagaraComponent>(TEXT("Fireball"));Flames->SetupAttachment(RootComponent);
 Flames->SetAsset(FireMage::Asset<UNiagaraSystem>(TEXT("/ModularGameVFX/ModularGameVFXLibrary/02_Projectile/NS_Fire_Fireball_Projectile")));Flames->SetCastShadow(false);
 Impact=FireMage::Asset<UNiagaraSystem>(TEXT("/ModularGameVFX/ModularGameVFXLibrary/05_Impact/NS_Fire_Impact_Burst"));
 FireLight=CreateDefaultSubobject<UPointLightComponent>(TEXT("FireBounce"));FireLight->SetupAttachment(RootComponent);
 FireLight->SetLightColor(FLinearColor(1,.23f,.025f));FireLight->SetIntensity(2800);FireLight->SetAttenuationRadius(280);FireLight->SetCastShadows(false);FireLight->SetSourceRadius(35);
}
void AFMFireProjectile::BeginPlay(){Super::BeginPlay();SetLifeSpan(4.4f);}
void AFMFireProjectile::Explode(const FVector& Location,AFMFrostMonster* Victim){
 if(bHit)return;bHit=true;
 if(IsValid(Victim)&&Victim->Health>0)Victim->ReceiveFire(Damage);
 UNiagaraFunctionLibrary::SpawnSystemAtLocation(this,Impact,Location,FRotator::ZeroRotator,FVector(1),true,true,ENCPoolMethod::AutoRelease);
 UE_LOG(LogTemp,Display,TEXT("PY_FIREBALL_HIT Damage=%.1f Victim=%s"),Victim?Damage:0.f,Victim?*Victim->GetName():TEXT("World"));Destroy();
}
void AFMFireProjectile::Tick(float D){
 Super::Tick(D);if(bHit)return;
 if(!IsValid(Target)||Target->Health<=0){Destroy();return;}
 const FVector Start=GetActorLocation(),Aim=Target->GetActorLocation()+FVector(0,0,22);
 const FVector Dir=(Aim-Start).GetSafeNormal();SetActorRotation(Dir.Rotation());
 const float Travel=FMath::Max(0.f,Speed)*D;const FVector Next=Start+Dir*FMath::Min(Travel,(Aim-Start).Size());
 FHitResult Hit;FCollisionQueryParams Query(SCENE_QUERY_STAT(FireballSweep),false,this);Query.AddIgnoredActor(GetOwner());
 if(GetWorld()->SweepSingleByChannel(Hit,Start,Next,FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(14),Query)){
  Explode(Hit.ImpactPoint,Cast<AFMFrostMonster>(Hit.GetActor()));return;
 }
 if(FVector::DistSquared(Next,Aim)<FMath::Square(50.f)){Explode(Next,Target);return;}
 SetActorLocation(Next);FireLight->SetIntensity(2600+350*FMath::Sin(GetGameTimeSinceCreation()*19));
}

AFMFireArea::AFMFireArea(){
 PrimaryActorTick.bCanEverTick=true;PrimaryActorTick.TickInterval=.05f;RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
 Flames=CreateDefaultSubobject<UNiagaraComponent>(TEXT("Flamestrike"));Flames->SetupAttachment(RootComponent);Flames->SetAsset(FireMage::Asset<UNiagaraSystem>(TEXT("/ModularGameVFX/ModularGameVFXLibrary/06_Area/NS_Fire_Flamestrike_Area")));Flames->SetCastShadow(false);
 FireLight=CreateDefaultSubobject<UPointLightComponent>(TEXT("FireBounce"));FireLight->SetupAttachment(RootComponent);FireLight->SetRelativeLocation(FVector(0,0,95));
 FireLight->SetLightColor(FLinearColor(1,.20f,.015f));FireLight->SetIntensity(6500);FireLight->SetAttenuationRadius(530);FireLight->SetSourceRadius(170);FireLight->SetCastShadows(false);
}
void AFMFireArea::DamageOccupants(float Damage){
 // Damage checks only at ignition and 2 Hz; independent of Niagara scalability.
 int32 Hits=0;
 for(TActorIterator<AFMFrostMonster> I(GetWorld());I;++I){
  if(I->Health<=0||FVector::DistSquaredXY(GetActorLocation(),I->GetActorLocation())>Radius*Radius||FMath::Abs(FireMage::Feet(*I).Z-GetActorLocation().Z)>180)continue;
  FHitResult Hit;FCollisionQueryParams Query(SCENE_QUERY_STAT(FireAreaSight),false,this);
  if(GetWorld()->LineTraceSingleByObjectType(Hit,GetActorLocation()+FVector(0,0,80),I->GetActorLocation(),FCollisionObjectQueryParams(ECC_WorldStatic),Query))continue;
  I->ReceiveFire(Damage);Hits++;
 }
 UE_LOG(LogTemp,Display,TEXT("PY_AREA_DAMAGE Hits=%d Damage=%.1f"),Hits,Damage);
}
void AFMFireArea::BeginPlay(){Super::BeginPlay();SetLifeSpan(6.4f);DamageOccupants(28);}
void AFMFireArea::Tick(float D){
 Super::Tick(D);Age+=D;
 const int32 Due=FMath::Min(8,FMath::FloorToInt(Age/.5f));
 if(Due>BurnTicks){DamageOccupants(4.f*(Due-BurnTicks));BurnTicks=Due;}
 FireLight->SetIntensity((5800+600*FMath::Sin(Age*13))*FMath::Clamp((5.6f-Age)/2.f,0.f,1.f));
}

bool AFMFrostMage::ValidateFireTarget(){
 if(!IsValid(Target)||Target->Health<=0)SelectNextTarget();
 if(!IsValid(Target)){Message(TEXT("Select a living target"));return false;}
 if(GetDistanceTo(Target)>2400){Message(TEXT("Target is out of range (24m)"));return false;}
 if(!Controller||!Controller->LineOfSightTo(Target)){Message(TEXT("Target is out of sight"));return false;}return true;
}
void AFMFrostMage::CastFireball(){
 if(Health<=0||CastingSpell||FireballCooldown>0||!ValidateFireTarget())return;
 FireCastTarget=Target;GetCharacterMovement()->StopMovementImmediately();SetActorRotation((Target->GetActorLocation()-GetActorLocation()).GetSafeNormal2D().Rotation());
 CastingSpell=5;CastDuration=CastRemaining=1.8f;Recovery=0;SpellLight->SetLightColor(FLinearColor(1,.23f,.02f));PlayPose(BoltAnimation,false);
 Charge=UNiagaraFunctionLibrary::SpawnSystemAttached(FireChargeSystem,RootComponent,NAME_None,FVector(0,0,-96),FRotator::ZeroRotator,EAttachLocation::KeepRelativeOffset,true,true,ENCPoolMethod::AutoRelease);
 UE_LOG(LogTemp,Display,TEXT("PY_FIREBALL_CAST_BEGIN"));
}
void AFMFrostMage::CastFireBlast(){
 // Off the casting lock: burst can be woven into a fireball or ground cast.
 if(Health<=0||FireBlastCooldown>0||!ValidateFireTarget())return;
 Target->ReceiveFire(22);
 UNiagaraFunctionLibrary::SpawnSystemAtLocation(this,FireImpactSystem,Target->GetActorLocation()+FVector(0,0,10),FRotator::ZeroRotator,FVector(.8f),true,true,ENCPoolMethod::AutoRelease);
 if(!CastingSpell&&GetVelocity().Size2D()<15){PlayPose(NovaAnimation,false);Recovery=.55f;}
 FireBlastCooldown=8;SpellLight->SetLightColor(FLinearColor(1,.3f,.035f));LightLife=.3f;
 Message(TEXT("Fire Blast  /  Instant  /  22 damage"));UE_LOG(LogTemp,Display,TEXT("PY_FIREBLAST_INSTANT"));
}
void AFMFrostMage::ClearFirePreview(){if(IsValid(GroundPreview))GroundPreview->DeactivateImmediate();GroundPreview=nullptr;}
void AFMFrostMage::CastFlamestrike(){
 if(Health<=0||CastingSpell||FlamestrikeCooldown>0)return;
 FVector Point;
 if(IsValid(Target)&&Target->Health>0){if(!ValidateFireTarget())return;Point=FireMage::Feet(Target);}
 else{
  FHitResult Hit;auto* PC=Cast<APlayerController>(Controller);
  if(PC&&PC->GetHitResultUnderCursor(ECC_Visibility,false,Hit))Point=Hit.ImpactPoint;
  else Point=FireMage::Feet(this)+GetActorForwardVector()*650;
  if(FVector::DistSquaredXY(Point,GetActorLocation())>FMath::Square(2400.f)){Message(TEXT("Ground target is out of range (24m)"));return;}
 }
 FireGroundPoint=FireMage::Ground(GetWorld(),Point,this);GetCharacterMovement()->StopMovementImmediately();
 CastingSpell=7;CastDuration=CastRemaining=2.f;Recovery=0;SpellLight->SetLightColor(FLinearColor(1,.18f,.01f));PlayPose(NovaAnimation,false);
 GroundPreview=UNiagaraFunctionLibrary::SpawnSystemAtLocation(this,FireTelegraphSystem,FireGroundPoint,FRotator::ZeroRotator,FVector(1),true,true,ENCPoolMethod::AutoRelease);
 Charge=UNiagaraFunctionLibrary::SpawnSystemAttached(FireChargeSystem,RootComponent,NAME_None,FVector(0,0,-96),FRotator::ZeroRotator,EAttachLocation::KeepRelativeOffset,true,true,ENCPoolMethod::AutoRelease);
 UE_LOG(LogTemp,Display,TEXT("PY_FLAMESTRIKE_CAST_BEGIN"));
}
void AFMFrostMage::ReleaseFireSpell(int32 Spell){
 ClearFirePreview();
 if(Spell==5){
  if(!IsValid(FireCastTarget)||FireCastTarget->Health<=0||GetDistanceTo(FireCastTarget)>2400||!Controller||!Controller->LineOfSightTo(FireCastTarget)){Message(TEXT("Fireball target lost or obstructed"));Recovery=.2f;FireCastTarget=nullptr;return;}
  const FVector Start=FireMage::Feet(this)+GetActorForwardVector()*66+FVector(0,0,134);
  FActorSpawnParameters Params;Params.Owner=this;Params.Instigator=this;
  auto* Bolt=GetWorld()->SpawnActor<AFMFireProjectile>(AFMFireProjectile::StaticClass(),Start,(FireCastTarget->GetActorLocation()-Start).Rotation(),Params);
  if(Bolt)Bolt->Target=FireCastTarget;FireCastTarget=nullptr;FireballCooldown=.5f;Recovery=.7f;LightLife=.3f;
  Message(TEXT("Fireball  /  30 damage"));UE_LOG(LogTemp,Display,TEXT("PY_FIREBALL_RELEASE"));
 }else if(Spell==7){
  FActorSpawnParameters Params;Params.Owner=this;Params.Instigator=this;
  GetWorld()->SpawnActor<AFMFireArea>(AFMFireArea::StaticClass(),FireGroundPoint,FRotator::ZeroRotator,Params);
  FlamestrikeCooldown=10;Recovery=.65f;LightLife=.5f;
  Message(TEXT("Flamestrike  /  3m radius  /  28 + burn 32"));UE_LOG(LogTemp,Display,TEXT("PY_FLAMESTRIKE_RELEASE"));
 }
}
void AFMPlayerController::FMTestFireball(){if(auto* P=Cast<AFMFrostMage>(GetPawn()))P->CastFireball();}
void AFMPlayerController::FMTestFireBlast(){if(auto* P=Cast<AFMFrostMage>(GetPawn()))P->CastFireBlast();}
void AFMPlayerController::FMTestFlamestrike(){if(auto* P=Cast<AFMFrostMage>(GetPawn()))P->CastFlamestrike();}
