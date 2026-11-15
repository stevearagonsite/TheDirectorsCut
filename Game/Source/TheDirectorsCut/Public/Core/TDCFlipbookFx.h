#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TDCFlipbookFx.generated.h"

class UStaticMeshComponent;
class USceneComponent;

/** Datos de un sprite sheet de VFX (pixelfx) importado por Tools/unreal/import_vfx.py como /Game/VFX/T_<Name>. */
struct FTDCFlipbook
{
	const TCHAR* Name;   // T_<Name> en /Game/VFX
	int32 Cols;
	int32 Rows;
	int32 Frames;
	float Fps;
	bool bLoop;
	float Aspect;        // ancho / alto de un frame
};

// Forma clásica (no `TDC::Fx`): más segura con UHT.
namespace TDC
{
namespace Fx
{
	inline const FTDCFlipbook ClapImpact     { TEXT("claquetazo_impact"), 3, 4, 10, 30.f, false, 128.f / 80.f };
	inline const FTDCFlipbook ExtraDefeat    { TEXT("extra_defeat"),      7, 7, 46, 30.f, false, 112.f / 108.f };
	inline const FTDCFlipbook ExtraStun      { TEXT("extra_stun"),        8, 8, 60, 30.f, true,  1.f };
	inline const FTDCFlipbook ScriptedBounce { TEXT("scripted_bounce"),   5, 5, 25, 30.f, false, 128.f / 112.f };
	inline const FTDCFlipbook LightToggle    { TEXT("light_toggle"),      7, 6, 37, 30.f, false, 1.f };
	inline const FTDCFlipbook Rewind         { TEXT("rewind"),            7, 8, 54, 30.f, true,  96.f / 72.f };
}
}

/**
 * VFX de sprite sheet: un quad (cara +Y de un cubo fino, hacia la cámara lateral) con M_TDCFlipbook.
 * Los one-shots se destruyen solos; los loops los destruye quien los creó. Si faltan los assets, no hace nada.
 */
UCLASS()
class THEDIRECTORSCUT_API ATDCFlipbookFx : public AActor
{
	GENERATED_BODY()

public:
	ATDCFlipbookFx();

	/** WidthCm = ancho en el mundo; bFlipX espeja (golpes hacia -X). Devuelve nullptr si no están los assets. */
	static ATDCFlipbookFx* Spawn(UWorld* World, const FTDCFlipbook& Fx, const FVector& Location, float WidthCm,
		bool bFlipX = false, USceneComponent* AttachTo = nullptr);

protected:
	UPROPERTY(VisibleAnywhere, Category = "TDC|Fx")
	TObjectPtr<UStaticMeshComponent> Quad;
};
