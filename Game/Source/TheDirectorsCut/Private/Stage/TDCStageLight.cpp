#include "Stage/TDCStageLight.h"
#include "Core/TDCVisuals.h"
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

	Housing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Housing"));
	Housing->SetupAttachment(Bulb);
	Housing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Housing->SetCastShadow(false); // la luz está adentro de la carcasa: si proyecta sombra, apaga el set
	Housing->SetUsingAbsoluteScale(true);
	Housing->SetVisibility(false);

	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(Bulb);
	Light->SetMobility(EComponentMobility::Movable);
	Light->SetLightColor(FLinearColor(1.f, 0.85f, 0.6f));
	Light->SetCastShadows(true);
}

void ATDCStageLight::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SetupHousing();
	bOn = bStartsOn;
	ApplyVisuals();
}

void ATDCStageLight::BeginPlay()
{
	Super::BeginPlay();
	SetupHousing();
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
		BulbMaterial = TDC::CreateReadColorMID(Bulb);
	}
	if (BulbMaterial)
	{
		BulbMaterial->SetVectorParameterValue(TEXT("Color"), bOn ? FLinearColor(1.f, 0.9f, 0.3f) : FLinearColor(0.08f, 0.08f, 0.08f));
		// El bulbo brilla cuando el reflector está prendido (parámetro Emissive de M_TDCColor).
		BulbMaterial->SetVectorParameterValue(TEXT("Emissive"), bOn ? FLinearColor(4.f, 3.4f, 1.2f) : FLinearColor::Black);
	}
}

void ATDCStageLight::SetupHousing()
{
	UStaticMesh* Mesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr,
		TEXT("/Game/Dressing/Props/SM_StageLight_Hanging.SM_StageLight_Hanging"), nullptr, LOAD_NoWarn | LOAD_Quiet));
	if (!Mesh)
	{
		Housing->SetVisibility(false);
		return; // sin el asset queda la esfera de siempre
	}
	Housing->SetStaticMesh(Mesh);
	Bulb->SetWorldScale3D(FVector(HangingBulbScale));
	// La posición relativa se escala con el padre (el bulbo): dividir para que el offset quede en cm de mundo.
	const FVector ParentScale = Bulb->GetComponentScale();
	const FVector SafeScale(FMath::Max(ParentScale.X, KINDA_SMALL_NUMBER), FMath::Max(ParentScale.Y, KINDA_SMALL_NUMBER),
		FMath::Max(ParentScale.Z, KINDA_SMALL_NUMBER));
	Housing->SetRelativeLocationAndRotation(HousingOffset / SafeScale, HousingRotation);
	Housing->SetWorldScale3D(FVector(1.f));
	Housing->SetVisibility(true);
}
