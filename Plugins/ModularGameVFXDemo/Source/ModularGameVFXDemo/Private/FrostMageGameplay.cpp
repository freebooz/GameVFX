#include "FrostMageGameplay.h"
#include "AIController.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Animation/AnimSequence.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "InputCoreTypes.h"
#include "InputKeyEventArgs.h"
#include "UObject/ConstructorHelpers.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "CanvasItem.h"
#include "Fonts/SlateFontInfo.h"

namespace FM {
template<class T> T* Asset(const TCHAR* Path) { ConstructorHelpers::FObjectFinder<T> F(Path); return F.Object; }
FVector Feet(const ACharacter* C) { return C->GetActorLocation()-FVector(0,0,C->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()); }
int32 ActionAtCursor(APlayerController* PC){
 if(PC->ActorHasTag(TEXT("VFXShowcaseUIInteractive")))return INDEX_NONE;
 int32 W=0,H=0;float MX=0,MY=0;PC->GetViewportSize(W,H);if(!PC->GetMousePosition(MX,MY))return INDEX_NONE;
 const int Cols=W<1280?4:8,Rows=Cols==4?2:1;const float CardW=FMath::Min(170.f,(W-64-(Cols-1)*8.f)/Cols),BarW=Cols*CardW+(Cols-1)*8;
 const float X=(W-BarW)/2,Y=H-102-(Rows-1)*72;
 for(int32 I=0;I<8;I++){const float SX=X+(I%Cols)*(CardW+8),SY=Y+(I/Cols)*72;if(MX>=SX&&MX<=SX+CardW&&MY>=SY&&MY<=SY+62)return I;}
 return INDEX_NONE;
}
FString ChineseStatus(FString Text){
 const TPair<const TCHAR*,const TCHAR*> Translations[]={
  {TEXT("1-4 Frost / Bloom  |  5 Fireball  |  6 Fire Blast  |  7 Flamestrike  |  8 Arcane"),TEXT("1—4 冰霜与花瓣　5 火球　6 火焰冲击　7 烈焰风暴　8 奥术飞弹")},
  {TEXT("Select a living target with Tab"),TEXT("请按 Tab 选择存活目标")},{TEXT("Select a living target"),TEXT("请选择存活目标")},
  {TEXT("Ground target is out of range (24m)"),TEXT("地面目标超出24米范围")},{TEXT("Target is out of range (24m)"),TEXT("目标超出24米范围")},
  {TEXT("Target is out of sight"),TEXT("目标被遮挡")},{TEXT("Fireball target lost or obstructed"),TEXT("火球目标丢失或被遮挡")},
  {TEXT("Arcane channel interrupted: target lost"),TEXT("奥术引导中断：目标丢失")},{TEXT("Cast interrupted"),TEXT("施法已中断")},{TEXT("Target lost"),TEXT("目标丢失")},
  {TEXT("Frostbolt released"),TEXT("寒冰箭已释放")},{TEXT("Frost Nova: "),TEXT("冰霜新星：")},{TEXT(" target(s) frozen for 5s"),TEXT(" 个目标冻结5秒")},
  {TEXT("Enemy Frostbolt  -"),TEXT("敌方寒冰箭：生命减少 ")},{TEXT(" HP  /  Chilled"),TEXT("，受到冰冷减速")},
  {TEXT("Rose Petal Bloom  /  6 seconds"),TEXT("绯樱花瓣：持续6秒")},{TEXT("Healing Bloom  /  +12 HP per second  /  Move freely"),TEXT("翡翠治疗：每秒恢复12生命，可自由移动")},
  {TEXT("Auto attack  /  6 damage"),TEXT("自动攻击：6点伤害")},{TEXT("Recovered at the test spawn"),TEXT("已在出生点恢复")},
  {TEXT("Fire Blast  /  Instant  /  22 damage"),TEXT("火焰冲击：瞬发，22点伤害")},{TEXT("Fireball  /  30 damage"),TEXT("火球术：30点伤害")},
  {TEXT("Flamestrike  /  3m radius  /  28 + burn 32"),TEXT("烈焰风暴：半径3米，28点伤害与32点灼烧")},
  {TEXT("Arcane Missiles  /  5 bolts  /  45 damage"),TEXT("奥术飞弹：5枚飞弹，共45点伤害")}
 };
 for(const auto& Pair:Translations)Text.ReplaceInline(Pair.Key,Pair.Value,ESearchCase::CaseSensitive);
 return Text;
}
}

