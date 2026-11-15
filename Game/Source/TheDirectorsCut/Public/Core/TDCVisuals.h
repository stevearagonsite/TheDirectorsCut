#pragma once

#include "CoreMinimal.h"

class UPrimitiveComponent;
class UMaterialInstanceDynamic;
class UAudioComponent;
class USceneComponent;

namespace TDC
{
	/** Material base de los colores de lectura del greybox: VectorParameter "Color" -> BaseColor (lo crea Tools/unreal/make_materials.py). */
	inline const TCHAR* ReadColorMaterialPath = TEXT("/Game/Materials/M_TDCColor.M_TDCColor");

	/**
	 * Crea el MID de un componente sobre M_TDCColor para que "Color" se vea (en 5.8 el parámetro de BasicShapeMaterial
	 * no se aplica). Si M_TDCColor no existe, usa el material actual del componente. Carga silenciosa.
	 */
	THEDIRECTORSCUT_API UMaterialInstanceDynamic* CreateReadColorMID(UPrimitiveComponent* Component, int32 ElementIndex = 0);

	/** MID de un material por ruta ("/Game/X/M.M"); nullptr si el material no existe (sin fallback). */
	THEDIRECTORSCUT_API UMaterialInstanceDynamic* CreateMIDFrom(UPrimitiveComponent* Component, const TCHAR* MaterialPath,
		int32 ElementIndex = 0);

	/** Sonido /Game/Audio/<Name> (import_audio.py) en una posición; sin asset no hace nada. */
	THEDIRECTORSCUT_API void PlaySfx(const UObject* WorldContext, const TCHAR* Name, const FVector& Location, float Volume = 1.f);

	/** Sonido 2D (voz, ambiente); devuelve el componente para loops (puede ser nullptr). */
	THEDIRECTORSCUT_API UAudioComponent* PlaySfx2D(const UObject* WorldContext, const TCHAR* Name, float Volume = 1.f);

	/** Sonido pegado a un componente (p. ej. loop de rebobinado); devuelve el componente o nullptr. */
	THEDIRECTORSCUT_API UAudioComponent* PlaySfxAttached(const TCHAR* Name, USceneComponent* AttachTo, float Volume = 1.f);

	/** Patrón de set (T-0021) sobre M_TDCColor: /Game/Materials/Textures/T_<Name>. Sin textura, no hace nada. */
	THEDIRECTORSCUT_API void ApplyReadPattern(UMaterialInstanceDynamic* Material, const TCHAR* TextureName, float Strength,
		float TileSize = 200.f);
}
