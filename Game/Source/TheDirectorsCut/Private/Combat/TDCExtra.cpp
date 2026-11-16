#include "Combat/TDCExtra.h"
#include "Core/TDCFlipbookFx.h"
#include "Core/TDCVisuals.h"
#include "Core/TDCVisuals.h"
#include "Character/TDCCharacter.h"
#include "Stage/TDCLightReactiveComponent.h"
#include "TheDirectorsCut.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSequence.h"
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

	Puppet = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Puppet"));
	Puppet->SetupAttachment(RootComponent);
	Puppet->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Puppet->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	// Que anime siempre (también fuera de cámara o en el primer frame) y a frecuencia completa.
	Puppet->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	Puppet->bEnableUpdateRateOptimizations = false;
}

void ATDCExtra::LoadPuppet()
{
	const TCHAR* Type = Vulnerability == ETDCVulnerability::Common ? TEXT("Comun")
		: Vulnerability == ETDCVulnerability::Resistant ? TEXT("Resistente") : TEXT("Sombra");
	auto Load = [Type](const TCHAR* Suffix, UClass* Cls) -> UObject*
	{
		const FString Name = FString::Printf(TEXT("SK_Extra_%s%s"), Type, Suffix);
		const FString Path = FString::Printf(TEXT("/Game/Characters/Extras/%s/%s.%s"), Type, *Name, *Name);
		return StaticLoadObject(Cls, nullptr, *Path, nullptr, LOAD_NoWarn | LOAD_Quiet);
	};
	USkeletalMesh* Mesh = Cast<USkeletalMesh>(Load(TEXT(""), USkeletalMesh::StaticClass()));
	if (!Mesh)
	{
		return; // sin modelo: queda el greybox
	}
	Puppet->SetSkeletalMesh(Mesh);
	Puppet->SetRelativeScale3D(FVector(PuppetScale));
	const FString AnimBase = FString::Printf(TEXT("/Game/Characters/Extras/%s/AS_Extra_%s_"), Type, Type);
	PuppetIdle = Cast<UAnimSequence>(StaticLoadObject(UAnimSequence::StaticClass(), nullptr,
		*(AnimBase + TEXT("Idle.AS_Extra_") + Type + TEXT("_Idle")), nullptr, LOAD_NoWarn | LOAD_Quiet));
	PuppetWalk = Cast<UAnimSequence>(StaticLoadObject(UAnimSequence::StaticClass(), nullptr,
		*(AnimBase + TEXT("Walk.AS_Extra_") + Type + TEXT("_Walk")), nullptr, LOAD_NoWarn | LOAD_Quiet));
	Puppet->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	Body->SetVisibility(false);
	ScriptMark->SetVisibility(false);
	bPuppetWalking = false;
	PuppetScaleCheckFrames = 0;
	if (PuppetIdle)
	{
		Puppet->PlayAnimation(PuppetIdle, true);
		// Evaluar la pose ya (sincrónico: sin tick function) y escalar en el mismo frame: shot/smoke capturan antes del
		// tick 3 y ahí el extra medía 2 cm. La medición del tick 3 queda como confirmación (es idempotente).
		Puppet->TickAnimation(0.f, false);
		Puppet->RefreshBoneTransforms();
		FitPuppetScale();
	}
	UpdatePuppet();
}

