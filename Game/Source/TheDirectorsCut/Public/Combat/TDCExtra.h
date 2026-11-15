#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Combat/TDCCombatTypes.h"
#include "Stage/TDCStageTypes.h"
#include "TDCExtra.generated.h"

class UCapsuleComponent;
class USkeletalMeshComponent;
class UAnimSequence;
class UStaticMeshComponent;
class UTDCLightReactiveComponent;
class UMaterialInstanceDynamic;

/**
 * "Extra" fuera de guion: enemigo greybox que patrulla en X y manda al director al último punto seguro si lo toca.
 * El claquetazo lo derriba, lo aturde o rebota según su vulnerabilidad (D-003).
 * Un extra "de guion" se resuelve dirigiendo la escena: si depende de la luz (p. ej. sombra = solo a oscuras),
 * cambiar la iluminación lo saca de la toma para siempre.
 * Colores de lectura: rojo = común, naranja = resistente, negro con franja blanca = de guion.
 */
UCLASS()
class THEDIRECTORSCUT_API ATDCExtra : public AActor
{
	GENERATED_BODY()

public:
	ATDCExtra();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TDC|Extra")
	ETDCVulnerability Vulnerability = ETDCVulnerability::Common;

	/** Si no es Always, el extra solo existe con luz o a oscuras; al desaparecer por un cambio de luz queda derrotado. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TDC|Extra")
	ETDCLightPresence Presence = ETDCLightPresence::Always;

	/** Distancia de patrulla a cada lado del punto inicial (cm, sobre X). 0 = quieto. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TDC|Extra", meta = (ClampMin = "0"))
	float PatrolDistance = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TDC|Extra", meta = (ClampMin = "0"))
	float PatrolSpeed = 150.f;

	/** Segundos que queda aturdido un resistente. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TDC|Extra", meta = (ClampMin = "0"))
	float StunSeconds = 1.5f;

	/** Empuje al recibir un claquetazo (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TDC|Extra", meta = (ClampMin = "0"))
	float KnockbackDistance = 120.f;

	/**
	 * Modelo animado del extra (Rodin), cargado por ruta según la vulnerabilidad:
	 * /Game/Characters/Extras/<Comun|Resistente|Sombra>/SK_Extra_<Tipo> + AS_Extra_<Tipo>_Idle/_Walk.
	 * Si no existe, queda el greybox. PuppetScale compensa la escala 0.01 del root en las animaciones de Rodin.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TDC|Extra")
	float PuppetScale = 1.f;

	/** Movimiento procedural al caminar (rebote + balanceo), encima del clip: se ve vivo aunque el clip venga quieto. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TDC|Extra", meta = (ClampMin = "0"))
	float WalkBobCm = 7.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TDC|Extra", meta = (ClampMin = "0"))
	float WalkSwayDegrees = 5.f; // T-0019: con los FBX de Rodin en cm, 1 = tamaño humano (100 los dejaba gigantes)

	/** Recibe un claquetazo desde una posición. Devuelve el resultado para el feedback del director. */
	UFUNCTION(BlueprintCallable, Category = "TDC|Extra")
	ETDCClapResult ReceiveClap(const FVector& FromLocation);

	UFUNCTION(BlueprintPure, Category = "TDC|Extra")
	bool IsDefeated() const { return bDefeated; }

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "TDC|Extra")
	TObjectPtr<UCapsuleComponent> Capsule;

	UPROPERTY(VisibleAnywhere, Category = "TDC|Extra")
	TObjectPtr<UStaticMeshComponent> Body;

	/** Marca de "extra de guion" (franja blanca tipo claqueta). */
	UPROPERTY(VisibleAnywhere, Category = "TDC|Extra")
	TObjectPtr<UStaticMeshComponent> ScriptMark;

	UPROPERTY(VisibleAnywhere, Category = "TDC|Extra")
	TObjectPtr<UTDCLightReactiveComponent> LightReactive;

	UPROPERTY(VisibleAnywhere, Category = "TDC|Extra")
	TObjectPtr<USkeletalMeshComponent> Puppet;

private:
	UFUNCTION()
	void OnTouch(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnPresenceChanged(bool bPresent);

	void Defeat();
	void ApplyColor();
	FLinearColor ReadColor() const;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> Material;

	FVector Origin = FVector::ZeroVector;
	float Direction = 1.f;
	float StunRemaining = 0.f;
	bool bDefeated = false;
	void LoadPuppet();
	void UpdatePuppet();
	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> PuppetIdle;
	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> PuppetWalk;
	bool bPuppetWalking = false;
	float PuppetFacing = 0.f;
	float BobPhase = 0.f;
	TWeakObjectPtr<AActor> StunFx;
};
