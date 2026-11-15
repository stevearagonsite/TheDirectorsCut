#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/TDCCombatTypes.h"
#include "TDCDirectorComponent.generated.h"

class ATDCStageLight;

/** Poderes del director. Cada uno se desbloquea por separado (gating metroidvania). */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ETDCPower : uint8
{
	None   = 0,
	Lights = 1 << 0,
	Rewind = 1 << 1,
	Cut    = 1 << 2,
	Set    = 1 << 3,
};
ENUM_CLASS_FLAGS(ETDCPower);

/** Lo que el jugador puede "dirigir". Vive en el personaje. */
UCLASS(ClassGroup = (TDC), meta = (BlueprintSpawnableComponent))
class THEDIRECTORSCUT_API UTDCDirectorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTDCDirectorComponent();

	/** Distancia máxima a la que el director alcanza un reflector. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TDC|Director")
	float LightReach = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TDC|Director", meta = (Bitmask, BitmaskEnum = "/Script/TheDirectorsCut.ETDCPower"))
	int32 UnlockedPowers = static_cast<int32>(ETDCPower::Lights | ETDCPower::Rewind);

	UFUNCTION(BlueprintPure, Category = "TDC|Director")
	bool HasPower(ETDCPower Power) const { return (UnlockedPowers & static_cast<int32>(Power)) != 0; }

	UFUNCTION(BlueprintCallable, Category = "TDC|Director")
	void UnlockPower(ETDCPower Power) { UnlockedPowers |= static_cast<int32>(Power); }

	/** "¡Luces!": prende/apaga el reflector más cercano al alcance. */
	UFUNCTION(BlueprintCallable, Category = "TDC|Director")
	void ToggleNearestLight();

	/** Reflector que se tocaría ahora (para resaltarlo en UI). */
	UFUNCTION(BlueprintPure, Category = "TDC|Director")
	ATDCStageLight* FindNearestLight() const;

	/** "Rebobinar la toma": mientras se mantiene, todo lo que está en toma vuelve atrás. */
	UFUNCTION(BlueprintCallable, Category = "TDC|Director")
	void BeginRewind();

	UFUNCTION(BlueprintCallable, Category = "TDC|Director")
	void EndRewind();

	UFUNCTION(BlueprintPure, Category = "TDC|Director")
	bool IsRewinding() const { return bRewinding; }

	/** "¡Claquetazo!": golpe básico con la claqueta hacia donde mira el director. Devuelve true si se ejecutó. */
	UFUNCTION(BlueprintCallable, Category = "TDC|Director")
	bool Clap();

	/** Alcance del claquetazo hacia adelante (cm) y radio del golpe (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TDC|Director")
	float ClapReach = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TDC|Director")
	float ClapRadius = 75.f;

	/** Segundos entre claquetazos. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TDC|Director")
	float ClapCooldown = 0.45f;

private:
	bool bRewinding = false;
	TWeakObjectPtr<AActor> RewindFx;
	double LastClapTime = -1000.0;
};