void ATDCExtra::FitPuppetScale()
{
	// T-0033: con el skeleton bien vinculado, los clips de Rodin aplican la escala 0.01 de su root y el extra quedaba de
	// ~2 cm (invisible). Medimos la altura de la pose animada (huesos en component space) contra la del mesh y, si
	// difiere más de 3x, compensamos. Si los clips no traen escala, no cambia nada.
	float MinZ = TNumericLimits<float>::Max();
	float MaxZ = TNumericLimits<float>::Lowest();
	for (int32 Bone = 0; Bone < Puppet->GetNumBones(); ++Bone)
	{
		const float Z = Puppet->GetBoneTransform(Bone, FTransform::Identity).GetLocation().Z;
		MinZ = FMath::Min(MinZ, Z);
		MaxZ = FMath::Max(MaxZ, Z);
	}
	const float PoseHeight = MaxZ - MinZ;
	const float MeshHeight = Puppet->GetSkeletalMeshAsset()->GetBounds().BoxExtent.Z * 2.f;
	float Scale = PuppetScale;
	if (PoseHeight > 0.01f && MeshHeight > 1.f)
	{
		const float Ratio = MeshHeight * 0.9f / PoseHeight; // los huesos cubren ~90 % de la altura del mesh
		if (Ratio > 3.f || Ratio < 0.33f)
		{
			Scale = PuppetScale * Ratio;
			Puppet->SetRelativeScale3D(FVector(Scale));
			// Los bounds salen del mesh (cm) por la escala del componente: sin esto medían ~200 m (culling y luz rotos).
			Puppet->SetBoundsScale(PuppetScale / Scale);
		}
	}
	UE_LOG(LogTDC, Log, TEXT("%s: pose %.1f cm, mesh %.1f cm -> escala del puppet %.3f"), *GetName(), PoseHeight, MeshHeight, Scale);
}

void ATDCExtra::UpdatePuppet()
{
	if (!Puppet || !Puppet->GetSkeletalMeshAsset())
	{
		return;
	}
	if (PuppetScaleCheckFrames >= 0 && ++PuppetScaleCheckFrames >= 3)
	{
		PuppetScaleCheckFrames = -1; // tres ticks: la pose del clip ya está evaluada
		FitPuppetScale();
	}
	// Mira hacia donde patrulla (el FBX mira a -Y; yaw -90 = +X).
	const float Yaw = Direction >= 0.f ? -90.f : 90.f;
	if (Yaw != PuppetFacing)
	{
		PuppetFacing = Yaw;
	}
	const bool bMoving = StunRemaining <= 0.f && PatrolDistance > 0.f && PatrolSpeed > 0.f && LightReactive->IsPresent();
	// Rebote y balanceo de "extra de utilería" al caminar; aturdido tiembla. Vuelve suave a la pose base.
	const float Dt = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.f;
	FVector Loc(0.f, 0.f, -90.f);
	FRotator Rot(0.f, PuppetFacing, 0.f);
	if (bMoving)
	{
		BobPhase += Dt * PatrolSpeed / 45.f; // ~2 pasos por segundo a 150 cm/s
		Loc.Z += FMath::Abs(FMath::Sin(BobPhase)) * WalkBobCm;
		Rot.Roll = FMath::Sin(BobPhase) * WalkSwayDegrees;
	}
	else if (StunRemaining > 0.f)
	{
		BobPhase += Dt * 40.f;
		Rot.Roll = FMath::Sin(BobPhase) * 3.f;
	}
	else
	{
		BobPhase += Dt * 1.5f; // respiración
		Loc.Z += FMath::Sin(BobPhase) * 1.5f;
	}
	Puppet->SetRelativeLocationAndRotation(Loc, Rot);

	const bool bWalk = PuppetWalk && bMoving;
	if (bWalk != bPuppetWalking)
	{
		bPuppetWalking = bWalk;
		if (UAnimSequence* Anim = bWalk ? PuppetWalk.Get() : PuppetIdle.Get())
		{
			Puppet->PlayAnimation(Anim, true);
		}
	}
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
	LoadPuppet();
	Capsule->OnComponentBeginOverlap.AddDynamic(this, &ATDCExtra::OnTouch);
	LightReactive->OnPresenceChanged.AddDynamic(this, &ATDCExtra::OnPresenceChanged);
	ApplyColor();
	// Al tick siguiente ya hicieron BeginPlay todas las luces: recién ahí un cambio de presencia es "dirigir la escena".
	GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]() { bPresenceArmed = true; }));
}

void ATDCExtra::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bDefeated)
	{
		return;
	}
	UpdatePuppet();
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
			TDC::PlaySfx(this, TEXT("sfx_extra_stun"), GetActorLocation());
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
	if (!bPresenceArmed)
	{
		return; // evaluación inicial (orden de registro de las luces): no es una acción del jugador
	}
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
	TDC::PlaySfx(this, TEXT("sfx_extra_defeat"), GetActorLocation());
	TDC::PlaySfx2D(this, TEXT("vo_corten"), 0.9f);
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
