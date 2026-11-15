#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Stage/TDCStageTypes.h"
#include "TDCLightReactiveComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTDCOnPresenceChanged, bool, bPresent);

/** Hace que el actor exista (visible + colisión) solo con luz o solo a oscuras. */
UCLASS(ClassGroup = (TDC), meta = (BlueprintSpawnableComponent))
class THEDIRECTORSCUT_API UTDCLightReactiveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTDCLightReactiveComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TDC|Light")
	ETDCLightPresence Presence = ETDCLightPresence::Always;

	/** Ocultar el actor cuando está ausente. Las piezas "de luz" lo apagan: quedan como fantasma tenue sin colisión. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TDC|Light")
	bool bHideWhenAbsent = true;

	UPROPERTY(BlueprintAssignable, Category = "TDC|Light")
	FTDCOnPresenceChanged OnPresenceChanged;

	UFUNCTION(BlueprintPure, Category = "TDC|Light")
	bool IsPresent() const { return bPresent; }

	/** Recalcula con las luces actuales. */
	UFUNCTION(BlueprintCallable, Category = "TDC|Light")
	void Evaluate();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void ApplyPresence(bool bNewPresent);

	FDelegateHandle LightingHandle;
	bool bPresent = true;
	bool bApplied = false;
};