AFMFrostMonster::AFMFrostMonster() {
 ChargeSystem=FM::Asset<UNiagaraSystem>(TEXT("/ModularGameVFX/ModularGameVFXLibrary/01_Cast/NS_Ice_Frostbolt_Charge"));
 PrimaryActorTick.bCanEverTick=true; GetCapsuleComponent()->InitCapsuleSize(58,122);
 GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
 AIControllerClass=AAIController::StaticClass(); AutoPossessAI=EAutoPossessAI::PlacedInWorldOrSpawned;
 GetCharacterMovement()->MaxWalkSpeed=145; GetCharacterMovement()->bOrientRotationToMovement=true;
 GetCharacterMovement()->RotationRate=FRotator(0,200,0); bUseControllerRotationYaw=false;
 BodyRoot=CreateDefaultSubobject<USceneComponent>(TEXT("GolemBody"));BodyRoot->SetupAttachment(RootComponent);BodyRoot->SetRelativeLocation(FVector(0,0,-122));
 UStaticMesh* Rock=FM::Asset<UStaticMesh>(TEXT("/ModularGameVFXDemo/Environment/Meshes/SM_Demo_MoonstoneGolemBody"));
 StoneMaterial=FM::Asset<UMaterialInterface>(TEXT("/ModularGameVFXDemo/Environment/Materials/M_Demo_MoonstoneGolem"));
 FrozenMaterial=FM::Asset<UMaterialInterface>(TEXT("/ModularGameVFX/ModularGameVFXLibrary/00_Core/Materials/Frost/MI_Frost_NovaIce"));
 const FVector Pos[]={ {0,0,135},{0,0,181},{8,0,220},{0,-55,169},{0,55,169},{4,-73,115},{4,73,115},{8,-78,73},{8,78,73},{0,-28,70},{0,28,70},{9,-29,26},{9,29,26} };
 const FVector Scale[]={ {1.04f,.93f,1.05f},{.96f,1.18f,.64f},{.56f,.58f,.58f},{.67f,.60f,.68f},{.67f,.60f,.68f},{.42f,.42f,.80f},{.42f,.42f,.80f},{.57f,.52f,.52f},{.57f,.52f,.52f},{.43f,.46f,.89f},{.43f,.46f,.89f},{.62f,.49f,.57f},{.62f,.49f,.57f} };
 for(int j=0;j<UE_ARRAY_COUNT(Pos);j++) {
  auto* M=CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Stone_%02d"),j));M->SetupAttachment(BodyRoot);M->SetStaticMesh(Rock);M->SetMaterial(0,StoneMaterial);M->SetCollisionEnabled(ECollisionEnabled::NoCollision);M->SetRelativeLocation(Pos[j]);M->SetRelativeScale3D(Scale[j]);Stones.Add(M);StoneOffsets.Add(Pos[j]);
 }
 UStaticMesh* Sphere=FM::Asset<UStaticMesh>(TEXT("/Engine/BasicShapes/Sphere"));
 auto* EyeMat=FM::Asset<UMaterialInterface>(TEXT("/ModularGameVFXDemo/Environment/Materials/M_Demo_GolemEyes"));
 for(int j=0;j<2;j++){auto* E=CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Eye_%d"),j));E->SetupAttachment(BodyRoot);E->SetStaticMesh(Sphere);E->SetMaterial(0,EyeMat);E->SetCollisionEnabled(ECollisionEnabled::NoCollision);E->SetRelativeLocation(FVector(29,j?12:-12,226));E->SetRelativeScale3D(FVector(.065f,.10f,.07f));}
 EyesGlow=CreateDefaultSubobject<UPointLightComponent>(TEXT("EyeGlow"));EyesGlow->SetupAttachment(BodyRoot);EyesGlow->SetRelativeLocation(FVector(37,0,226));EyesGlow->SetIntensity(20);EyesGlow->SetLightColor(FLinearColor(.63f,.20f,1));EyesGlow->SetAttenuationRadius(120);EyesGlow->SetCastShadows(false);
}
void AFMFrostMonster::BeginPlay(){Super::BeginPlay();Home=GetActorLocation();ChooseDestination();}
void AFMFrostMonster::ChooseDestination(){const float A=FMath::FRandRange(0,2*PI);const float R=FMath::FRandRange(140,WanderRadius);WanderDestination=Home+FVector(FMath::Cos(A)*R,FMath::Sin(A)*R,0);ChooseAt=GetWorld()->GetTimeSeconds()+FMath::FRandRange(3.5f,6.0f);}
void AFMFrostMonster::ReceiveFrost(float Damage,bool bRoot){
 if(!ApplySpellDamage(Damage))return;SlowRemaining=5.f;
 if(bRoot&&Health>0){FrozenRemaining=5;StopAttack();if(auto* AI=Cast<AAIController>(GetController()))AI->StopMovement();ConsumeMovementInputVector();GetCharacterMovement()->StopMovementImmediately();GetCharacterMovement()->DisableMovement();for(auto* M:Stones)M->SetMaterial(0,FrozenMaterial);UE_LOG(LogTemp,Display,TEXT("FM_MONSTER_FROZEN Duration=5"));}
}
void AFMFrostMonster::ReceiveFire(float Damage){ApplySpellDamage(Damage);}
void AFMFrostMonster::ReceiveArcane(float Damage){ApplySpellDamage(Damage);}
void AFMFrostMonster::ReceiveWeaponDamage(float Damage){ApplySpellDamage(Damage);}
bool AFMFrostMonster::ApplySpellDamage(float Damage){
 if(Health<=0||!FMath::IsFinite(Damage)||Damage<=0)return false;Health=FMath::Max(0.f,Health-Damage);
 if(Health>0&&bCombatEnabled){bChasing=true;AttackCooldown=FMath::Min(AttackCooldown,.15f);UE_LOG(LogTemp,Display,TEXT("FM_MONSTER_RETALIATE"));}
 if(Health<=0){bChasing=false;StopAttack();SetActorHiddenInGame(true);SetActorEnableCollision(false);RespawnAt=GetWorld()->GetTimeSeconds()+2.5f;}
 return true;
}
void AFMFrostMonster::StopAttack(){CastRemaining=0;if(IsValid(AttackCharge))AttackCharge->Deactivate();AttackCharge=nullptr;}
void AFMFrostMonster::UpdateCombat(float D){
 auto* Player=Cast<AFMFrostMage>(UGameplayStatics::GetPlayerCharacter(this,0));
 bChasing=bCombatEnabled&&IsValid(Player)&&Player->Health>0&&GetDistanceTo(Player)<(bChasing?3600.f:2400.f);
 if(!bChasing){StopAttack();return;}
 AttackCooldown=FMath::Max(0.f,AttackCooldown-D);
 const FVector To=Player->GetActorLocation()-GetActorLocation();
 if(CastRemaining>0){
  GetCharacterMovement()->StopMovementImmediately();SetActorRotation(To.GetSafeNormal2D().Rotation());CastRemaining-=D;
  if(CastRemaining<=0){
   StopAttack();FActorSpawnParameters Params;Params.Owner=this;Params.Instigator=this;
   const FVector Start=FM::Feet(this)+GetActorForwardVector()*78+FVector(0,0,175);
   auto* Bolt=GetWorld()->SpawnActor<AFMFrostProjectile>(AFMFrostProjectile::StaticClass(),Start,(Player->GetActorLocation()-Start).Rotation(),Params);
   if(Bolt){Bolt->PlayerTarget=Player;Bolt->Speed=800;Bolt->Damage=14;}
   AttackCooldown=3.f;UE_LOG(LogTemp,Display,TEXT("FM_ENEMY_BOLT_RELEASE"));
  }
 }else if(To.Size2D()<1250&&AttackCooldown<=0&&GetController()&&GetController()->LineOfSightTo(Player)){
  CastRemaining=1.1f;GetCharacterMovement()->StopMovementImmediately();SetActorRotation(To.GetSafeNormal2D().Rotation());
  AttackCharge=UNiagaraFunctionLibrary::SpawnSystemAttached(ChargeSystem,RootComponent,NAME_None,FVector(0,0,-122),FRotator::ZeroRotator,EAttachLocation::KeepRelativeOffset,true,true,ENCPoolMethod::AutoRelease);
  UE_LOG(LogTemp,Display,TEXT("FM_ENEMY_BOLT_CAST_BEGIN"));
 }else if(To.Size2D()>650){
  GetCharacterMovement()->MaxWalkSpeed=SlowRemaining>0?85:220;AddMovementInput(To.GetSafeNormal2D(),1);
 }
}
void AFMFrostMonster::Tick(float D){Super::Tick(D);const float T=GetWorld()->GetTimeSeconds();
 if(Health<=0){if(T>=RespawnAt){Health=100;FrozenRemaining=SlowRemaining=0;bChasing=false;AttackCooldown=1.5f;GetCharacterMovement()->SetMovementMode(MOVE_Walking);SetActorLocation(Home);SetActorHiddenInGame(false);SetActorEnableCollision(true);for(auto* M:Stones)M->SetMaterial(0,StoneMaterial);ChooseDestination();}return;}
 const bool WasFrozen=FrozenRemaining>0;FrozenRemaining=FMath::Max(0.f,FrozenRemaining-D);SlowRemaining=FMath::Max(0.f,SlowRemaining-D);
 if(WasFrozen&&FrozenRemaining<=0){GetCharacterMovement()->SetMovementMode(MOVE_Walking);for(auto* M:Stones)M->SetMaterial(0,StoneMaterial);}
 if(FrozenRemaining<=0){
  UpdateCombat(D);
  if(!bChasing){
  if(T>ChooseAt||FVector::Dist2D(GetActorLocation(),WanderDestination)<80)ChooseDestination();
  FVector Dir=(WanderDestination-GetActorLocation()).GetSafeNormal2D();
  if(APawn* P=UGameplayStatics::GetPlayerPawn(this,0)){FVector Away=GetActorLocation()-P->GetActorLocation();if(Away.Size2D()<180)Dir=(Dir+Away.GetSafeNormal2D()*2).GetSafeNormal2D();}
  GetCharacterMovement()->MaxWalkSpeed=SlowRemaining>0?65:145;AddMovementInput(Dir,1);
  }
  const float Speed=GetVelocity().Size2D();WalkPhase+=D*Speed*.035f;BodyRoot->SetRelativeLocation(FVector(0,0,-122+FMath::Abs(FMath::Sin(WalkPhase))*4));
  for(int j=5;j<Stones.Num();j++){const float Swing=FMath::Sin(WalkPhase+(j%2)*PI)*(j>=9?14:10);Stones[j]->SetRelativeLocation(StoneOffsets[j]+FVector(Swing,0,j>=9?FMath::Max(0.f,Swing*.35f):0));Stones[j]->SetRelativeRotation(FRotator(Swing*.6f,0,0));}
 }
 if(bSelected){DrawDebugCircle(GetWorld(),FM::Feet(this)+FVector(0,0,5),85,64,FrozenRemaining>0?FColor(100,215,255):FColor(200,115,255),false,-1,0,1.8f,FVector(1,0,0),FVector(0,1,0),false);}
}

AFMFrostProjectile::AFMFrostProjectile(){
 PrimaryActorTick.bCanEverTick=true;RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
 Frost=CreateDefaultSubobject<UNiagaraComponent>(TEXT("Frostbolt"));Frost->SetupAttachment(RootComponent);Frost->SetAsset(FM::Asset<UNiagaraSystem>(TEXT("/ModularGameVFX/ModularGameVFXLibrary/02_Projectile/NS_Ice_Frostbolt_Projectile")));
 Impact=FM::Asset<UNiagaraSystem>(TEXT("/ModularGameVFX/ModularGameVFXLibrary/05_Impact/NS_Ice_Frostbolt_Impact"));
 Glow=CreateDefaultSubobject<UPointLightComponent>(TEXT("ColdLight"));Glow->SetupAttachment(RootComponent);Glow->SetIntensity(75);Glow->SetLightColor(FLinearColor(.2f,.65f,1));Glow->SetAttenuationRadius(290);Glow->SetCastShadows(false);
}
void AFMFrostProjectile::BeginPlay(){Super::BeginPlay();SetLifeSpan(5);}
void AFMFrostProjectile::Tick(float D){Super::Tick(D);if(bHit)return;Alive+=D;
 FVector Next=GetActorLocation()+GetActorForwardVector()*Speed*D;
 if(IsValid(Target)&&Target->Health>0){
  FVector Aim=Target->GetActorLocation()+FVector(0,0,25);FVector To=Aim-GetActorLocation();
  if(To.Size()<Speed*D+58){bHit=true;UNiagaraFunctionLibrary::SpawnSystemAtLocation(this,Impact,Aim,GetActorRotation());Target->ReceiveFrost(Damage,false);UE_LOG(LogTemp,Display,TEXT("FM_BOLT_HIT Health=%.0f"),Target->Health);Destroy();return;}
  SetActorRotation(To.Rotation());Next=GetActorLocation()+To.GetSafeNormal()*Speed*D;
 }
 if(IsValid(PlayerTarget)&&PlayerTarget->Health>0){
  const FVector Aim=PlayerTarget->GetActorLocation()+FVector(0,0,10);const FVector To=Aim-GetActorLocation();
  if(To.Size()<Speed*D+34){bHit=true;UNiagaraFunctionLibrary::SpawnSystemAtLocation(this,Impact,Aim,GetActorRotation());PlayerTarget->ReceiveEnemyFrost(Damage);Destroy();return;}
  SetActorRotation(To.Rotation());Next=GetActorLocation()+GetActorForwardVector()*Speed*D;
 }
 SetActorLocation(Next);if(Alive>3)Destroy();
}

AFMPetalSpell::AFMPetalSpell(){
 PrimaryActorTick.bCanEverTick=true;
 PrimaryActorTick.TickGroup=TG_PostPhysics;
 RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("BloomRoot"));
 Petals=CreateDefaultSubobject<UNiagaraComponent>(TEXT("PetalBloom"));Petals->SetupAttachment(RootComponent);Petals->SetAutoActivate(false);
 Petals->SetAsset(FM::Asset<UNiagaraSystem>(TEXT("/ModularGameVFX/ModularGameVFXLibrary/07_Status/NS_Nature_PetalBloom_Aura")));Petals->SetCastShadow(false);
 BloomLight=CreateDefaultSubobject<UPointLightComponent>(TEXT("PetalBounce"));BloomLight->SetupAttachment(RootComponent);BloomLight->SetRelativeLocation(FVector(0,0,65));BloomLight->SetAttenuationRadius(400);BloomLight->SetIntensity(0);BloomLight->SetCastShadows(false);
}
void AFMPetalSpell::BeginPlay(){
 Super::BeginPlay();Duration=FMath::Clamp(Duration,1.f,15.f);Intensity=FMath::Clamp(Intensity,0.f,3.f);
 if(IsValid(FollowTarget)){AddTickPrerequisiteComponent(FollowTarget->GetCharacterMovement());UpdateFollowLocation();}
 Petals->AddTickPrerequisiteActor(this);
 Petals->SetVariableLinearColor(TEXT("User.GlowColor"),GlowColor);Petals->SetVariableLinearColor(TEXT("User.PetalColor"),PetalColor);Petals->SetVariableLinearColor(TEXT("User.CoreColor"),CoreColor);
 Petals->SetVariableFloat(TEXT("User.Intensity"),Intensity);Petals->SetVariableFloat(TEXT("User.SpellDuration"),Duration);Petals->Activate(true);
 BloomLight->SetLightColor(GlowColor);SetLifeSpan(Duration+.15f);
 UE_LOG(LogTemp,Display,TEXT("FM_PETAL_SPAWN Duration=%.1f Glow=(%.3f,%.3f,%.3f)"),Duration,GlowColor.R,GlowColor.G,GlowColor.B);
}
void AFMPetalSpell::UpdateFollowLocation(){
 if(!IsValid(FollowTarget))return;
 FVector Center=FM::Feet(FollowTarget);
 FHitResult Ground;FCollisionQueryParams Query(SCENE_QUERY_STAT(HealingBloomGround),false,this);Query.AddIgnoredActor(FollowTarget);
 if(GetWorld()->LineTraceSingleByObjectType(Ground,Center+FVector(0,0,100),Center-FVector(0,0,1500),FCollisionObjectQueryParams(ECC_WorldStatic),Query))Center.Z=Ground.ImpactPoint.Z;
 SetActorLocation(Center+FVector(0,0,3));
}
void AFMPetalSpell::Tick(float D){
 Super::Tick(D);
 if(bHealing&&!IsValid(FollowTarget)){Destroy();return;}
 UpdateFollowLocation();
 const float ActiveDelta=FMath::Clamp(Duration-Age,0.f,D);Age+=D;
 if(bHealing&&IsValid(FollowTarget))FollowTarget->ApplyHealing(FMath::Max(0.f,HealingPerSecond)*ActiveDelta);
 const float Fade=FMath::Clamp(Age/.3f,0.f,1.f)*FMath::Clamp((Duration-Age)/1.1f,0.f,1.f);
 BloomLight->SetIntensity(95.f*Intensity*Fade);
}

