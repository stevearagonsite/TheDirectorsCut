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

	/** Carcasa del reflector colgante (T-0036): /Game/Dressing/Props/SM_StageLight_Hanging si existe; si no, solo la esfera. */
	UPROPERTY(VisibleAnywhere, Category = "TDC|Light")
	TObjectPtr<UStaticMeshComponent> Housing;

	/** Offset de la carcasa respecto del bulbo, en cm de mundo. El pivot del modelo está abajo (90 cm de alto): cuelga hacia arriba. */
	UPROPERTY(EditAnywhere, Category = "TDC|Light")
	FVector HousingOffset = FVector(0.f, 0.f, -20.f);

	/** Los FBX de Rodin miran a -Y; 180 los gira hacia la cámara (+Y). Ajustar si la lente queda de espaldas. */
	UPROPERTY(EditAnywhere, Category = "TDC|Light")
	FRotator HousingRotation = FRotator(0.f, 180.f, 0.f);

	/** Con carcasa, el bulbo emisivo se achica para quedar en la lente. */
	UPROPERTY(EditAnywhere, Category = "TDC|Light", meta = (ClampMin = "0.05"))
	float HangingBulbScale = 0.18f;

	/** Cono de luz pintado (T-0054): quad aditivo desde la lente hasta el plano de juego, ancho = alcance. Muestra qué
	 * ilumina el reflector. Necesita /Game/Materials/M_TDCLightCone (make_materials.py); sin él no se dibuja. */
	UPROPERTY(VisibleAnywhere, Category = "TDC|Light")
	TObjectPtr<UStaticMeshComponent> Cone;

	/** Z del plano de juego donde "pega" el cono. */
	UPROPERTY(EditAnywhere, Category = "TDC|Light")
	float PlayPlaneZ = 0.f;

	UPROPERTY(EditAnywhere, Category = "TDC|Light", meta = (ClampMin = "0"))
	float ConeIntensity = 1.0f; // T-0055: con 1.8 el haz tapaba el skyline detrás

private:
	void ApplyVisuals();
	void SetupHousing();
	void SetupCone();

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ConeMaterial;
	bool bConeReady = false;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BulbMaterial;

	bool bOn = true;
};
