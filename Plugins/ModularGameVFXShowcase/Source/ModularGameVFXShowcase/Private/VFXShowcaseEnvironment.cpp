#include "VFXShowcaseEnvironment.h"
#include "Camera/CameraComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
UStaticMeshComponent* AddCube(AActor* Owner, FName Name, USceneComponent* Parent, const FVector& Location, const FVector& Scale)
{
    UStaticMeshComponent* Mesh = Owner->CreateDefaultSubobject<UStaticMeshComponent>(Name);
    Mesh->SetupAttachment(Parent);
    Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")));
    Mesh->SetRelativeLocation(Location);
    Mesh->SetRelativeScale3D(Scale);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    return Mesh;
}
void AddLabel(AActor* Owner, FName Name, const FString& Text, FVector Location, FColor Color = FColor::White)
{
    UTextRenderComponent* Label = Owner->CreateDefaultSubobject<UTextRenderComponent>(Name);
    Label->SetupAttachment(Owner->GetRootComponent());
    Label->SetText(FText::FromString(Text));
    Label->SetRelativeLocation(Location);
    Label->SetRelativeRotation(FRotator(0,-90,0));
    Label->SetWorldSize(45);
    Label->SetTextRenderColor(Color);
}
}
AVFXTestOrigin::AVFXTestOrigin()
{
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Origin")));
    AddCube(this,TEXT("OriginMarker"),RootComponent,FVector(0,0,-95),FVector(.35,.35,.1));
    AddLabel(this,TEXT("OriginLabel"),TEXT("ORIGIN"),FVector(0,0,-70),FColor::Cyan);
}
AVFXTestTarget::AVFXTestTarget()
{
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Target")));
    AddCube(this,TEXT("Dummy"),RootComponent,FVector(0,0,-10),FVector(.5,.5,1.6));
    AddLabel(this,TEXT("TargetLabel"),TEXT("TARGET"),FVector(0,0,110),FColor::Yellow);
}
AVFXTestCharacter::AVFXTestCharacter()
{
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("root")));
    AddCube(this,TEXT("Torso"),RootComponent,FVector(0,0,100),FVector(.45,.65,.8));
    AddCube(this,TEXT("HeadProxy"),RootComponent,FVector(0,0,165),FVector(.35,.35,.4));
    const TArray<FName> Names = {TEXT("pelvis"),TEXT("spine"),TEXT("head"),TEXT("hand_l"),TEXT("hand_r"),TEXT("foot_l"),TEXT("foot_r"),TEXT("weapon")};
    const TArray<FVector> Positions = {FVector(0,0,80),FVector(0,0,120),FVector(0,0,165),FVector(0,-65,120),FVector(0,65,120),FVector(0,-20,8),FVector(0,20,8),FVector(0,90,120)};
    for (int32 I=0; I<Names.Num(); ++I)
    {
        USceneComponent* Anchor = CreateDefaultSubobject<USceneComponent>(Names[I]);
        Anchor->SetupAttachment(RootComponent); Anchor->SetRelativeLocation(Positions[I]); Anchors.Add(Anchor);
    }
    AddCube(this,TEXT("WeaponProxy"),GetAnchor(TEXT("weapon")),FVector(0,0,30),FVector(.07,.07,.9));
    AddLabel(this,TEXT("RigLabel"),TEXT("PROCEDURAL ATTACHMENT RIG"),FVector(0,0,220),FColor::Cyan);
}
USceneComponent* AVFXTestCharacter::GetAnchor(FName Name) const
{
    for (USceneComponent* Anchor : Anchors) if (Anchor && Anchor->GetFName() == Name) return Anchor;
    return GetRootComponent();
}
AVFXShowcaseEnvironment::AVFXShowcaseEnvironment()
{
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("StageRoot")));
    Camera=CreateDefaultSubobject<UCameraComponent>(TEXT("PreviewCamera")); Camera->SetupAttachment(RootComponent);
    Camera->FieldOfView=65;
    Camera->PostProcessSettings.bOverride_AutoExposureMinBrightness=true;
    Camera->PostProcessSettings.bOverride_AutoExposureMaxBrightness=true;
    Camera->PostProcessSettings.AutoExposureMinBrightness=1;
    Camera->PostProcessSettings.AutoExposureMaxBrightness=1;
    Camera->PostProcessBlendWeight=1;
    ScreenAnchor=CreateDefaultSubobject<USceneComponent>(TEXT("ScreenPreviewLayer")); ScreenAnchor->SetupAttachment(Camera);
    ScreenAnchor->SetRelativeLocation(FVector(180,0,0));
    Light=CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("KeyLight")); Light->SetupAttachment(RootComponent); Light->SetRelativeRotation(FRotator(-50,-40,0)); Light->SetIntensity(4);
    Floor=AddCube(this,TEXT("Ground"),RootComponent,FVector(1200,2500,-15),FVector(100,100,.2));
    SurfaceWall=AddCube(this,TEXT("SurfaceWall"),RootComponent,FVector(1200,800,120),FVector(.1,5,2.4));
    const int32 Radii[] = {100,200,500,1000};
    for (int32 Radius : Radii)
    {
        for (int32 Side=0; Side<4; ++Side)
        {
            FVector Pos(1000,0,1); FVector Scale;
            if(Side<2){Pos.X += (Side==0?-Radius:Radius); Scale=FVector(.02,Radius*.02,.01);}
            else {Pos.Y += (Side==2?-Radius:Radius); Scale=FVector(Radius*.02,.02,.01);}
            AddCube(this,*FString::Printf(TEXT("Grid%d_%d"),Radius,Side),RootComponent,Pos,Scale);
        }
        AddLabel(this,*FString::Printf(TEXT("GridLabel%d"),Radius),FString::Printf(TEXT("%dm"),Radius/100),FVector(1000+Radius,-Radius,8),FColor::Green);
    }
    AddLabel(this,TEXT("StageTitle"),TEXT("SHOWCASE | 100 cm = 1 m"),FVector(200,-550,30));
    AddLabel(this,TEXT("StressTitle"),TEXT("STRESS TEST ZONE"),FVector(200,3800,30),FColor::Red);
    AddLabel(this,TEXT("EnvironmentTitle"),TEXT("ENVIRONMENT ZONE"),FVector(2500,0,30));
    AddLabel(this,TEXT("NearDistance"),TEXT("NEAR 2.5 m"),FVector(250,-250,10),FColor::Cyan);
    AddLabel(this,TEXT("MediumDistance"),TEXT("MEDIUM 10 m"),FVector(1000,-350,10),FColor::Cyan);
    AddLabel(this,TEXT("FarDistance"),TEXT("FAR 40 m"),FVector(4000,-350,10),FColor::Cyan);
    for(int32 I=0;I<15;++I)
    {
        UStaticMeshComponent* Mesh=AddCube(this,*FString::Printf(TEXT("ComplexProp%d"),I),RootComponent,FVector(400+I%5*350,900+I/5*250,70+I%3*40),FVector(1+I%2,.5,1.5+I%3));
        Mesh->SetVisibility(false); ComplexMeshes.Add(Mesh);
    }
    UpdateCamera();
}
void AVFXShowcaseEnvironment::BeginPlay()
{
    Super::BeginPlay();
    Origin=GetWorld()->SpawnActor<AVFXTestOrigin>(GetOriginPoint(),FRotator::ZeroRotator);
    Target=GetWorld()->SpawnActor<AVFXTestTarget>(GetTargetPoint(1000),FRotator::ZeroRotator);
    ProjectileTarget=GetWorld()->SpawnActor<AVFXTestProjectileTarget>(GetTargetPoint(2000),FRotator::ZeroRotator);
    Character=GetWorld()->SpawnActor<AVFXTestCharacter>(GetActorLocation()+FVector(0,-250,0),FRotator::ZeroRotator);
    for(AActor* Actor : {static_cast<AActor*>(Origin),static_cast<AActor*>(Target),static_cast<AActor*>(ProjectileTarget),static_cast<AActor*>(Character)})
        if(Actor){Actor->SetOwner(this); Actor->AttachToActor(this,FAttachmentTransformRules::KeepWorldTransform); SpawnedActors.Add(Actor);}
    if(APlayerController* PC=UGameplayStatics::GetPlayerController(this,0)) PC->SetViewTarget(this);
}
void AVFXShowcaseEnvironment::EndPlay(const EEndPlayReason::Type Reason)
{
    for(AActor* Actor:SpawnedActors)if(IsValid(Actor))Actor->Destroy();SpawnedActors.Reset();
    Super::EndPlay(Reason);
}
void AVFXShowcaseEnvironment::SetBackground(EVFXShowcaseBackground Value)
{
    const bool bBright=Value==EVFXShowcaseBackground::Bright;
    Light->SetIntensity(Value==EVFXShowcaseBackground::Dark?.12f:bBright?10.f:4.f);
    Light->SetLightColor(bBright?FLinearColor(1.f,.96f,.85f):Value==EVFXShowcaseBackground::Dark?FLinearColor(.25f,.35f,.6f):FLinearColor::White);
    for(UStaticMeshComponent* Mesh:ComplexMeshes) Mesh->SetVisibility(Value==EVFXShowcaseBackground::Complex);
}
void AVFXShowcaseEnvironment::SetCameraDistance(float DistanceCm){CameraDistance=FMath::Clamp(DistanceCm,250.f,5000.f);UpdateCamera();}
void AVFXShowcaseEnvironment::FocusStress(bool bStress){bStressCamera=bStress;bEnvironmentCamera=false;UpdateCamera();}
void AVFXShowcaseEnvironment::FocusEnvironment(){bStressCamera=false;bEnvironmentCamera=true;UpdateCamera();}
void AVFXShowcaseEnvironment::UpdateCamera()
{
    const FVector Focus=bStressCamera?FVector(1000,5000,100):bEnvironmentCamera?FVector(2800,0,100):FVector(800,0,100);
    const FVector Pos=Focus+FVector(-CameraDistance*.55,-CameraDistance,CameraDistance*.55);
    Camera->SetRelativeLocation(Pos);Camera->SetRelativeRotation((Focus-Pos).Rotation());
}
void AVFXShowcaseEnvironment::SetSurface(FName Surface)
{
    const bool bGround=Surface==TEXT("Ground");
    bSurfaceIsGround=bGround;
    SurfaceWall->SetRelativeLocation(bGround?FVector(1000,0,0):FVector(1200,800,120));
    SurfaceWall->SetRelativeScale3D(bGround?FVector(5,5,.1):FVector(.1,5,2.4));
    // Distinct orientation/roughness test geometry; these are proxy surfaces, not physical-material certification.
    SurfaceWall->SetRelativeRotation(FRotator(0,Surface==TEXT("Wood")?25:Surface==TEXT("Metal")?-25:0,0));
}
FTransform AVFXShowcaseEnvironment::GetSurfaceTransform() const
{
    if(!SurfaceWall||!SurfaceWall->GetStaticMesh()) return GetActorTransform();
    const FVector Extent=SurfaceWall->GetStaticMesh()->GetBounds().BoxExtent;
    const FVector LocalNormal=bSurfaceIsGround?FVector::UpVector:-FVector::ForwardVector;
    const FVector LocalFace=bSurfaceIsGround?FVector(0,0,Extent.Z):FVector(-Extent.X,0,0);
    const FTransform MeshTransform=SurfaceWall->GetComponentTransform();
    const FVector Normal=MeshTransform.TransformVectorNoScale(LocalNormal).GetSafeNormal();
    return FTransform(FRotationMatrix::MakeFromZ(Normal).ToQuat(),MeshTransform.TransformPosition(LocalFace)+Normal*.5f);
}
FVector AVFXShowcaseEnvironment::GetOriginPoint() const{return GetActorLocation()+FVector(0,0,100);}
FVector AVFXShowcaseEnvironment::GetTargetPoint(float DistanceCm) const{return GetOriginPoint()+FVector(DistanceCm,0,0);}
FVector AVFXShowcaseEnvironment::GetGroundPoint() const{return GetActorLocation()+FVector(1000,0,5);}
FVector AVFXShowcaseEnvironment::GetStressPoint() const{return GetActorLocation()+FVector(0,4500,10);}
FVector AVFXShowcaseEnvironment::GetEnvironmentPoint() const{return GetActorLocation()+FVector(2800,0,100);}
