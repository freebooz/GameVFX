#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArcaneMageGameplay.generated.h"
class AFMFrostMonster;class UNiagaraComponent;class UNiagaraSystem;
UCLASS()
class MODULARGAMEVFXDEMO_API AFMArcaneMissile : public AActor {
 GENERATED_BODY()
public:
 AFMArcaneMissile();virtual void BeginPlay() override;virtual void Tick(float Delta) override;
 UPROPERTY(BlueprintReadWrite) TObjectPtr<AFMFrostMonster> Target;
 UPROPERTY(BlueprintReadWrite) int32 VolleyIndex=0;
 UPROPERTY(BlueprintReadWrite) float Damage=9.f;
private:
 UPROPERTY() UNiagaraComponent* Energy;
 UPROPERTY() UNiagaraSystem* Impact;
 FVector Start,ControlA,LastPosition;
 float Age=0,FlightDuration=.7f;
 bool bImpacted=false;
 void ImpactAt(const FVector& Location,AFMFrostMonster* Victim);
};
