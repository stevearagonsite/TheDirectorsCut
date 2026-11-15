#pragma once

#include "CoreMinimal.h"
#include "TDCCombatTypes.generated.h"

/** Cómo reacciona un enemigo al claquetazo (D-003). */
UENUM(BlueprintType)
enum class ETDCVulnerability : uint8
{
	Common    UMETA(DisplayName = "Común (el claquetazo lo derriba)"),
	Resistant UMETA(DisplayName = "Resistente (el claquetazo lo aturde)"),
	Scripted  UMETA(DisplayName = "De guion (inmune: solo poderes)"),
};

UENUM(BlueprintType)
enum class ETDCClapResult : uint8
{
	Defeated,
	Stunned,
	Immune,
};

namespace TDC
{
	/** Regla del claquetazo según la vulnerabilidad del enemigo. */
	inline ETDCClapResult ResolveClap(ETDCVulnerability Vulnerability)
	{
		switch (Vulnerability)
		{
		case ETDCVulnerability::Common:    return ETDCClapResult::Defeated;
		case ETDCVulnerability::Resistant: return ETDCClapResult::Stunned;
		default:                           return ETDCClapResult::Immune;
		}
	}
}
