#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FireMageGameplay.generated.h"
class AFMFrostMonster; class UNiagaraSystem; class UNiagaraComponent; class UPointLightComponent;

UCLASS()
class MODULARGAMEVFXDEMO_API AFMFireProjectile : public AActor {
 GENERATED_BODY()
public:
 AFMFireProjectile(); virtual void BeginPlay() override; virtual void Tick(float Delta) override;
 UPROPERTY(BlueprintReadWrite) TObjectPtr<AFMFrostMonster> Target;
 UPROPERTY(BlueprintReadWrite) float Damage=30.f;
 UPROPERTY(BlueprintReadWrite) float Speed=1450.f;
private:
 UPROPERTY() UNiagaraComponent* Flames;
 UPROPERTY() UNiagaraSystem* Impact;
 UPROPERTY() UPointLightComponent* FireLight;
 bool bHit=false;
 void Explode(const FVector& Location,AFMFrostMonster* Victim);
};

UCLASS()
class MODULARGAMEVFXDEMO_API AFMFireArea : public AActor {
 GENERATED_BODY()
public:
 AFMFireArea(); virtual void BeginPlay() override; virtual void Tick(float Delta) override;
 UPROPERTY(BlueprintReadOnly) float Radius=300.f;
 UPROPERTY(BlueprintReadOnly) float Age=0.f;
private:
 UPROPERTY() UNiagaraComponent* Flames;
 UPROPERTY() UPointLightComponent* FireLight;
 int32 BurnTicks=0;
 void DamageOccupants(float Damage);
};