AFMFrostMage::AFMFrostMage(){
 PrimaryActorTick.bCanEverTick=true;GetCapsuleComponent()->InitCapsuleSize(34,96);
 GetMesh()->SetSkeletalMesh(FM::Asset<USkeletalMesh>(TEXT("/ModularGameVFXDemo/Characters/Mannequins/Meshes/SKM_Manny_Simple")));GetMesh()->SetRelativeLocation(FVector(0,0,-96));GetMesh()->SetRelativeRotation(FRotator(0,-90,0));
 GetCharacterMovement()->MaxWalkSpeed=380;GetCharacterMovement()->JumpZVelocity=490;GetCharacterMovement()->AirControl=.3f;GetCharacterMovement()->bOrientRotationToMovement=false;GetCharacterMovement()->RotationRate=FRotator(0,640,0);bUseControllerRotationYaw=false;
 CameraBoom=CreateDefaultSubobject<USpringArmComponent>(TEXT("ThirdPersonCamera"));CameraBoom->SetupAttachment(RootComponent);CameraBoom->TargetArmLength=690;CameraBoom->TargetOffset=FVector(0,0,80);CameraBoom->bUsePawnControlRotation=true;CameraBoom->bEnableCameraLag=true;CameraBoom->CameraLagSpeed=9;
 FollowCamera=CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));FollowCamera->SetupAttachment(CameraBoom,USpringArmComponent::SocketName);FollowCamera->FieldOfView=65;
 float MoonFillIntensity=12000.f;GConfig->GetFloat(TEXT("FrostMage.Visuals"),TEXT("CharacterMoonFillIntensity"),MoonFillIntensity,GGameIni);
 auto* MoonFill=CreateDefaultSubobject<UPointLightComponent>(TEXT("CharacterMoonFill"));MoonFill->SetupAttachment(RootComponent);MoonFill->SetRelativeLocation(FVector(-100,-140,240));MoonFill->SetLightColor(FLinearColor(.48f,.66f,1.f));MoonFill->SetIntensity(FMath::Clamp(MoonFillIntensity,0.f,50000.f));MoonFill->SetAttenuationRadius(1050.f);MoonFill->SetSourceRadius(100.f);MoonFill->SetCastShadows(false);
 ChargeSystem=FM::Asset<UNiagaraSystem>(TEXT("/ModularGameVFX/ModularGameVFXLibrary/01_Cast/NS_Ice_Frostbolt_Charge"));NovaSystem=FM::Asset<UNiagaraSystem>(TEXT("/ModularGameVFX/ModularGameVFXLibrary/06_Area/NS_Ice_FrostNova_Burst"));
 FireChargeSystem=FM::Asset<UNiagaraSystem>(TEXT("/ModularGameVFX/ModularGameVFXLibrary/01_Cast/NS_Fire_Fireball_Charge"));FireImpactSystem=FM::Asset<UNiagaraSystem>(TEXT("/ModularGameVFX/ModularGameVFXLibrary/05_Impact/NS_Fire_Impact_Burst"));FireTelegraphSystem=FM::Asset<UNiagaraSystem>(TEXT("/ModularGameVFX/ModularGameVFXLibrary/14_Indicator/NS_Fire_Flamestrike_Warning"));
 ArcaneChannelSystem=FM::Asset<UNiagaraSystem>(TEXT("/ModularGameVFX/ModularGameVFXLibrary/01_Cast/NS_Arcane_Missiles_Channel"));
 BoltAnimation=FM::Asset<UAnimSequence>(TEXT("/ModularGameVFXDemo/Animations/AS_Manny_CastProjectile"));NovaAnimation=FM::Asset<UAnimSequence>(TEXT("/ModularGameVFXDemo/Animations/AS_Manny_CastGroundBurst"));
 IdleAnimation=FM::Asset<UAnimSequence>(TEXT("/ModularGameVFXDemo/Characters/Mannequins/Anims/Unarmed/MM_Idle"));WalkAnimation=FM::Asset<UAnimSequence>(TEXT("/ModularGameVFXDemo/Characters/Mannequins/Anims/Unarmed/Jog/MF_Unarmed_Jog_Fwd"));
 const TCHAR* Directions[]={TEXT("Fwd"),TEXT("Fwd_Right"),TEXT("Right"),TEXT("Bwd_Right"),TEXT("Bwd"),TEXT("Bwd_Left"),TEXT("Left"),TEXT("Fwd_Left")};
 for(const TCHAR* Direction:Directions){
  JogDirections.Add(FM::Asset<UAnimSequence>(*FString::Printf(TEXT("/ModularGameVFXDemo/Characters/Mannequins/Anims/Unarmed/Jog/MF_Unarmed_Jog_%s"),Direction)));
  WalkDirections.Add(FM::Asset<UAnimSequence>(*FString::Printf(TEXT("/ModularGameVFXDemo/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_%s"),Direction)));
 }
 SpellLight=CreateDefaultSubobject<UPointLightComponent>(TEXT("SpellFlash"));SpellLight->SetupAttachment(RootComponent);SpellLight->SetRelativeLocation(FVector(45,0,35));SpellLight->SetLightColor(FLinearColor(.18f,.60f,1));SpellLight->SetAttenuationRadius(650);SpellLight->SetIntensity(0);SpellLight->SetCastShadows(false);
}
void AFMFrostMage::PlayPose(UAnimSequence* A,bool Loop){CurrentLocomotion=nullptr;if(A){GetMesh()->PlayAnimation(A,Loop);GetMesh()->SetPlayRate(1.f);}}
void AFMFrostMage::UpdateLocomotion(){
 const FVector Local=GetActorRotation().UnrotateVector(GetVelocity());bWalking=Local.Size2D()>15;
 UAnimSequence* Pose=IdleAnimation;
 if(bWalking){const int Direction=(FMath::RoundToInt(FMath::RadiansToDegrees(FMath::Atan2(Local.Y,Local.X))/45.f)+8)%8;
  const auto& Poses=(bWalkMode||Local.X<-15)?WalkDirections:JogDirections;Pose=Poses.IsValidIndex(Direction)?Poses[Direction]:WalkAnimation;
 }
 if(Pose&&Pose!=CurrentLocomotion){PlayPose(Pose,true);CurrentLocomotion=Pose;}
 if(bWalking)GetMesh()->SetPlayRate(FMath::Clamp(Local.Size2D()/((bWalkMode||Local.X<-15)?180.f:380.f),.65f,1.4f));
}
void AFMFrostMage::BeginPlay(){Super::BeginPlay();SpawnLocation=GetActorLocation();MaxHealth=FMath::Max(1.f,MaxHealth);Health=FMath::Clamp(Health,0.f,MaxHealth);PlayPose(IdleAnimation,true);Message(TEXT("1-4 Frost / Bloom  |  5 Fireball  |  6 Fire Blast  |  7 Flamestrike  |  8 Arcane"));}
void AFMFrostMage::Message(const FString& S){StatusMessage=S;StatusRemaining=3;}
void AFMFrostMage::SelectNextTarget(){CycleTarget(1);}
void AFMFrostMage::SelectPreviousTarget(){CycleTarget(-1);}
void AFMFrostMage::CycleTarget(int Direction){
 TArray<AFMFrostMonster*> Candidates;for(TActorIterator<AFMFrostMonster> I(GetWorld());I;++I)if(I->Health>0)Candidates.Add(*I);
 Candidates.Sort([this](const AFMFrostMonster& A,const AFMFrostMonster& B){return GetSquaredDistanceTo(&A)<GetSquaredDistanceTo(&B);});
 const int Index=Candidates.IndexOfByKey(Target);if(Target)Target->bSelected=false;
 Target=Candidates.Num()?Candidates[Index==INDEX_NONE?(Direction>0?0:Candidates.Num()-1):(Index+Direction+Candidates.Num())%Candidates.Num()]:nullptr;if(Target)Target->bSelected=true;
}
void AFMFrostMage::CastFrostbolt(){
 if(Health<=0||CastingSpell||BoltCooldown>0)return;if(!IsValid(Target)||Target->Health<=0)SelectNextTarget();
 if(!Target){Message(TEXT("Select a living target with Tab"));return;}
 if(GetDistanceTo(Target)>2400){Message(TEXT("Target is out of range (24m)"));return;}
 GetCharacterMovement()->StopMovementImmediately();SetActorRotation((Target->GetActorLocation()-GetActorLocation()).GetSafeNormal2D().Rotation());
 CastingSpell=1;CastDuration=CastRemaining=1.5f;Recovery=0;SpellLight->SetLightColor(FLinearColor(.18f,.60f,1));PlayPose(BoltAnimation,false);
 Charge=UNiagaraFunctionLibrary::SpawnSystemAttached(ChargeSystem,RootComponent,NAME_None,FVector(0,0,-96),FRotator::ZeroRotator,EAttachLocation::KeepRelativeOffset,true,true,ENCPoolMethod::AutoRelease);
 UE_LOG(LogTemp,Display,TEXT("FM_BOLT_CAST_BEGIN"));
}
void AFMFrostMage::CastFrostNova(){
 if(Health<=0||CastingSpell||NovaCooldown>0)return;CastingSpell=2;CastDuration=CastRemaining=0;Recovery=0;SpellLight->SetLightColor(FLinearColor(.18f,.60f,1));
 if(GetVelocity().Size2D()<15)PlayPose(NovaAnimation,false);
 ReleaseSpell();UE_LOG(LogTemp,Display,TEXT("FM_NOVA_INSTANT"));
}
void AFMFrostMage::CastPetalBloom(){
 if(Health<=0||CastingSpell||PetalCooldown>0)return;
 GetCharacterMovement()->StopMovementImmediately();CastingSpell=3;CastDuration=CastRemaining=.72f;Recovery=0;
 SpellLight->SetLightColor(PetalGlowColor);PlayPose(NovaAnimation,false);UE_LOG(LogTemp,Display,TEXT("FM_PETAL_CAST_BEGIN"));
}
void AFMFrostMage::CastHealingBloom(){
 if(Health<=0||CastingSpell||HealingCooldown>0)return;
 CastingSpell=4;CastDuration=CastRemaining=.35f;Recovery=0;
 SpellLight->SetLightColor(HealingGlowColor);
 if(GetVelocity().Size2D()<15)PlayPose(NovaAnimation,false);
 UE_LOG(LogTemp,Display,TEXT("FM_HEAL_CAST_BEGIN"));
}
float AFMFrostMage::ApplyHealing(float Amount){
 if(!FMath::IsFinite(Amount)||Amount<=0||Health<=0)return 0;
 const float Previous=Health;Health=FMath::Min(Health+Amount,FMath::Max(1.f,MaxHealth));return Health-Previous;
}
void AFMFrostMage::ReceiveEnemyFrost(float Damage){
 if(Health<=0||!FMath::IsFinite(Damage)||Damage<=0)return;
 Health=FMath::Max(0.f,Health-Damage);ChilledRemaining=2.f;
 Message(FString::Printf(TEXT("Enemy Frostbolt  -%.0f HP  /  Chilled"),Damage));UE_LOG(LogTemp,Display,TEXT("FM_PLAYER_FROST_HIT Health=%.1f"),Health);
 if(Health<=0){CancelCast();RespawnRemaining=3.f;GetCharacterMovement()->StopMovementImmediately();for(TActorIterator<AFMPetalSpell> I(GetWorld());I;++I)if(I->FollowTarget==this)I->Destroy();HealingRemaining=0;}
}
void AFMFrostMage::CancelCast(){if(CastingSpell){CastingSpell=0;CastRemaining=0;if(IsValid(Charge))Charge->DeactivateImmediate();Charge=nullptr;ClearFirePreview();FireCastTarget=nullptr;ArcaneTarget=nullptr;PlayPose(IdleAnimation,true);Message(TEXT("Cast interrupted"));}}
void AFMFrostMage::SetMovementIntent(float F,float R){
 bBackpedaling=F<-.05f;
 GetCharacterMovement()->MaxWalkSpeed=(ChilledRemaining>0?.75f:1.f)*(bWalkMode?160.f:(bBackpedaling?190.f:380.f));
 if(Health<=0)return;
 if(FMath::Abs(F)+FMath::Abs(R)>.05f){if(CastingSpell==1||CastingSpell==5||CastingSpell==7||CastingSpell==8)CancelCast();if(CastingSpell==2||CastingSpell==3)return;
  if(CastingSpell==4&&!bWalking){bWalking=true;PlayPose(WalkAnimation?WalkAnimation:IdleAnimation,true);}
  const FRotator Yaw(0,GetActorRotation().Yaw,0);AddMovementInput(Yaw.Vector(),F);AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y),R);
 }
}
void AFMFrostMage::ReleaseSpell(){
 const int Spell=CastingSpell;CastingSpell=0;CastRemaining=0;if(IsValid(Charge))Charge->Deactivate();Charge=nullptr;
 if(Spell==8){ArcaneTarget=nullptr;Recovery=.3f;Message(TEXT("Arcane Missiles  /  5 bolts  /  45 damage"));UE_LOG(LogTemp,Display,TEXT("AM_CHANNEL_COMPLETE Shots=%d"),ArcaneShotsFired);return;}
 if(Spell>=5){ReleaseFireSpell(Spell);return;}
 if(Spell==1){
  if(!IsValid(Target)||Target->Health<=0){Message(TEXT("Target lost"));Recovery=.25f;return;}
  FVector Start=FM::Feet(this)+GetActorForwardVector()*60+FVector(0,0,132);FRotator Dir=(Target->GetActorLocation()+FVector(0,0,25)-Start).Rotation();
  FActorSpawnParameters P;P.Owner=this;P.Instigator=this;auto* Bolt=GetWorld()->SpawnActor<AFMFrostProjectile>(AFMFrostProjectile::StaticClass(),Start,Dir,P);if(Bolt)Bolt->Target=Target;
  BoltCooldown=.65f;Recovery=.85f;LightLife=.26f;Message(TEXT("Frostbolt released"));
 }else if(Spell==2){
  UNiagaraFunctionLibrary::SpawnSystemAtLocation(this,NovaSystem,FM::Feet(this)+FVector(0,0,3),GetActorRotation(),FVector(1),true,true,ENCPoolMethod::AutoRelease);
  int Hits=0;for(TActorIterator<AFMFrostMonster> I(GetWorld());I;++I)if(I->Health>0&&FVector::Dist2D(GetActorLocation(),I->GetActorLocation())<=550){I->ReceiveFrost(16,true);Hits++;}
  NovaCooldown=6;Recovery=1.35f;LightLife=.7f;Message(FString::Printf(TEXT("Frost Nova: %d target(s) frozen for 5s"),Hits));UE_LOG(LogTemp,Display,TEXT("FM_NOVA_RELEASE Hits=%d"),Hits);
 }else if(Spell==3){
  const FTransform Transform(GetActorRotation(),FM::Feet(this)+FVector(0,0,3),FVector(1.15f));
  auto* Bloom=GetWorld()->SpawnActorDeferred<AFMPetalSpell>(AFMPetalSpell::StaticClass(),Transform,this,this,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
  if(Bloom){Bloom->GlowColor=PetalGlowColor;Bloom->PetalColor=PetalSurfaceColor;Bloom->CoreColor=PetalCoreColor;Bloom->Intensity=PetalIntensity;UGameplayStatics::FinishSpawningActor(Bloom,Transform);}
  PetalCooldown=8;Recovery=1.35f;LightLife=.65f;Message(TEXT("Rose Petal Bloom  /  6 seconds"));UE_LOG(LogTemp,Display,TEXT("FM_PETAL_RELEASE"));
 }else if(Spell==4){
  const FTransform Transform(FRotator::ZeroRotator,FM::Feet(this)+FVector(0,0,3),FVector(1.15f));
  auto* Bloom=GetWorld()->SpawnActorDeferred<AFMPetalSpell>(AFMPetalSpell::StaticClass(),Transform,this,this,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
  if(Bloom){Bloom->GlowColor=HealingGlowColor;Bloom->PetalColor=HealingPetalColor;Bloom->CoreColor=HealingCoreColor;Bloom->Intensity=HealingIntensity;Bloom->FollowTarget=this;Bloom->bHealing=true;UGameplayStatics::FinishSpawningActor(Bloom,Transform);}
  HealingCooldown=8;HealingRemaining=6;Recovery=0;LightLife=.45f;
  bWalking=GetVelocity().Size2D()>15;PlayPose(bWalking&&WalkAnimation?WalkAnimation:IdleAnimation,true);
  Message(TEXT("Healing Bloom  /  +12 HP per second  /  Move freely"));UE_LOG(LogTemp,Display,TEXT("FM_HEAL_RELEASE"));
 }
}
void AFMFrostMage::Tick(float D){Super::Tick(D);NovaCooldown=FMath::Max(0.f,NovaCooldown-D);BoltCooldown=FMath::Max(0.f,BoltCooldown-D);PetalCooldown=FMath::Max(0.f,PetalCooldown-D);HealingCooldown=FMath::Max(0.f,HealingCooldown-D);HealingRemaining=FMath::Max(0.f,HealingRemaining-D);StatusRemaining=FMath::Max(0.f,StatusRemaining-D);
 ChilledRemaining=FMath::Max(0.f,ChilledRemaining-D);
 ArcaneCooldown=FMath::Max(0.f,ArcaneCooldown-D);
 WeaponSwingRemaining=FMath::Max(0.f,WeaponSwingRemaining-D);
 if(bAutoAttacking&&(!IsValid(Target)||Target->Health<=0||Health<=0))bAutoAttacking=false;
 if(bAutoAttacking&&!CastingSpell&&WeaponSwingRemaining<=0&&GetDistanceTo(Target)<210&&FVector::DotProduct(GetActorForwardVector(),(Target->GetActorLocation()-GetActorLocation()).GetSafeNormal2D())>.5f&&Controller&&Controller->LineOfSightTo(Target)){
  Target->ReceiveWeaponDamage(6);WeaponSwingRemaining=2;LightLife=.1f;Message(TEXT("Auto attack  /  6 damage"));UE_LOG(LogTemp,Display,TEXT("FM_MOUSE_AUTO_ATTACK"));
 }
 FireballCooldown=FMath::Max(0.f,FireballCooldown-D);FireBlastCooldown=FMath::Max(0.f,FireBlastCooldown-D);FlamestrikeCooldown=FMath::Max(0.f,FlamestrikeCooldown-D);
 if(Health<=0){RespawnRemaining-=D;if(RespawnRemaining<=0){Health=MaxHealth;ChilledRemaining=0;SetActorLocation(SpawnLocation);Message(TEXT("Recovered at the test spawn"));}return;}
 if(CastingSpell==8)TickArcaneChannel(D);
 if(CastingSpell){const auto* PC=Cast<AFMPlayerController>(Controller);if((CastingSpell==1||CastingSpell==5||CastingSpell==8)&&IsValid(Target)&&!(PC&&PC->bRightMouseHeld))SetActorRotation((Target->GetActorLocation()-GetActorLocation()).GetSafeNormal2D().Rotation());CastRemaining-=D;if(CastRemaining<=0)ReleaseSpell();}
 else if(Recovery>0){Recovery-=D;if(Recovery<=0){bWalking=false;PlayPose(IdleAnimation,true);}}
 if((!CastingSpell&&(Recovery<=0||GetVelocity().Size2D()>15))||(CastingSpell==4&&GetVelocity().Size2D()>15))UpdateLocomotion();
 LightLife=FMath::Max(0.f,LightLife-D);SpellLight->SetIntensity(CastingSpell?28*(1-CastRemaining/CastDuration):(LightLife>0?220*LightLife:0));
}

AFMPlayerController::AFMPlayerController(){bShowMouseCursor=true;DefaultMouseCursor=EMouseCursor::Default;}
bool AFMPlayerController::IsDebugUIInteractive() const{return ActorHasTag(TEXT("VFXShowcaseUIInteractive"));}
void AFMPlayerController::SetCursorMode(){FInputModeGameAndUI Mode;Mode.SetHideCursorDuringCapture(false);Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);SetInputMode(Mode);bShowMouseCursor=true;}
void AFMPlayerController::BeginPlay(){Super::BeginPlay();SetCursorMode();SetControlRotation(FRotator(-20,0,0));
 const TCHAR* Section=TEXT("FrostMage.Mouse");
 GConfig->GetFloat(Section,TEXT("CameraYawMoveSpeed"),CameraYawMoveSpeed,GGameIni);GConfig->GetFloat(Section,TEXT("CameraPitchMoveSpeed"),CameraPitchMoveSpeed,GGameIni);
 GConfig->GetFloat(Section,TEXT("MouseSensitivityMultiplier"),MouseSensitivityMultiplier,GGameIni);GConfig->GetFloat(Section,TEXT("RawDegreesPerCount"),RawDegreesPerCount,GGameIni);
 GConfig->GetBool(Section,TEXT("InvertMousePitch"),bInvertMousePitch,GGameIni);GConfig->GetBool(Section,TEXT("SmartCameraFollow"),bSmartCameraFollow,GGameIni);
 if(auto* P=Cast<AFMFrostMage>(GetPawn()))ZoomGoal=P->CameraBoom->TargetArmLength;
 UE_LOG(LogTemp,Display,TEXT("FM_MOUSE_PROFILE Yaw=%.1f Pitch=%.1f RawScale=%.3f Multiplier=%.2f"),CameraYawMoveSpeed,CameraPitchMoveSpeed,RawDegreesPerCount,MouseSensitivityMultiplier);
}
void AFMPlayerController::SetupInputComponent(){Super::SetupInputComponent();
 // Gate this controller's combat bindings only; F1 on the Showcase actor still runs.
 auto BindCombat=[this](FInputChord Chord,EInputEvent Event,void(AFMPlayerController::*Action)()){
  FInputKeyBinding Binding(Chord,Event);
  Binding.KeyDelegate.GetDelegateForManualSet().BindWeakLambda(this,[this,Action](){if(!IsDebugUIInteractive())(this->*Action)();});
  InputComponent->KeyBindings.Add(MoveTemp(Binding));
 };
 BindCombat(EKeys::One,IE_Pressed,&AFMPlayerController::FMTestBolt);BindCombat(EKeys::Two,IE_Pressed,&AFMPlayerController::FMTestNova);BindCombat(EKeys::Three,IE_Pressed,&AFMPlayerController::FMTestPetal);BindCombat(EKeys::Tab,IE_Pressed,&AFMPlayerController::FMTestSelect);
 BindCombat(EKeys::Eight,IE_Pressed,&AFMPlayerController::FMTestArcane);
 BindCombat(EKeys::Four,IE_Pressed,&AFMPlayerController::FMTestHeal);
 BindCombat(EKeys::Five,IE_Pressed,&AFMPlayerController::FMTestFireball);BindCombat(EKeys::Six,IE_Pressed,&AFMPlayerController::FMTestFireBlast);BindCombat(EKeys::Seven,IE_Pressed,&AFMPlayerController::FMTestFlamestrike);
 BindCombat(EKeys::LeftMouseButton,IE_Pressed,&AFMPlayerController::BeginLeftMouse);BindCombat(EKeys::LeftMouseButton,IE_Released,&AFMPlayerController::EndLeftMouse);
 BindCombat(EKeys::RightMouseButton,IE_Pressed,&AFMPlayerController::BeginOrbit);BindCombat(EKeys::RightMouseButton,IE_Released,&AFMPlayerController::EndOrbit);
 BindCombat(EKeys::SpaceBar,IE_Pressed,&AFMPlayerController::JumpStart);BindCombat(EKeys::SpaceBar,IE_Released,&AFMPlayerController::JumpStop);
 BindCombat(EKeys::MouseScrollUp,IE_Pressed,&AFMPlayerController::ZoomIn);BindCombat(EKeys::MouseScrollDown,IE_Pressed,&AFMPlayerController::ZoomOut);BindCombat(EKeys::Escape,IE_Pressed,&AFMPlayerController::Cancel);
 BindCombat(EKeys::NumLock,IE_Pressed,&AFMPlayerController::ToggleAutoRun);BindCombat(EKeys::Divide,IE_Pressed,&AFMPlayerController::ToggleWalk);
 BindCombat(FInputChord(EKeys::Tab,true,false,false,false),IE_Pressed,&AFMPlayerController::FMTestSelect);
}
void AFMPlayerController::PlayerTick(float D){Super::PlayerTick(D);auto* P=Cast<AFMFrostMage>(GetPawn());if(!P)return;
 const bool bDebugInteractive=IsDebugUIInteractive();
 if(bDebugInteractive||bWasDebugUIInteractive){
  // Do not call our FlushPressedKeys override here: it resets input mode and would
  // steal focus from the debug search field. Only clear game key/held-button state.
  if(bDebugInteractive!=bWasDebugUIInteractive)Super::FlushPressedKeys();
  bWasDebugUIInteractive=bDebugInteractive;
  bLeftMouseHeld=bRightMouseHeld=bOrbiting=bAutoRun=false;
  bLeftMouseDragged=bRightMouseDragged=false;LeftMouseTravel=RightMouseTravel=0;
  P->SetMovementIntent(0,0);P->GetCharacterMovement()->StopMovementImmediately();P->bAutoAttacking=false;
  if(bDebugInteractive)return;
 }
 if(bOrbiting){float X=0,Y=0;GetInputMouseDelta(X,Y);
  if(bSkipFirstOrbitDelta){X=Y=0;bSkipFirstOrbitDelta=false;}
  if(bLeftMouseHeld){LeftMouseTravel+=FMath::Abs(X)+FMath::Abs(Y);bLeftMouseDragged|=LeftMouseTravel>4.f;}
  if(bRightMouseHeld){RightMouseTravel+=FMath::Abs(X)+FMath::Abs(Y);bRightMouseDragged|=RightMouseTravel>4.f;}
  const float Gain=FMath::Clamp(RawDegreesPerCount,.001f,1.f)*FMath::Clamp(MouseSensitivityMultiplier,.1f,4.f);
  FRotator R=GetControlRotation();R.Yaw+=X*Gain*FMath::Clamp(CameraYawMoveSpeed,10.f,720.f)/180.f;
  R.Pitch=FMath::Clamp(FRotator::NormalizeAxis(R.Pitch)+Y*Gain*FMath::Clamp(CameraPitchMoveSpeed,5.f,360.f)/180.f*(bInvertMousePitch?-1.f:1.f),-89.f,89.f);R.Roll=0;SetControlRotation(R);CameraFollowDelay=.65f;
 }
 CameraFollowDelay=FMath::Max(0.f,CameraFollowDelay-D);
 if(!bOrbiting&&bSmartCameraFollow&&CameraFollowDelay<=0&&P->GetVelocity().Size2D()>20){FRotator R=GetControlRotation();R.Yaw=FMath::FixedTurn(R.Yaw,P->GetActorRotation().Yaw,180.f*D);SetControlRotation(R);}
 P->CameraBoom->TargetArmLength=FMath::FInterpConstantTo(P->CameraBoom->TargetArmLength,ZoomGoal,D,600.f);P->GetMesh()->SetOwnerNoSee(P->CameraBoom->TargetArmLength<90.f);
 if(bRightMouseHeld)P->SetActorRotation(FRotator(0,GetControlRotation().Yaw,0));
 const float Turn=(IsInputKeyDown(EKeys::D)||IsInputKeyDown(EKeys::Right)?1.f:0.f)-(IsInputKeyDown(EKeys::A)||IsInputKeyDown(EKeys::Left)?1.f:0.f);
 if(!bRightMouseHeld&&Turn!=0&&P->Health>0){const float YawDelta=Turn*110.f*D;P->AddActorWorldRotation(FRotator(0,YawDelta,0));if(!bLeftMouseHeld){FRotator R=GetControlRotation();R.Yaw+=YawDelta;SetControlRotation(R);}}
 const bool ForwardKey=IsInputKeyDown(EKeys::W)||IsInputKeyDown(EKeys::Up),BackKey=IsInputKeyDown(EKeys::S)||IsInputKeyDown(EKeys::Down);
 if(ForwardKey||BackKey||(bLeftMouseHeld&&bRightMouseHeld)||P->Health<=0)bAutoRun=false;
 const float F=bLeftMouseHeld&&bRightMouseHeld?1.f:(ForwardKey||bAutoRun?1.f:0.f)-(BackKey?1.f:0.f);
 const float S=FMath::Clamp((IsInputKeyDown(EKeys::E)?1.f:0.f)-(IsInputKeyDown(EKeys::Q)?1.f:0.f)+(bRightMouseHeld?Turn:0.f),-1.f,1.f);P->SetMovementIntent(F,S);
}
void AFMPlayerController::FlushPressedKeys(){Super::FlushPressedKeys();bLeftMouseHeld=bRightMouseHeld=bOrbiting=bAutoRun=false;bLeftMouseDragged=bRightMouseDragged=false;LeftMouseTravel=RightMouseTravel=0;bShowMouseCursor=true;SetCursorMode();}
void AFMPlayerController::FMTestBolt(){if(auto* P=Cast<AFMFrostMage>(GetPawn()))P->CastFrostbolt();}
void AFMPlayerController::SelectUnderCursor(){
 if(IsDebugUIInteractive())return;
 if(bOrbiting)return;auto* P=Cast<AFMFrostMage>(GetPawn());if(!P)return;
 FHitResult Hit;GetHitResultUnderCursor(ECC_Visibility,false,Hit);auto* Monster=Cast<AFMFrostMonster>(Hit.GetActor());
 if(IsValid(P->Target))P->Target->bSelected=false;
 P->Target=IsValid(Monster)&&Monster->Health>0?Monster:nullptr;
 if(IsValid(P->Target))P->Target->bSelected=true;
 UE_LOG(LogTemp,Display,TEXT("FM_MOUSE_TARGET %s"),P->Target?*P->Target->GetName():TEXT("None"));
}
void AFMPlayerController::RestoreCursor(){bOrbiting=false;SetCursorMode();SetMouseLocation(FMath::RoundToInt(MouseRestoreX),FMath::RoundToInt(MouseRestoreY));CameraFollowDelay=.65f;}
void AFMPlayerController::BeginLeftMouse(){
 const int32 Action=bRightMouseHeld?INDEX_NONE:FM::ActionAtCursor(this);if(Action!=INDEX_NONE){if(auto* P=Cast<AFMFrostMage>(GetPawn()))switch(Action){case 0:P->CastFrostbolt();break;case 1:P->CastFrostNova();break;case 2:P->CastPetalBloom();break;case 3:P->CastHealingBloom();break;case 4:P->CastFireball();break;case 5:P->CastFireBlast();break;case 6:P->CastFlamestrike();break;case 7:P->CastArcaneMissiles();break;}return;}
 if(bLeftMouseHeld)return;if(!bRightMouseHeld){GetMousePosition(MouseRestoreX,MouseRestoreY);bSkipFirstOrbitDelta=true;}
 if(!bRightMouseHeld){FInputModeGameOnly Mode;Mode.SetConsumeCaptureMouseDown(false);SetInputMode(Mode);}else bRightMouseDragged=true;
 bLeftMouseHeld=true;bOrbiting=true;bShowMouseCursor=false;LeftMouseTravel=0;bLeftMouseDragged=bRightMouseHeld;
}
void AFMPlayerController::EndLeftMouse(){
 if(!bLeftMouseHeld)return;bLeftMouseHeld=false;const bool bSelect=!bLeftMouseDragged&&!bRightMouseHeld;
 if(!bRightMouseHeld){RestoreCursor();if(bSelect)SelectUnderCursor();}
}
void AFMPlayerController::BeginOrbit(){
 if(!bLeftMouseHeld&&FM::ActionAtCursor(this)!=INDEX_NONE)return;
 if(bRightMouseHeld)return;if(!bLeftMouseHeld){GetMousePosition(MouseRestoreX,MouseRestoreY);bSkipFirstOrbitDelta=true;}else bLeftMouseDragged=true;
 if(!bLeftMouseHeld){FInputModeGameOnly Mode;Mode.SetConsumeCaptureMouseDown(false);SetInputMode(Mode);}
 RightMouseTravel=0;bRightMouseDragged=bLeftMouseHeld;
 bRightMouseHeld=true;bOrbiting=true;bShowMouseCursor=false;if(auto* P=Cast<AFMFrostMage>(GetPawn()))P->SetActorRotation(FRotator(0,GetControlRotation().Yaw,0));
}
void AFMPlayerController::EndOrbit(){if(!bRightMouseHeld)return;const bool Click=!bRightMouseDragged&&!bLeftMouseHeld;bRightMouseHeld=false;if(!bLeftMouseHeld){RestoreCursor();if(Click){FHitResult Hit;GetHitResultUnderCursor(ECC_Visibility,false,Hit);if(auto* M=Cast<AFMFrostMonster>(Hit.GetActor()))if(M->Health>0){SelectUnderCursor();if(auto* P=Cast<AFMFrostMage>(GetPawn()))P->bAutoAttacking=IsValid(P->Target);}}}}
void AFMPlayerController::ToggleAutoRun(){bAutoRun=!bAutoRun;}
void AFMPlayerController::ToggleWalk(){if(auto* P=Cast<AFMFrostMage>(GetPawn()))P->bWalkMode=!P->bWalkMode;}
void AFMPlayerController::FMTestNova(){if(auto* P=Cast<AFMFrostMage>(GetPawn()))P->CastFrostNova();}
void AFMPlayerController::FMTestPetal(){if(auto* P=Cast<AFMFrostMage>(GetPawn()))P->CastPetalBloom();}
void AFMPlayerController::FMTestHeal(){if(auto* P=Cast<AFMFrostMage>(GetPawn()))P->CastHealingBloom();}
void AFMPlayerController::FMTestSelect(){if(auto* P=Cast<AFMFrostMage>(GetPawn())){if(IsInputKeyDown(EKeys::LeftShift)||IsInputKeyDown(EKeys::RightShift))P->SelectPreviousTarget();else P->SelectNextTarget();}}
void AFMPlayerController::SimulateKey(FName KeyName,bool bPressed){InputKey(FInputKeyEventArgs::CreateSimulated(FKey(KeyName),bPressed?IE_Pressed:IE_Released,bPressed?1.f:0.f));}
void AFMPlayerController::SimulateMouse(float X,float Y){InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::MouseX,IE_Axis,X,1));InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::MouseY,IE_Axis,Y,1));}
void AFMPlayerController::JumpStart(){if(auto* P=Cast<AFMFrostMage>(GetPawn())){if(P->Health<=0)return;if(P->CastingSpell!=4)P->CancelCast();P->Jump();}}
void AFMPlayerController::JumpStop(){if(auto* P=Cast<AFMFrostMage>(GetPawn()))P->StopJumping();}
void AFMPlayerController::ZoomIn(){ZoomGoal=FMath::Max(0.f,ZoomGoal-70);}
void AFMPlayerController::ZoomOut(){ZoomGoal=FMath::Min(1250.f,ZoomGoal+70);}
void AFMPlayerController::Cancel(){if(auto* P=Cast<AFMFrostMage>(GetPawn())){if(P->CastingSpell)P->CancelCast();else{if(IsValid(P->Target))P->Target->bSelected=false;P->Target=nullptr;P->bAutoAttacking=false;}}}

