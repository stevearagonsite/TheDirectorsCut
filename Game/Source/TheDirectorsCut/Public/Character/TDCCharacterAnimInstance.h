#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TDCCharacterAnimInstance.generated.h"

/**
 * Base del AnimBP del director. Expone lo que necesita la máquina de estados (idle/run/jump),
 * así el AnimBP del editor no tiene lógica: solo transiciones por Speed y bIsInAir.
 */
UCLASS()
class THEDIRECTORSCUT_API UTDCCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	/** Velocidad horizontal (cm/s). */
	UPROPERTY(BlueprintReadOnly, Category = "TDC|Anim")
	float Speed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "TDC|Anim")
	bool bIsInAir = false;

	/** Velocidad vertical (cm/s): > 0 subiendo, < 0 cayendo. */
	UPROPERTY(BlueprintReadOnly, Category = "TDC|Anim")
	float VerticalSpeed = 0.f;

	/** True mientras el director rebobina (para una pose/gesto de "moviola"). */
	UPROPERTY(BlueprintReadOnly, Category = "TDC|Anim")
	bool bIsRewinding = false;

protected:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
