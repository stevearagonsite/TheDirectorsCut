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
#include "Core/TDCFlipbookFx.h"
#include "TimerManager.h"
#include "Components/InstancedStaticMeshComponent.h"

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

	if (Mesh->GetMaterial(0))
	{
		SetupMaterial();
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
	SetupMaterial();
	if (Presence != ETDCLightPresence::Always)
	{
		// Antes de Super::BeginPlay: el componente evalúa la luz en su BeginPlay y avisa por OnPresenceChanged.
		LightReactive->OnPresenceChanged.AddUniqueDynamic(this, &ATDCSetPiece::HandlePresenceChanged);
	}

	if (UTDCStageSubsystem* Stage = GetWorld()->GetSubsystem<UTDCStageSubsystem>())
	{
		LightingHandle = Stage->OnLightingChanged.AddUObject(this, &ATDCSetPiece::OnLightingChanged);
		if (bRecordTime)
		{
			TimeModeHandle = Stage->OnTimeModeChanged.AddUObject(this, &ATDCSetPiece::OnTimeModeChanged);
		}
	}

	BuildKitTrim();
	Super::BeginPlay();
	// Desde acá los cambios de presencia son del jugador (prender/apagar luces): llevan VFX.
	bLastPresent = LightReactive->IsPresent();
	bPresenceFxReady = true;
}

void ATDCSetPiece::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UTDCStageSubsystem* Stage = GetWorld()->GetSubsystem<UTDCStageSubsystem>())
	{
		Stage->OnLightingChanged.Remove(LightingHandle);
		Stage->OnTimeModeChanged.Remove(TimeModeHandle);
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
	if (!PatternOverride.IsNone())
	{
		// Superficies de decorado con su propia textura (p. ej. el adoquín de la calle en Pit_Floor, T-0041).
		const FString Name = PatternOverride.ToString();
		TDC::ApplyReadPattern(Material, *Name, PatternOverrideStrength, 300.f);
		return;
	}
	if (ColorOverride.A > 0.f)
	{
		return; // marcadores (salida, escenario) quedan lisos
	}
	const TCHAR* Tex = bRecordTime ? TEXT("T_Take_Holo")
		: (Presence != ETDCLightPresence::Always ? TEXT("T_Gel_Panel") : TEXT("T_Stage_Floor"));
	// T-0023: T_Stage_Floor es casi carbón; con 0.9 las plataformas salían negras. 0.35 = textura que modula el color sin apagarlo.
	const float Strength = 0.35f;
	TDC::ApplyReadPattern(Material, Tex, Strength);
}

bool ATDCSetPiece::WantsLightMaterial() const
{
	return Presence != ETDCLightPresence::Always && !bRecordTime && ColorOverride.A <= 0.f;
}

void ATDCSetPiece::SetupMaterial()
{
	if (!Material && WantsLightMaterial())
	{
		Material = TDC::CreateMIDFrom(Mesh, TEXT("/Game/Materials/M_TDCLightPiece.M_TDCLightPiece"));
		bLightMaterial = Material != nullptr;
	}
	if (!Material)
	{
		Material = TDC::CreateReadColorMID(Mesh);
	}
	if (!Material)
	{
		return;
	}
	Material->SetVectorParameterValue(TEXT("Color"), ReadColor());
	if (bLightMaterial)
	{
		// Hecha de luz: translúcida, brilla y no tapa la luz del set. Ausente = fantasma tenue (sin colisión).
		LightReactive->bHideWhenAbsent = false;
		Mesh->SetCastShadow(false);
		HandlePresenceChanged(LightReactive->IsPresent());
	}
	else
	{
		ApplyPattern();
	}
}

void ATDCSetPiece::HandlePresenceChanged(bool bNowPresent)
{
	if (bPresenceFxReady && bNowPresent != bLastPresent)
	{
		SpawnPresenceFx(bNowPresent);
	}
	bLastPresent = bNowPresent;
	if (!Material || !bLightMaterial)
	{
		return;
	}
	// La de oscuridad brilla menos: es "sombra sólida", no luz.
	const float FullGlow = Presence == ETDCLightPresence::OnlyInLight ? 1.4f : 0.6f;
	// Ausente: casi sin relleno y con el borde marcado (contorno de "dónde va a estar"). Al 20 % de relleno la pared W1
	// se leía como un panel gris sólido (T-0052).
	Material->SetScalarParameterValue(TEXT("Opacity"), bNowPresent ? 0.5f : 0.07f);
	Material->SetScalarParameterValue(TEXT("Glow"), bNowPresent ? FullGlow : 0.35f);
	Material->SetScalarParameterValue(TEXT("EdgeGlow"), bNowPresent ? 2.5f : 1.8f);
	Material->SetScalarParameterValue(TEXT("Pulse"), bNowPresent ? 0.08f : 0.f);
}

void ATDCSetPiece::OnTimeModeChanged(ETDCTimeMode NewMode)
{
	// Solo las piezas que se movieron (más que el frame de partida) muestran que el tiempo las tocó.
	const bool bAffected = Timeline && Timeline->GetRecordedSeconds() * Timeline->SamplesPerSecond > 1.5f;
	bTakeRewinding = NewMode == ETDCTimeMode::Rewind && bAffected;
	if (bTakeRewinding)
	{
		SetTakeGlow(1.f);
		if (!TakeFx.IsValid())
		{
			const FBox Box = GetComponentsBoundingBox(true);
			const float Width = FMath::Max(Box.GetSize().X * 1.2f, 150.f);
			TakeFx = ATDCFlipbookFx::Spawn(GetWorld(), TDC::Fx::Rewind, Box.GetCenter() + FVector(0.f, Box.GetExtent().Y + 20.f, 0.f),
				Width, false, Mesh);
		}
	}
	else if (TakeFx.IsValid())
	{
		TakeFx->Destroy();
		TakeFx.Reset();
	}
	if (TakeGlow > 0.f && !GetWorldTimerManager().IsTimerActive(TakeGlowTimer))
	{
		GetWorldTimerManager().SetTimer(TakeGlowTimer, this, &ATDCSetPiece::TickTakeGlow, 0.05f, true);
	}
}

