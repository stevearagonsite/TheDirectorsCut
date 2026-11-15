#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TDCStageTriggers.generated.h"

class UBoxComponent;
class ATDCSetPiece;

/** Cuando el jugador entra, suelta (activa física) las piezas indicadas. Se dispara una vez. */
UCLASS()
class THEDIRECTORSCUT_API ATDCFallTrigger : public AActor
{
	GENERATED_BODY()

public:
	ATDCFallTrigger();

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "TDC|Trigger")
	TArray<TObjectPtr<ATDCSetPiece>> Targets;

protected:
	UPROPERTY(VisibleAnywhere, Category = "TDC|Trigger")
	TObjectPtr<UBoxComponent> Box;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	bool bFired = false;
};

/** Si el jugador entra (cayó a un pozo), vuelve a su último punto seguro. */
UCLASS()
class THEDIRECTORSCUT_API ATDCRespawnVolume : public AActor
{
	GENERATED_BODY()

public:
	ATDCRespawnVolume();

protected:
	UPROPERTY(VisibleAnywhere, Category = "TDC|Trigger")
	TObjectPtr<UBoxComponent> Box;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