void AFMFrostHUD::DrawHUD(){Super::DrawHUD();if(!Canvas)return;auto* P=Cast<AFMFrostMage>(GetOwningPawn());if(!P)return;
 const float W=Canvas->SizeX,H=Canvas->SizeY;UFont* Font=GEngine->GetMediumFont();
 auto Label=[&](const FString& S,float X,float Y,float Scale,FLinearColor C=FLinearColor(.72f,.83f,.94f)){DrawText(S,C,X,Y,Font,Scale,false);};
 DrawRect(FLinearColor(.006f,.012f,.024f,.8f),20,20,335,67);Label(TEXT("E M B E R  &  F R O S T"),36,28,1.25f,FLinearColor(.65f,.85f,1));Label(TEXT("THIRD PERSON  /  SPELL TEST GROUNDS"),36,59,.72f);
 if(IsValid(P->Target)&&P->Target->Health>0){
  const float X=W/2-155;DrawRect(FLinearColor(.008f,.012f,.026f,.92f),X,24,310,64);Label(TEXT("MOONSTONE GOLEM"),X+14,30,.95f,FLinearColor(.87f,.70f,1));
  DrawRect(FLinearColor(.15f,.10f,.22f,1),X+14,57,282,10);DrawRect(FLinearColor(.52f,.25f,.79f,1),X+14,57,282*P->Target->Health/100,10);
  FString State=P->Target->FrozenRemaining>0?FString::Printf(TEXT("FROZEN  %.1fs"),P->Target->FrozenRemaining):(P->Target->CastRemaining>0?FString::Printf(TEXT("CASTING FROSTBOLT  %.1fs"),P->Target->CastRemaining):(P->Target->bChasing?TEXT("PURSUING YOU"):TEXT("WANDERING")));Label(State,X+14,72,.67f);
 }
 const int Cols=W<1280?4:8,Rows=Cols==4?2:1;
 const float CardW=FMath::Min(170.f,(W-64-(Cols-1)*8)/Cols),Gap=8,BarW=Cols*CardW+(Cols-1)*Gap;
 const float X=(W-BarW)/2,Y=H-102-(Rows-1)*72;
 DrawRect(FLinearColor(.004f,.009f,.022f,.91f),X-10,Y-10,BarW+20,Rows*72+12);
 const TCHAR* Names[]={TEXT("FROSTBOLT"),TEXT("FROST NOVA"),TEXT("ROSE BLOOM"),TEXT("HEALING BLOOM"),TEXT("FIREBALL"),TEXT("FIRE BLAST"),TEXT("FLAMESTRIKE"),TEXT("ARCANE MISSILES")};
 const TCHAR* Hints[]={TEXT("24m / Cast 1.5s"),TEXT("Instant / Root 5s"),TEXT("Petals / 6s"),TEXT("+12 HP/s / 6s"),TEXT("30 dmg / 1.8s"),TEXT("22 dmg / Instant"),TEXT("AoE + burn / 2s"),TEXT("5 bolts / Channel")};
 const float CDs[]={P->BoltCooldown,P->NovaCooldown,P->PetalCooldown,P->HealingCooldown,P->FireballCooldown,P->FireBlastCooldown,P->FlamestrikeCooldown,P->ArcaneCooldown};
 const float MaxCDs[]={.65f,6.f,8.f,8.f,.5f,8.f,10.f,4.f};
 for(int i=0;i<8;i++){
  const float SX=X+(i%Cols)*(CardW+Gap),SY=Y+(i/Cols)*72,CD=CDs[i];
  const FLinearColor Accent=i==7?FLinearColor(.64f,.28f,1.f):(i>=4?FLinearColor(1,.32f,.065f):(i==3?FLinearColor(.13f,.82f,.58f):(i==2?FLinearColor(.98f,.28f,.58f):FLinearColor(.19f,.49f,.72f))));
  DrawRect(i>=4?FLinearColor(.16f,.044f,.018f,.96f):FLinearColor(.03f,.10f,.18f,.96f),SX,SY,CardW,62);DrawRect(Accent,SX,SY,3,62);
  if(CD>0)DrawRect(FLinearColor(.003f,.009f,.025f,.60f),SX,SY,CardW*FMath::Clamp(CD/MaxCDs[i],0.f,1.f),62);
  Label(FString::FromInt(i+1),SX+9,SY+9,1.3f,Accent);Label(Names[i],SX+33,SY+9,.71f);
  Label(CD>0?FString::Printf(TEXT("%.1fs"),CD):Hints[i],SX+33,SY+35,.64f,CD>0?FLinearColor(.9f,.69f,.3f):FLinearColor(.58f,.70f,.82f));
 }
 if(P->CastingSpell){
  const float BX=W/2-185,BY=Y-47;DrawRect(FLinearColor(.006f,.013f,.03f,.9f),BX,BY,370,24);
  const FLinearColor Tint=P->CastingSpell==8?FLinearColor(.48f,.15f,.95f,.9f):(P->CastingSpell>=5?FLinearColor(.95f,.26f,.025f,.9f):(P->CastingSpell==4?FLinearColor(.06f,.70f,.42f,.9f):FLinearColor(.17f,.56f,.88f,.9f)));
  DrawRect(Tint,BX+2,BY+2,366*(P->CastingSpell==8?P->CastRemaining/FMath::Max(.01f,P->CastDuration):1-P->CastRemaining/FMath::Max(.01f,P->CastDuration)),20);
  const TCHAR* CastNames[]={TEXT(""),TEXT("Conjuring Frostbolt..."),TEXT("Unleashing Frost Nova..."),TEXT("Summoning Rose Bloom..."),TEXT("Invoking Healing Bloom..."),TEXT("Conjuring Fireball..."),TEXT("Fire Blast"),TEXT("Calling Flamestrike..."),TEXT("Channeling Arcane Missiles...")};
  Label(CastNames[FMath::Clamp(P->CastingSpell,0,8)],BX+10,BY+2,.8f);
 }
 DrawRect(FLinearColor(.006f,.012f,.024f,.85f),20,99,335,68);
 Label(FString::Printf(TEXT("HEALTH  %.0f / %.0f"),P->Health,P->MaxHealth),36,106,.85f);
 DrawRect(FLinearColor(.04f,.14f,.12f,1),36,132,303,10);DrawRect(FLinearColor(.1f,.8f,.45f,1),36,132,303*FMath::Clamp(P->Health/FMath::Max(1.f,P->MaxHealth),0.f,1.f),10);
 if(P->HealingRemaining>0)Label(FString::Printf(TEXT("REGENERATING  +12 HP/s  %.1fs"),P->HealingRemaining),36,148,.70f,FLinearColor(.25f,1,.65f));
 else if(P->ChilledRemaining>0)Label(TEXT("CHILLED  /  Movement slowed"),36,148,.70f);
 if(P->Health<=0)Label(FString::Printf(TEXT("RECOVERING  %.1fs"),P->RespawnRemaining),W/2-130,H/2,1.3f,FLinearColor(1,.35f,.45f));
 if(P->StatusRemaining>0)Label(P->StatusMessage,W/2-220,Y-79,.82f,FLinearColor(.66f,.86f,1));
 Label(TEXT("W/S Move   A/D Turn   Q/E Strafe   LMB Select/Look   RMB Steer   Both Mouse Run   Wheel Zoom"),24,H-28,.66f);
}
AFMFrostGameMode::AFMFrostGameMode(){DefaultPawnClass=AFMFrostMage::StaticClass();PlayerControllerClass=AFMPlayerController::StaticClass();HUDClass=AFMFrostHUD::StaticClass();}
void AFMFrostGameMode::BeginPlay(){Super::BeginPlay();bool HasMonster=false;for(TActorIterator<AFMFrostMonster> I(GetWorld());I;++I)HasMonster=true;if(!HasMonster){FActorSpawnParameters P;P.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;GetWorld()->SpawnActor<AFMFrostMonster>(AFMFrostMonster::StaticClass(),FVector(700,0,122),FRotator(0,180,0),P);}}

