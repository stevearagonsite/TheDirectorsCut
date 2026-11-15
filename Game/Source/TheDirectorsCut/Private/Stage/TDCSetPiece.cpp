#include "Stage/TDCSetPiece.h"
#include "Core/TDCVisuals.h"
#include "Stage/TDCLightReactiveComponent.h"
#include "Stage/TDCTimelineComponent.h"
#include "Stage/TDCStageSubsystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Engine/CollisionProfile.h"

ATDCSetPiece::ATDCSetPiece()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	RootComponent = Mesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Cube.Succeeded())
	{
		Mesh->SetStaticMesh(Cube.Object);
	}

	LightReactive = CreateDefaultSubobject<UTDCLightReactiveComponent>(TEXT("LightReactive"));
	Timeline = CreateDefaultSubobject<UTDCTimelineComponent>(TEXT("Timeline"));
	Timeline->bAutoActivate = false;
}

FLinearColor ATDCSetPiece::ReadColor() const
{
	if (ColorOverride.A > 0.f)
	{
		return ColorOverride;
	}
	if (bRecordTime)
	{
		return FLinearColor(0.25f, 0.6f, 1.f);
	}
	switch (Presence)
	{
	case ETDCLightPresence::OnlyInLight: return FLinearColor(1.f, 0.8f, 0.15f);
	case ETDCLightPresence::OnlyInDark:  return FLinearColor(0.45f, 0.15f, 0.8f);
	default:                             return FLinearColor(0.35f, 0.35f, 0.38f);
	}
}

void ATDCSetPiece::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	LightReactive->Presence = Presence;

	if (!Material && Mesh->GetMaterial(0))
	{
		Material = TDC::CreateReadColorMID(Mesh);
	}
	if (Material)
	{
		Material->SetVectorParameterValue(TEXT("Color"), ReadColor());
		ApplyPattern();
	}
}

void ATDCSetPiece::BeginPlay()
{
	LightReactive->Presence = Presence;
	if (bRecordTime)
	{
		Timeline->Activate(true);
	}
	else
	{
		Timeline->SetComponentTickEnabled(false);
	}
	Mesh->SetSimulatePhysics(bStartsSimulating);

	// En el build empaquetado el construction script no corre para las piezas del nivel: el color se aplica acá también.
	if (!Material)
	{
		Material = TDC::CreateReadColorMID(Mesh);
	}
	if (Material)
	{
		Material->SetVectorParameterValue(TEXT("Color"), ReadColor());
		ApplyPattern();
	}

	if (UTDCStageSubsystem* Stage = GetWorld()->GetSubsystem<UTDCStageSubsystem>())
	{
		LightingHandle = Stage->OnLightingChanged.AddUObject(this, &ATDCSetPiece::OnLightingChanged);
	}

	Super::BeginPlay();
}

void ATDCSetPiece::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UTDCStageSubsystem* Stage = GetWorld()->GetSubsystem<UTDCStageSubsystem>())
	{
		Stage->OnLightingChanged.Remove(LightingHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void ATDCSetPiece::OnLightingChanged()
{
	if (Mesh->IsSimulatingPhysics())
	{
		Mesh->WakeAllRigidBodies();
	}
}

void ATDCSetPiece::Release()
{
	Mesh->SetSimulatePhysics(true);
	Mesh->WakeAllRigidBodies();
}

void ATDCSetPiece::ApplyPattern()
{
	// Tablones = fija; gelatina = existe con luz / a oscuras; película = en toma (rebobinable). El color lo sigue poniendo ReadColor.
	if (ColorOverride.A > 0.f)
	{
		return; // marcadores (salida, escenario) quedan lisos
	}
	const TCHAR* Tex = bRecordTime ? TEXT("T_Take_Holo")
		: (Presence != ETDCLightPresence::Always ? TEXT("T_Gel_Panel") : TEXT("T_Stage_Floor"));
	const float Strength = Presence == ETDCLightPresence::Always && !bRecordTime ? 0.9f : 0.75f;
	TDC::ApplyReadPattern(Material, Tex, Strength);
}
