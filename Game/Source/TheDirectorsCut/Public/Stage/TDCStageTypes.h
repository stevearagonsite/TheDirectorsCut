#pragma once

#include "CoreMinimal.h"
#include "TDCStageTypes.generated.h"

/** Cuándo existe una pieza del set según la iluminación. */
UENUM(BlueprintType)
enum class ETDCLightPresence : uint8
{
	Always      UMETA(DisplayName = "Siempre"),
	OnlyInLight UMETA(DisplayName = "Solo con luz"),
	OnlyInDark  UMETA(DisplayName = "Solo a oscuras"),
};

/** Modo del tiempo de la toma. */
UENUM(BlueprintType)
enum class ETDCTimeMode : uint8
{
	Play,
	Rewind,
};

/** Foto de una luz del set para evaluar iluminación. */
struct FTDCLightSample
{
	FVector Location = FVector::ZeroVector;
	float Radius = 0.f;
	bool bOn = false;
};

/** Un instante grabado de un objeto "en toma". */
struct FTDCTimeFrame
{
	FTransform Transform;
	FVector LinearVelocity = FVector::ZeroVector;
	FVector AngularVelocityDeg = FVector::ZeroVector;
	bool bSimulatingPhysics = false;
};

/** Buffer circular de frames: Push graba, Pop rebobina (LIFO). Si se llena, pisa los más viejos. */
class THEDIRECTORSCUT_API FTDCTimeHistory
{
public:
	explicit FTDCTimeHistory(int32 InCapacity = 300) { SetCapacity(InCapacity); }

	void SetCapacity(int32 InCapacity)
	{
		Capacity = FMath::Max(1, InCapacity);
		Frames.Reset();
		Frames.SetNum(Capacity);
		Head = 0;
		Count = 0;
	}

	void Push(const FTDCTimeFrame& Frame)
	{
		Frames[Head] = Frame;
		Head = (Head + 1) % Capacity;
		Count = FMath::Min(Count + 1, Capacity);
	}

	bool Pop(FTDCTimeFrame& OutFrame)
	{
		if (Count == 0)
		{
			return false;
		}
		Head = (Head - 1 + Capacity) % Capacity;
		OutFrame = Frames[Head];
		--Count;
		return true;
	}

	const FTDCTimeFrame* Last() const
	{
		return Count == 0 ? nullptr : &Frames[(Head - 1 + Capacity) % Capacity];
	}

	int32 Num() const { return Count; }
	int32 GetCapacity() const { return Capacity; }
	void Clear() { Head = 0; Count = 0; }

private:
	TArray<FTDCTimeFrame> Frames;
	int32 Capacity = 1;
	int32 Head = 0;
	int32 Count = 0;
};

namespace TDC
{
	/** ¿Alguna luz encendida alcanza la caja? (distancia de la luz al punto más cercano de la caja <= radio) */
	inline bool IsLit(const FBox& Bounds, TArrayView<const FTDCLightSample> Lights)
	{
		for (const FTDCLightSample& Light : Lights)
		{
			if (Light.bOn && Bounds.ComputeSquaredDistanceToPoint(Light.Location) <= FMath::Square(Light.Radius))
			{
				return true;
			}
		}
		return false;
	}

	/** ¿La pieza existe con esta iluminación? */
	inline bool IsPresent(ETDCLightPresence Presence, bool bLit)
	{
		switch (Presence)
		{
		case ETDCLightPresence::OnlyInLight: return bLit;
		case ETDCLightPresence::OnlyInDark:  return !bLit;
		default:                             return true;
		}
	}
}
