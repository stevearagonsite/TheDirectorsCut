#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Stage/TDCStageTypes.h"
#include "TDCStageLight.generated.h"

class UPointLightComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;

/** Reflector del set. El director lo prende/apaga; las piezas reactivas cambian según su alcance (Radius). */
UCLASS()
class THEDIRECTORSCUT_API ATDCStageLight : public AActor
{
	GENERATED_BODY()

public:
	ATDCStageLight();

	UFUNCTION(BlueprintCallable, Category = "TDC|Light")
	void SetOn(bool bNewOn);

	UFUNCTION(BlueprintCallable, Category = "TDC|Light")
	void Toggle() { SetOn(!bOn); }

	UFUNCTION(BlueprintPure, Category = "TDC|Light")
	bool IsOn() const { return bOn; }

	FTDCLightSample GetSample() const;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Alcance de la luz sobre las piezas reactivas (cm). También es el radio de atenuación visual. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TDC|Light", meta = (ClampMin = "50"))
	float Radius = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TDC|Light")
	bool bStartsOn = true;

	UPROPERTY(EditAnywhere, Category = "TDC|Light")
	float Intensity = 40.f; // candelas (unidad por defecto de PointLight en UE5)

	UPROPERTY(VisibleAnywhere, Category = "TDC|Light")
	TObjectPtr<UStaticMeshComponent> Bulb;

	UPROPERTY(VisibleAnywhere, Category = "TDC|Light")
	TObjectPtr<UPointLightComponent> Light;

private:
	void ApplyVisuals();

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BulbMaterial;

	bool bOn = true;
};
