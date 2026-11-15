#include "Core/TDCVisuals.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"

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

UMaterialInstanceDynamic* TDC::CreateMIDFrom(UPrimitiveComponent* Component, const TCHAR* MaterialPath, int32 ElementIndex)
{
	if (!Component)
	{
		return nullptr;
	}
	UMaterialInterface* Base = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr,
		MaterialPath, nullptr, LOAD_NoWarn | LOAD_Quiet));
	return Base ? Component->CreateDynamicMaterialInstance(ElementIndex, Base) : nullptr;
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

namespace
{
	USoundBase* LoadSfx(const TCHAR* Name)
	{
		const FString Path = FString::Printf(TEXT("/Game/Audio/%s.%s"), Name, Name);
		return Cast<USoundBase>(StaticLoadObject(USoundBase::StaticClass(), nullptr, *Path, nullptr, LOAD_NoWarn | LOAD_Quiet));
	}
}

void TDC::PlaySfx(const UObject* WorldContext, const TCHAR* Name, const FVector& Location, float Volume)
{
	if (USoundBase* Sound = LoadSfx(Name))
	{
		UGameplayStatics::PlaySoundAtLocation(WorldContext, Sound, Location, Volume);
	}
}

UAudioComponent* TDC::PlaySfx2D(const UObject* WorldContext, const TCHAR* Name, float Volume)
{
	USoundBase* Sound = LoadSfx(Name);
	return Sound ? UGameplayStatics::SpawnSound2D(WorldContext, Sound, Volume) : nullptr;
}

UAudioComponent* TDC::PlaySfxAttached(const TCHAR* Name, USceneComponent* AttachTo, float Volume)
{
	USoundBase* Sound = LoadSfx(Name);
	return (Sound && AttachTo) ? UGameplayStatics::SpawnSoundAttached(Sound, AttachTo, NAME_None, FVector::ZeroVector,
		EAttachLocation::KeepRelativeOffset, false, Volume) : nullptr;
}
