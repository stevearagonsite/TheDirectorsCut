#include "Combat/TDCExtra.h"
#include "Core/TDCFlipbookFx.h"
#include "Core/TDCVisuals.h"
#include "Character/TDCCharacter.h"
#include "Stage/TDCLightReactiveComponent.h"
#include "TheDirectorsCut.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ATDCExtra::ATDCExtra()
{
	PrimaryActorTick.bCanEverTick = true;

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->InitCapsuleSize(40.f, 90.f);
	Capsule->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Capsule->SetGenerateOverlapEvents(true);
	RootComponent = Capsule;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(RootComponent);
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Body->SetRelativeScale3D(FVector(0.7f, 0.7f, 1.7f));
	if (Cylinder.Succeeded())
	{
		Body->SetStaticMesh(Cylinder.Object);
	}

	ScriptMark = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScriptMark"));
	ScriptMark->SetupAttachment(RootComponent);
	ScriptMark->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ScriptMark->SetRelativeLocation(FVector(0.f, 0.f, 60.f));
	ScriptMark->SetRelativeScale3D(FVector(0.75f, 0.75f, 0.12f));
	if (Cube.Succeeded())
	{
		ScriptMark->SetStaticMesh(Cube.Object);
	}

	LightReactive = CreateDefaultSubobject<UTDCLightReactiveComponent>(TEXT("LightReactive"));
}

FLinearColor ATDCExtra::ReadColor() const
{
	if (StunRemaining > 0.f)
	{
		return FLinearColor(0.9f, 0.9f, 0.9f);
	}
	switch (Vulnerability)
	{
	case ETDCVulnerability::Common:    return FLinearColor(0.75f, 0.12f, 0.1f);
	case ETDCVulnerability::Resistant: return FLinearColor(0.9f, 0.45f, 0.08f);
	default:                           return FLinearColor(0.03f, 0.03f, 0.04f);
	}
}

void ATDCExtra::ApplyColor()
{
	if (!Material && Body->GetMaterial(0))
	{
		Material = TDC::CreateReadColorMID(Body);
	}
	if (Material)
	{
		Material->SetVectorParameterValue(TEXT("Color"), ReadColor());
	}
}

void ATDCExtra::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	LightReactive->Presence = Presence;
	ScriptMark->SetVisibility(Vulnerability == ETDCVulnerability::Scripted);
	ApplyColor();
}

void ATDCExtra::BeginPlay()
{
	Super::BeginPlay();
	Origin = GetActorLocation();
	Capsule->OnComponentBeginOverlap.AddDynamic(this, &ATDCExtra::OnTouch);
	LightReactive->OnPresenceChanged.AddDynamic(this, &ATDCExtra::OnPresenceChanged);
	ApplyColor();
}

void ATDCExtra::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bDefeated)
	{
		return;
	}
	if (StunRemaining > 0.f)
	{
		StunRemaining -= DeltaSeconds;
		if (StunRemaining <= 0.f)
		{
			ApplyColor();
			if (StunFx.IsValid())
			{
				StunFx->Destroy();
			}
			StunFx.Reset();
		}
		return;
	}
	if (PatrolDistance <= 0.f || PatrolSpeed <= 0.f || !LightReactive->IsPresent())
	{
		return;
	}
	FVector Location = GetActorLocation();
	Location.X += Direction * PatrolSpeed * DeltaSeconds;
	if (FMath::Abs(Location.X - Origin.X) >= PatrolDistance)
	{
		Location.X = Origin.X + FMath::Sign(Location.X - Origin.X) * PatrolDistance;
		Direction = -Direction;
	}
	SetActorLocation(Location);
}

ETDCClapResult ATDCExtra::ReceiveClap(const FVector& FromLocation)
{
	if (bDefeated)
	{
		return ETDCClapResult::Defeated;
	}
	const ETDCClapResult Result = TDC::ResolveClap(Vulnerability);
	switch (Result)
	{
	case ETDCClapResult::Defeated:
		Defeat();
		break;
	case ETDCClapResult::Stunned:
	{
		StunRemaining = StunSeconds;
		const float Away = FMath::Sign(GetActorLocation().X - FromLocation.X);
		SetActorLocation(GetActorLocation() + FVector(Away * KnockbackDistance, 0.f, 0.f));
		ApplyColor();
		if (!StunFx.IsValid())
		{
			StunFx = ATDCFlipbookFx::Spawn(GetWorld(), TDC::Fx::ExtraStun, GetActorLocation() + FVector(0.f, 50.f, 110.f), 110.f,
				false, GetRootComponent());
		}
		break;
	}
	default:
		break;
	}
	UE_LOG(LogTDC, Log, TEXT("Claquetazo a %s -> %d"), *GetName(), static_cast<int32>(Result));
	return Result;
}

void ATDCExtra::OnPresenceChanged(bool bPresent)
{
	// Dirigir la escena: si la luz lo saca de la toma, el extra queda fuera para siempre.
	if (!bPresent && Presence != ETDCLightPresence::Always)
	{
		Defeat();
	}
}

void ATDCExtra::Defeat()
{
	if (bDefeated)
	{
		return;
	}
	bDefeated = true;
	ATDCFlipbookFx::Spawn(GetWorld(), TDC::Fx::ExtraDefeat, GetActorLocation() + FVector(0.f, 60.f, 0.f), 230.f);
	if (StunFx.IsValid())
	{
		StunFx->Destroy();
	}
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
}

void ATDCExtra::OnTouch(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bDefeated || StunRemaining > 0.f)
	{
		return;
	}
	if (ATDCCharacter* Director = Cast<ATDCCharacter>(OtherActor))
	{
		Director->Respawn();
	}
}
