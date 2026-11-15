#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Stage/TDCStageTypes.h"
#include "TDCSetPiece.generated.h"

class UStaticMeshComponent;
class UTDCLightReactiveComponent;
class UTDCTimelineComponent;
class UMaterialInstanceDynamic;

/**
 * Pieza de escenografía greybox: un mesh (cubo por defecto) que puede depender de la luz y/o estar "en toma".
 * Colores de lectura: gris = fija, amarillo = solo con luz, violeta = solo a oscuras, celeste = en toma (rebobinable).
 */
UCLASS()
class THEDIRECTORSCUT_API ATDCSetPiece : public AActor
{
	GENERATED_BODY()

public:
	ATDCSetPiece();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TDC|SetPiece")
	ETDCLightPresence Presence = ETDCLightPresence::Always;

	/** Si está en toma, el director la puede rebobinar. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TDC|SetPiece")
	bool bRecordTime = false;

	/** Simular física desde el inicio (si no, un ATDCFallTrigger la puede soltar). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TDC|SetPiece")
	bool bStartsSimulating = false;

	/** Sobrescribe el color de lectura (alpha 0 = usar el automático). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TDC|SetPiece")
	FLinearColor ColorOverride = FLinearColor(0.f, 0.f, 0.f, 0.f);

	UFUNCTION(BlueprintCallable, Category = "TDC|SetPiece")
	void Release();

	UStaticMeshComponent* GetMesh() const { return Mesh; }

	UTDCLightReactiveComponent* GetLightReactive() const { return LightReactive; }

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TDC|SetPiece")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TDC|SetPiece")
	TObjectPtr<UTDCLightReactiveComponent> LightReactive;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TDC|SetPiece")
	TObjectPtr<UTDCTimelineComponent> Timeline;

private:
	FLinearColor ReadColor() const;
	void ApplyPattern();
	/** Material, color y patrón. Las piezas de luz/oscuridad usan M_TDCLightPiece (translúcido, "de luz") si existe. */
	void SetupMaterial();
	bool WantsLightMaterial() const;
	UFUNCTION()
	void HandlePresenceChanged(bool bNowPresent);

	bool bLightMaterial = false;
	/** Si cambia la luz, despertar la física: lo que estaba apoyado en una pieza que desapareció tiene que caer. */
	void OnLightingChanged();

	FDelegateHandle LightingHandle;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> Material;
};
