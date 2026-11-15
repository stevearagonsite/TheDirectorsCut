#pragma once

#include "CoreMinimal.h"

class UPrimitiveComponent;
class UMaterialInstanceDynamic;

namespace TDC
{
	/** Material base de los colores de lectura del greybox: VectorParameter "Color" -> BaseColor (lo crea Tools/unreal/make_materials.py). */
	inline const TCHAR* ReadColorMaterialPath = TEXT("/Game/Materials/M_TDCColor.M_TDCColor");

	/**
	 * Crea el MID de un componente sobre M_TDCColor para que "Color" se vea (en 5.8 el parámetro de BasicShapeMaterial
	 * no se aplica). Si M_TDCColor no existe, usa el material actual del componente. Carga silenciosa.
	 */
	THEDIRECTORSCUT_API UMaterialInstanceDynamic* CreateReadColorMID(UPrimitiveComponent* Component, int32 ElementIndex = 0);
}
