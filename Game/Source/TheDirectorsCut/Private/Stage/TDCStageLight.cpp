#include "Stage/TDCStageLight.h"
#include "Stage/TDCStageSubsystem.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"

ATDCStageLight::ATDCStageLight()
{
	PrimaryActorTick.bCanEverTick = false;

	Bulb = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bulb"));
	Bulb->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Bulb->SetWorldScale3D(FVector(0.4f));
	RootComponent = Bulb;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (Sphere.Succeeded())
	{
		Bulb->SetStaticMesh(Sphere.Object);
	}

	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(Bulb);
	Light->SetMobility(EComponentMobility::Movable);
	Light->SetLightColor(FLinearColor(1.f, 0.85f, 0.6f));
	Light->SetCastShadows(true);
}

void ATDCStageLight::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	bOn = bStartsOn;
	ApplyVisuals();
}

void ATDCStageLight::BeginPlay()
{
	Super::BeginPlay();
	bOn = bStartsOn;
	ApplyVisuals();
	if (UTDCStageSubsystem* Stage = GetWorld()->GetSubsystem<UTDCStageSubsystem>())
	{
		Stage->RegisterLight(this);
	}
}

void ATDCStageLight::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UTDCStageSubsystem* Stage = GetWorld()->GetSubsystem<UTDCStageSubsystem>())
	{
		Stage->UnregisterLight(this);
	}
	Super::EndPlay(EndPlayReason);
}

void ATDCStageLight::SetOn(bool bNewOn)
{
	if (bOn == bNewOn)
	{
		return;
	}
	bOn = bNewOn;
	ApplyVisuals();
	if (UTDCStageSubsystem* Stage = GetWorld()->GetSubsystem<UTDCStageSubsystem>())
	{
		Stage->NotifyLightingChanged();
	}
}

FTDCLightSample ATDCStageLight::GetSample() const
{
	FTDCLightSample Sample;
	Sample.Location = GetActorLocation();
	Sample.Radius = Radius;
	Sample.bOn = bOn;
	return Sample;
}

void ATDCStageLight::ApplyVisuals()
{
	Light->SetAttenuationRadius(Radius);
	Light->SetIntensity(Intensity);
	Light->SetVisibility(bOn);

	if (!BulbMaterial && Bulb->GetMaterial(0))
	{
		BulbMaterial = Bulb->CreateDynamicMaterialInstance(0);
	}
	if (BulbMaterial)
	{
		BulbMaterial->SetVectorParameterValue(TEXT("Color"), bOn ? FLinearColor(1.f, 0.9f, 0.3f) : FLinearColor(0.08f, 0.08f, 0.08f));
	}
}
