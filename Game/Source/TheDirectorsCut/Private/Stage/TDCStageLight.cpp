#include "Stage/TDCStageLight.h"
#include "Core/TDCVisuals.h"
#include "Stage/TDCStageSubsystem.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"

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

	Cone = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cone"));
	Cone->SetupAttachment(Bulb);
	Cone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Cone->SetCastShadow(false);
	Cone->SetUsingAbsoluteScale(true);
	Cone->SetUsingAbsoluteRotation(true);
	Cone->SetVisibility(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh.Succeeded())
	{
		Cone->SetStaticMesh(PlaneMesh.Object);
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
	SetupHousing();
	SetupCone();
	bOn = bStartsOn;
	ApplyVisuals();
}

void ATDCStageLight::BeginPlay()
{
	Super::BeginPlay();
	SetupHousing();
	SetupCone();
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
	// Halo en la niebla volumétrica (build_dressing): muestra el alcance del reflector cuando está prendido.
	Light->SetVolumetricScatteringIntensity(bOn ? 6.f : 0.f);
	Cone->SetVisibility(bOn && bConeReady);
	if (ConeMaterial)
	{
		ConeMaterial->SetScalarParameterValue(TEXT("Intensity"), ConeIntensity);
	}

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

void ATDCStageLight::SetupCone()
{
	bConeReady = false;
	UMaterialInterface* Base = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr,
		TEXT("/Game/Materials/M_TDCLightCone.M_TDCLightCone"), nullptr, LOAD_NoWarn | LOAD_Quiet));
	if (!Base || !Cone->GetStaticMesh())
	{
		return;
	}
	if (!ConeMaterial)
	{
		ConeMaterial = Cone->CreateDynamicMaterialInstance(0, Base);
	}
	// Quad vertical de cara a la cámara (+Y): ancho = 2 × alcance / 0.9 (la textura usa el 90 % del ancho en el piso),
	// alto = distancia lente → plano de juego / 0.925. Un poco detrás del plano de juego para no lavar a los personajes.
	const FVector Lens = GetActorLocation();
	const float Drop = FMath::Max(300.f, static_cast<float>(Lens.Z) - PlayPlaneZ);
	const float Width = 2.f * Radius / 0.9f;
	const float Height = Drop / 0.925f;
	Cone->SetWorldRotation(FRotator(0.f, 0.f, 90.f)); // el Plane del motor mira a +Z: roll 90 lo para (su Y pasa a Z)
	Cone->SetWorldScale3D(FVector(Width / 100.f, Height / 100.f, 1.f));
	Cone->SetWorldLocation(FVector(Lens.X, Lens.Y - 60.f, Lens.Z - Height * 0.5f));
	bConeReady = ConeMaterial != nullptr;
}
