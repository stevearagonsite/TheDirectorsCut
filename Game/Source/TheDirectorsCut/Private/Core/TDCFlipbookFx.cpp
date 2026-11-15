#include "Core/TDCFlipbookFx.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ATDCFlipbookFx::ATDCFlipbookFx()
{
	PrimaryActorTick.bCanEverTick = false;
	Quad = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Quad"));
	RootComponent = Quad;
	Quad->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Quad->SetCastShadow(false);
	Quad->SetMobility(EComponentMobility::Movable);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Cube.Succeeded())
	{
		Quad->SetStaticMesh(Cube.Object);
	}
}

ATDCFlipbookFx* ATDCFlipbookFx::Spawn(UWorld* World, const FTDCFlipbook& Fx, const FVector& Location, float WidthCm,
	bool bFlipX, USceneComponent* AttachTo)
{
	if (!World)
	{
		return nullptr;
	}
	UMaterialInterface* Base = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr,
		TEXT("/Game/VFX/M_TDCFlipbook.M_TDCFlipbook"), nullptr, LOAD_NoWarn | LOAD_Quiet));
	const FString TexPath = FString::Printf(TEXT("/Game/VFX/T_%s.T_%s"), Fx.Name, Fx.Name);
	UTexture2D* Sheet = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *TexPath, nullptr,
		LOAD_NoWarn | LOAD_Quiet));
	if (!Base || !Sheet)
	{
		return nullptr; // sin assets de VFX el juego sigue igual
	}
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ATDCFlipbookFx* Actor = World->SpawnActor<ATDCFlipbookFx>(Location, FRotator::ZeroRotator, Params);
	if (!Actor)
	{
		return nullptr;
	}
	const float Height = WidthCm / FMath::Max(0.1f, Fx.Aspect);
	Actor->Quad->SetWorldScale3D(FVector((bFlipX ? -1.f : 1.f) * WidthCm / 100.f, 0.01f, Height / 100.f));
	UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(Base, Actor);
	Mat->SetTextureParameterValue(TEXT("Sheet"), Sheet);
	Mat->SetScalarParameterValue(TEXT("Cols"), static_cast<float>(Fx.Cols));
	Mat->SetScalarParameterValue(TEXT("Rows"), static_cast<float>(Fx.Rows));
	Mat->SetScalarParameterValue(TEXT("Frames"), static_cast<float>(Fx.Frames));
	Mat->SetScalarParameterValue(TEXT("FPS"), Fx.Fps);
	Mat->SetScalarParameterValue(TEXT("Loop"), Fx.bLoop ? 1.f : 0.f);
	Mat->SetScalarParameterValue(TEXT("StartTime"), static_cast<float>(World->GetTimeSeconds()));
	Actor->Quad->SetMaterial(0, Mat);
	Actor->Quad->SetTranslucentSortPriority(10);
	if (AttachTo)
	{
		Actor->AttachToComponent(AttachTo, FAttachmentTransformRules::KeepWorldTransform);
	}
	if (!Fx.bLoop)
	{
		Actor->SetLifeSpan(Fx.Frames / Fx.Fps + 0.05f);
	}
	return Actor;
}
