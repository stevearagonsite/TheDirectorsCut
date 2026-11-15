#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Stage/TDCStageTypes.h"
#include "TDCTimelineComponent.generated.h"

class UPrimitiveComponent;

/**
 * Pone al actor "en toma": graba su transform mientras se mueve y lo rebobina cuando el director rebobina.
 * Solo graba cuando hay movimiento, así los períodos quietos no gastan buffer.
 * Rebobinar hasta el primer frame deja la pieza como al inicio de la toma (incluida la física apagada).
 */
UCLASS(ClassGroup = (TDC), meta = (BlueprintSpawnableComponent))
class THEDIRECTORSCUT_API UTDCTimelineComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTDCTimelineComponent();

	/** Segundos de movimiento que se pueden rebobinar. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TDC|Time", meta = (ClampMin = "1"))
	float RecordSeconds = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TDC|Time", meta = (ClampMin = "5"))
	float SamplesPerSecond = 30.f;

	/** Velocidad del rebobinado respecto de la grabación (2 = el doble de rápido). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TDC|Time", meta = (ClampMin = "0.1"))
	float RewindSpeed = 1.5f;

	UFUNCTION(BlueprintPure, Category = "TDC|Time")
	float GetRecordedSeconds() const { return History.Num() / SamplesPerSecond; }

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void OnTimeModeChanged(ETDCTimeMode NewMode);
	FTDCTimeFrame Capture() const;
	void Apply(const FTDCTimeFrame& Frame);
	UPrimitiveComponent* GetBody() const;

	FTDCTimeHistory History;
	FTDCTimeFrame CurrentFrame;
	FDelegateHandle TimeModeHandle;
	ETDCTimeMode Mode = ETDCTimeMode::Play;
	float Accumulator = 0.f;
	bool bHasCurrentFrame = false;
};
