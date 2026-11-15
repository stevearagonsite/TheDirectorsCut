#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Stage/TDCStageTypes.h"
#include "TDCStageSubsystem.generated.h"

class ATDCStageLight;

DECLARE_MULTICAST_DELEGATE(FTDCOnLightingChanged);
DECLARE_MULTICAST_DELEGATE_OneParam(FTDCOnTimeModeChanged, ETDCTimeMode);

/**
 * Estado global del set (la "escena" que dirige el jugador): luces registradas y modo del tiempo.
 * Las piezas escuchan sus eventos; el director los dispara.
 */
UCLASS()
class THEDIRECTORSCUT_API UTDCStageSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void RegisterLight(ATDCStageLight* Light);
	void UnregisterLight(ATDCStageLight* Light);

	/** Lo llaman las luces cuando cambian; avisa a todas las piezas reactivas. */
	void NotifyLightingChanged();

	void GatherLightSamples(TArray<FTDCLightSample>& OutSamples) const;
	const TArray<TWeakObjectPtr<ATDCStageLight>>& GetLights() const { return Lights; }

	UFUNCTION(BlueprintCallable, Category = "TDC|Stage")
	void SetTimeMode(ETDCTimeMode NewMode);

	UFUNCTION(BlueprintPure, Category = "TDC|Stage")
	ETDCTimeMode GetTimeMode() const { return TimeMode; }

	FTDCOnLightingChanged OnLightingChanged;
	FTDCOnTimeModeChanged OnTimeModeChanged;

private:
	TArray<TWeakObjectPtr<ATDCStageLight>> Lights;
	ETDCTimeMode TimeMode = ETDCTimeMode::Play;
};