void ATDCSetPiece::TickTakeGlow()
{
	if (bTakeRewinding)
	{
		// Pulso de proyector mientras rebobina.
		SetTakeGlow(0.75f + 0.25f * FMath::Sin(GetWorld()->GetTimeSeconds() * 12.f));
		return;
	}
	// Después: resplandor cian que se apaga en ~2.5 s, con parpadeo de película al principio.
	TakeGlow = FMath::Max(0.f, TakeGlow - 0.02f);
	const float Flicker = TakeGlow > 0.5f && FMath::FRand() < 0.25f ? 0.4f : 1.f;
	SetTakeGlow(TakeGlow, Flicker);
	if (TakeGlow <= 0.f)
	{
		GetWorldTimerManager().ClearTimer(TakeGlowTimer);
	}
}

void ATDCSetPiece::SetTakeGlow(float Amount, float Visual)
{
	TakeGlow = FMath::Clamp(Amount, 0.f, 1.f);
	if (Material)
	{
		const FLinearColor Cyan(0.25f, 0.85f, 1.f);
		Material->SetVectorParameterValue(TEXT("Emissive"), Cyan * (3.f * TakeGlow * Visual));
	}
}

void ATDCSetPiece::SpawnPresenceFx(bool bNowPresent)
{
	const bool bLight = Presence == ETDCLightPresence::OnlyInLight;
	const FTDCFlipbook& Fx = bLight ? (bNowPresent ? TDC::Fx::LightPieceAppear : TDC::Fx::LightPieceVanish)
		: (bNowPresent ? TDC::Fx::DarkPieceAppear : TDC::Fx::DarkPieceVanish);
	const FBox Box = GetComponentsBoundingBox(/*bNonColliding*/ true);
	if (!Box.IsValid)
	{
		return;
	}
	const FVector Size = Box.GetSize();
	// Paredes (más altas que anchas): el efecto se gira 90° y recorre la pieza de abajo arriba.
	const bool bVertical = Size.Z > Size.X;
	const float Width = FMath::Max(bVertical ? Size.Z : Size.X, 100.f) * 1.21f;
	FVector Location = Box.GetCenter() + FVector(0.f, Box.GetExtent().Y + 15.f, 0.f);
	if (!bVertical)
	{
		// En el sprite la pieza está 10 px (de 136) por debajo del centro del frame: subir el quad para alinearla.
		Location.Z += Width * 10.f / 136.f;
	}
	if (ATDCFlipbookFx* Spawned = ATDCFlipbookFx::Spawn(GetWorld(), Fx, Location, Width))
	{
		if (bVertical)
		{
			Spawned->SetActorRotation(FRotator(90.f, 0.f, 0.f));
		}
	}
}

void ATDCSetPiece::BuildKitTrim()
{
	if (!bKitTrim || KitTrim || Presence != ETDCLightPresence::Always || bRecordTime)
	{
		return; // solo piezas fijas: las de luz/toma tienen su propio material (translúcido, cian)
	}
	UStaticMesh* Kit = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr,
		TEXT("/Game/Dressing/Kit/SM_Ledge_Mid.SM_Ledge_Mid"), nullptr, LOAD_NoWarn | LOAD_Quiet));
	if (!Kit)
	{
		return;
	}
	const FVector Size = Mesh->Bounds.BoxExtent * 2.0; // tamaño en mundo del bloque (las piezas no rotan)
	const FBoxSphereBounds KitBounds = Kit->GetBounds();
	const FVector Module = KitBounds.BoxExtent * 2.0;
	if (Module.X < 1.0 || Size.X < 1.0)
	{
		return;
	}
	const int32 Count = FMath::Max(1, FMath::RoundToInt(Size.X / Module.X));
	const double ScaleX = Size.X / (Count * Module.X); // estira apenas para cerrar el ancho exacto
	KitTrim = NewObject<UInstancedStaticMeshComponent>(this, TEXT("KitTrim"));
	KitTrim->SetupAttachment(RootComponent);
	KitTrim->SetUsingAbsoluteScale(true); // instancias en cm, sin heredar la escala del cubo
	KitTrim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	KitTrim->SetStaticMesh(Kit);
	KitTrim->RegisterComponent();
	const FRotator Facing(0.0, KitTrimYaw, 0.0);
	const FVector Scale(ScaleX, 1.0, 1.0);
	// Centro del módulo en su espacio local, ya escalado y girado: se compensa para centrar cada instancia.
	const FVector Pivot = Facing.RotateVector(KitBounds.Origin * Scale);
	const double TopZ = Size.Z * 0.5;
	for (int32 i = 0; i < Count; ++i)
	{
		const double X = -Size.X * 0.5 + (i + 0.5) * Size.X / Count;
		// Frente del bloque (+Y, hacia la cámara), con la tapa de la cornisa alineada con la tapa del bloque.
		const FVector Center(X, Size.Y * 0.5, TopZ - Module.Z * 0.5);
		KitTrim->AddInstance(FTransform(Facing, Center - Pivot, Scale));
	}
}
