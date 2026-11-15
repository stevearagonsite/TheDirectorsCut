#include "Core/TDCVisuals.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

UMaterialInstanceDynamic* TDC::CreateReadColorMID(UPrimitiveComponent* Component, int32 ElementIndex)
{
	if (!Component)
	{
		return nullptr;
	}
	UMaterialInterface* Base = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr,
		ReadColorMaterialPath, nullptr, LOAD_NoWarn | LOAD_Quiet));
	return Base ? Component->CreateDynamicMaterialInstance(ElementIndex, Base) : Component->CreateDynamicMaterialInstance(ElementIndex);
}
