#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VFXShowcaseTypes.h"
#include "VFXShowcaseEnvironment.generated.h"
class UCameraComponent;
class UDirectionalLightComponent;
class UStaticMeshComponent;
class UTextRenderComponent;
class UWidgetComponent;

/** A portable procedural proxy, with named attachment components rather than project skeletal assets. */
UCLASS(Blueprintable)
class MODULARGAMEVFXSHOWCASE_API AVFXTestCharacter : public AActor
{
    GENERATED_BODY()
public:
    AVFXTestCharacter();
    UFUNCTION(BlueprintPure, Category="Showcase") USceneComponent* GetAnchor(FName Name) const;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Showcase") TArray<TObjectPtr<USceneComponent>> Anchors;
};
UCLASS(Blueprintable)
class MODULARGAMEVFXSHOWCASE_API AVFXTestOrigin : public AActor
{
    GENERATED_BODY()
public: AVFXTestOrigin();
};
UCLASS(Blueprintable)
class MODULARGAMEVFXSHOWCASE_API AVFXTestTarget : public AActor
{
    GENERATED_BODY()
public: AVFXTestTarget();
};
UCLASS(Blueprintable)
class MODULARGAMEVFXSHOWCASE_API AVFXTestProjectileTarget : public AVFXTestTarget
{
    GENERATED_BODY()
};
UCLASS(Blueprintable)
class MODULARGAMEVFXSHOWCASE_API AVFXShowcaseEnvironment : public AActor
{
    GENERATED_BODY()
public:
    AVFXShowcaseEnvironment();
    virtual void Tick(float DeltaSeconds) override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Showcase") TObjectPtr<UCameraComponent> Camera;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Showcase") TObjectPtr<USceneComponent> ScreenAnchor;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Showcase") TObjectPtr<UDirectionalLightComponent> Light;
    UPROPERTY(BlueprintReadOnly, Category="Showcase") TObjectPtr<AVFXTestCharacter> Character;
    UPROPERTY(BlueprintReadOnly, Category="Showcase") TObjectPtr<AVFXTestOrigin> Origin;
    UPROPERTY(BlueprintReadOnly, Category="Showcase") TObjectPtr<AVFXTestTarget> Target;
    UPROPERTY(BlueprintReadOnly, Category="Showcase") TObjectPtr<AVFXTestProjectileTarget> ProjectileTarget;
    UFUNCTION(BlueprintCallable, Category="Showcase") void SetBackground(EVFXShowcaseBackground Value);
    UFUNCTION(BlueprintCallable, Category="Showcase") void SetCameraDistance(float DistanceCm);
    UFUNCTION(BlueprintCallable, Category="Showcase") void FocusStress(bool bStress);
    void FrameStressGrid(int32 Columns, int32 Rows, float Spacing);
    UFUNCTION(BlueprintCallable, Category="Showcase") void FocusEnvironment();
    UFUNCTION(BlueprintCallable, Category="Showcase") void SetSurface(FName Surface);
    /** Local +Z is the selected visible surface normal; location lies on its face. */
    UFUNCTION(BlueprintPure, Category="Showcase") FTransform GetSurfaceTransform() const;
    FVector GetOriginPoint() const;
    FVector GetTargetPoint(float DistanceCm) const;
    FVector GetGroundPoint() const;
    FVector GetStressPoint() const;
    FVector GetEnvironmentPoint() const;
private:
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Floor;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> SurfaceWall;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> ComplexMeshes;
    UPROPERTY() TArray<TObjectPtr<AActor>> SpawnedActors;
    UPROPERTY(Transient) TArray<TObjectPtr<UWidgetComponent>> HUDLabels;
    bool bStressCamera = false;
    bool bEnvironmentCamera = false;
    float CameraDistance = 1600;
    float StressCameraDistance = 2400;
    FVector StressFocus = FVector(1000,5000,100);
    bool bSurfaceIsGround = false;
    void UpdateCamera();
};
