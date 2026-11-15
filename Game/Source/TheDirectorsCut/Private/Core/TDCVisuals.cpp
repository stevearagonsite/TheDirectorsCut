#include "Core/TDCVisuals.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Texture2D.h"

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

void TDC::ApplyReadPattern(UMaterialInstanceDynamic* Material, const TCHAR* TextureName, float Strength, float TileSize)
{
	if (!Material)
	{
		return;
	}
	const FString Path = FString::Printf(TEXT("/Game/Materials/Textures/%s.%s"), TextureName, TextureName);
	if (UTexture2D* Tex = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *Path, nullptr,
		LOAD_NoWarn | LOAD_Quiet)))
	{
		Material->SetTextureParameterValue(TEXT("Pattern"), Tex);
		Material->SetScalarParameterValue(TEXT("PatternStrength"), Strength);
		Material->SetScalarParameterValue(TEXT("TileSize"), TileSize);
	}
}
