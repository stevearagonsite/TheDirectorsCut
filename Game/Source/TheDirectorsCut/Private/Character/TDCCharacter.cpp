#include "Character/TDCCharacter.h"
#include "UI/TDCHUD.h"
#include "Core/TDCVisuals.h"
#include "Director/TDCDirectorComponent.h"
#include "Debug/TDCPlaytestRunner.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace1D.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Stage/TDCStageLight.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UnrealClient.h"
#include "Engine/SkeletalMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/StaticMesh.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ATDCCharacter::ATDCCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(40.f, 90.f);
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.f, 1080.f, 0.f);
	Movement->MaxWalkSpeed = 600.f;
	Movement->JumpZVelocity = 750.f;
	Movement->GravityScale = 1.5f;
	Movement->AirControl = 0.8f;
	Movement->GroundFriction = 8.f;
	Movement->BrakingDecelerationWalking = 2400.f;
	Movement->SetPlaneConstraintEnabled(true);
	Movement->SetPlaneConstraintNormal(FVector(0.f, 1.f, 0.f));
	Movement->bSnapToPlaneAtStart = true;

	// Cámara lateral: el brazo mira hacia -Y, así la cámara queda en +Y y la derecha de pantalla es +X.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	// T-0040 (auditoría visual): un poco más alta y mirando más hacia abajo. Se ve la tapa de las plataformas (tablones) y
	// el plano de juego ocupa el centro, en vez de una franja negra abajo.
	CameraBoom->SetRelativeRotation(FRotator(-9.f, -90.f, 0.f));
	// D-012: encuadre amplio para anticipar el tramo que viene. 1900 (antes 2200): el director medía 11 % del alto de
	// pantalla y se perdía (auditoría #5); sigue viéndose el tramo siguiente gracias al TargetOffset.
	CameraBoom->TargetArmLength = 1900.f;
	CameraBoom->SocketOffset = FVector(0.f, 0.f, 250.f);
	// Look-ahead (D-012): el nivel avanza hacia +X, así que el director queda en el tercio izquierdo y se ve lo que viene.
	CameraBoom->TargetOffset = FVector(700.f, 0.f, 0.f);
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 6.f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	Camera->SetFieldOfView(60.f);

	// Greybox del personaje: cilindro + "nariz" que indica hacia dónde mira.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(RootComponent);
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Body->SetRelativeScale3D(FVector(0.8f, 0.8f, 1.8f));
	if (Cylinder.Succeeded())
	{
		Body->SetStaticMesh(Cylinder.Object);
	}

	Nose = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Nose"));
	Nose->SetupAttachment(RootComponent);
	Nose->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Nose->SetRelativeLocation(FVector(45.f, 0.f, 50.f));
	Nose->SetRelativeScale3D(FVector(0.3f, 0.2f, 0.2f));
	if (Cube.Succeeded())
	{
		Nose->SetStaticMesh(Cube.Object);
	}

	// Mesh final (Skeletal Mesh del director): se asigna en BP_Director. Pies en la base de la cápsula.
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Director = CreateDefaultSubobject<UTDCDirectorComponent>(TEXT("Director"));

	Clapper = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Clapper"));
	Clapper->SetupAttachment(GetMesh());
	Clapper->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Clapper->SetVisibility(false);
}

namespace
{
	template <typename T>
	T* LoadQuiet(const FSoftObjectPath& Path)
	{
		// Sin warnings si el asset todavía no existe: el juego sigue con el greybox.
		return Path.IsValid() ? Cast<T>(StaticLoadObject(T::StaticClass(), nullptr, *Path.ToString(), nullptr,
			LOAD_NoWarn | LOAD_Quiet)) : nullptr;
	}
}

void ATDCCharacter::LoadCharacterAssets()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		return;
	}
	// Si un Blueprint ya asignó mesh/AnimBP, se respeta.
	if (!MeshComp->GetSkeletalMeshAsset())
	{
		if (USkeletalMesh* LoadedMesh = LoadQuiet<USkeletalMesh>(CharacterMeshPath))
		{
			MeshComp->SetSkeletalMesh(LoadedMesh);
			MeshComp->SetRelativeRotation(FRotator(0.f, MeshYaw, 0.f));
			MeshComp->SetRelativeScale3D(FVector(AnimatedMeshScale));
		}
	}
	if (!MeshComp->GetSkeletalMeshAsset() || MeshComp->GetAnimClass())
	{
		return;
	}
	if (UStaticMesh* ClapperMesh = LoadQuiet<UStaticMesh>(ClapperMeshPath))
	{
		Clapper->SetStaticMesh(ClapperMesh);
		Clapper->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, ClapperSocket);
		Clapper->SetRelativeTransform(ClapperOffset);
		Clapper->SetVisibility(true);
	}
	ClapAnim = LoadQuiet<UAnimSequence>(ClapAnimPath);
	IdleAnim = LoadQuiet<UAnimSequence>(IdleAnimPath);
	RunAnim = LoadQuiet<UAnimSequence>(RunAnimPath);
	WalkAnim = LoadQuiet<UAnimSequence>(WalkAnimPath);
	SprintAnim = LoadQuiet<UAnimSequence>(SprintAnimPath);
	LocomotionBlendSpace = bUseLocomotionBlendSpace ? LoadQuiet<UBlendSpace1D>(LocomotionBlendSpacePath) : nullptr;
	if (LocomotionBlendSpace && LocomotionBlendSpace->GetBlendSamples().Num() < 2)
	{
		LocomotionBlendSpace = nullptr; // un BS sin muestras deja la pose congelada
	}
	JumpAnim = LoadQuiet<UAnimSequence>(JumpAnimPath);
	if (IdleAnim || RunAnim || JumpAnim)
	{
		MeshComp->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		UpdateAnimState();
	}
}

void ATDCCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// Playtest: eje sostenido por el replay/bot, y grabación del eje del jugador cuando cambia.
	if (ScriptedMoveAxis != 0.f)
	{
		AddMovementInput(FVector::ForwardVector, ScriptedMoveAxis);
	}
	if (PlaytestRunner.IsValid())
	{
		const float Axis = FMath::RoundToFloat(PlayerMoveAxis * 10.f) / 10.f;
		if (Axis != RecordedMoveAxis)
		{
			RecordedMoveAxis = Axis;
			PlaytestRunner->NotifyAction(ETDCPlayAction::Move, Axis, GetActorLocation());
		}
	}
	if (bAutoRun)
	{
		TickAutoRun(DeltaSeconds);
	}
	if (GetCharacterMovement() && GetCharacterMovement()->IsMovingOnGround() && GetWorld())
	{
		LastGroundedTime = GetWorld()->GetTimeSeconds();
	}
	UpdateAnimState();
	ApplyClapPunch();
	UpdateLightPrompt();
	ShowControls();
}

void ATDCCharacter::TickAutoRun(float DeltaSeconds)
{
	if (AutoRunTime == 0.f && CameraBoom)
	{
		// demo: más cerca para que la pose se lea, pero detrás de la utilería de primer plano (y 550–900): con 900 la
		// cámara la atravesaba y las capturas de la galería salían tapadas (auditoría visual #10).
		CameraBoom->TargetArmLength = 1300.f;
		CameraBoom->TargetOffset = FVector::ZeroVector; // con el brazo corto, el offset de anticipación lo dejaba fuera de cuadro
	}
	AutoRunTime += DeltaSeconds;
	// Claquetazo cada 0.6 s: limpia al extra común de Beat 1 (si no, lo toca y reaparece al inicio).
	if (Director && FMath::Fmod(AutoRunTime, 0.6f) < DeltaSeconds)
	{
		Director->Clap();
	}
	// 0-1 s quieto (idle), después corre a la derecha y salta cada 2.5 s, hasta los 12 s.
	if (AutoRunTime > 1.f && AutoRunTime < 12.f)
	{
		AddMovementInput(FVector::ForwardVector, 1.f);
	}
	if (AutoRunTime > NextAutoJump && AutoRunTime < 12.f)
	{
		Jump();
		NextAutoJump += 2.5f;
	}
	else if (AutoRunTime > NextAutoJump - 2.3f)
	{
		StopJumping();
	}
	if (AutoRunTime > NextAutoShot && AutoShotIndex < 24)
	{
		FScreenshotRequest::RequestScreenshot(FString::Printf(TEXT("TDCAutoRun_%02d"), AutoShotIndex++), false, false);
		NextAutoShot += 0.5f;
	}
}

void ATDCCharacter::ShowControls()
{
	// Panel de controles del HUD (ATDCHUD): se pide cada frame mientras se juega.
	if (!IsPlayerControlled())
	{
		return;
	}
	if (UTDCHudMessages* Hud = GetWorld() ? GetWorld()->GetSubsystem<UTDCHudMessages>() : nullptr)
	{
		Hud->RequestControls();
	}
}

void ATDCCharacter::UpdateLightPrompt()
{
	// Legibilidad: si hay un reflector al alcance, decir qué tecla lo dirige.
	if (!IsPlayerControlled() || !Director || !Director->HasPower(ETDCPower::Lights))
	{
		return;
	}
	if (const ATDCStageLight* Light = Director->FindNearestLight())
	{
		TDC::HudMessage(this, 7003, Light->IsOn() ? TEXT("[E]  Apagar el reflector") : TEXT("[E]  ¡Luces! Prender el reflector"),
			FLinearColor(1.f, 0.82f, 0.25f), 0.15f, ETDCHudSlot::Prompt);
	}
}

void ATDCCharacter::UpdateAnimState()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp || MeshComp->GetAnimationMode() != EAnimationMode::AnimationSingleNode)
	{
		return;
	}
	const FVector Velocity = GetVelocity();
	const float Speed = FVector(Velocity.X, Velocity.Y, 0.f).Size();
	const bool bInAir = GetCharacterMovement() && GetCharacterMovement()->IsFalling();

	// Estado objetivo. Con blendspace, idle/walk/run/sprint son un solo estado que mezcla por velocidad (sin cortes).
	EAnimState NewState = EAnimState::Idle;
	UAnimSequence* LoopAnim = IdleAnim;
	float NativeSpeed = 0.f;
	if (ClapAnim && GetWorld() && GetWorld()->GetTimeSeconds() < ClapAnimUntil)
	{
		NewState = EAnimState::Clap;
	}
	else if (bInAir && JumpAnim)
	{
		NewState = Velocity.Z > 0.f ? EAnimState::JumpRise : EAnimState::JumpFall;
	}
	else if (LocomotionBlendSpace)
	{
		NewState = EAnimState::Locomotion;
	}
	else if (Speed > RunSpeedThreshold)
	{
		// Histéresis simple por umbrales intermedios entre las velocidades nativas de cada clip.
		if (SprintAnim && Speed > 0.5f * (RunAnimSpeed + SprintAnimSpeed))
		{
			NewState = EAnimState::Sprint; LoopAnim = SprintAnim; NativeSpeed = SprintAnimSpeed;
		}
		else if (RunAnim && (!WalkAnim || Speed > 0.5f * (WalkAnimSpeed + RunAnimSpeed)))
		{
			NewState = EAnimState::Run; LoopAnim = RunAnim; NativeSpeed = RunAnimSpeed;
		}
		else if (WalkAnim)
		{
			NewState = EAnimState::Walk; LoopAnim = WalkAnim; NativeSpeed = WalkAnimSpeed;
		}
	}

	if (NewState != AnimState)
	{
		AnimState = NewState;
		switch (NewState)
		{
		case EAnimState::Clap:
		{
			// Golpe seco: arranca en el tramo de impacto y el resto del clip entra en ClapDuration (≤300 ms).
			const float Length = static_cast<float>(ClapAnim->GetPlayLength());
			const float Start = Length * ClapStartFraction;
			MeshComp->PlayAnimation(ClapAnim, /*bLooping*/ false);
			MeshComp->SetPosition(Start, /*bFireNotifies*/ false);
			MeshComp->SetPlayRate(FMath::Max(1.f, (Length - Start) / ClapDuration));
			break;
		}
		case EAnimState::JumpRise:
		case EAnimState::JumpFall:
		{
			// Pose fija del salto: el clip de Rodin anticipa en el piso, así que se congela en el tramo aéreo.
			MeshComp->PlayAnimation(JumpAnim, /*bLooping*/ false);
			const float Fraction = NewState == EAnimState::JumpRise ? JumpRisePose : JumpFallPose;
			MeshComp->SetPosition(static_cast<float>(JumpAnim->GetPlayLength()) * Fraction, /*bFireNotifies*/ false);
			MeshComp->Stop();
			break;
		}
		case EAnimState::Locomotion:
			MeshComp->PlayAnimation(LocomotionBlendSpace, /*bLooping*/ true);
			MeshComp->SetPlayRate(1.f);
			break;
		default:
			if (LoopAnim)
			{
				MeshComp->PlayAnimation(LoopAnim, /*bLooping*/ true);
			}
			MeshComp->SetPlayRate(1.f);
			break;
		}
	}

	if (AnimState == EAnimState::Locomotion)
	{
		if (UAnimSingleNodeInstance* Single = MeshComp->GetSingleNodeInstance())
		{
			Single->SetBlendSpacePosition(FVector(Speed, 0.f, 0.f));
		}
	}
	else if (NativeSpeed > 0.f)
	{
		// Los pies acompañan la velocidad real: menos patinaje.
		MeshComp->SetPlayRate(RunPlayRate * FMath::Clamp(Speed / NativeSpeed, 0.5f, 1.5f));
	}
}

bool ATDCCharacter::HasCharacterMesh() const
{
	return GetMesh() && GetMesh()->GetSkeletalMeshAsset() != nullptr;
}

void ATDCCharacter::UpdatePlaceholderVisibility()
{
	// Con mesh final, el greybox (cilindro + nariz) se oculta.
	const bool bShowPlaceholder = !HasCharacterMesh();
	Body->SetVisibility(bShowPlaceholder);
	Nose->SetVisibility(bShowPlaceholder);
}

void ATDCCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdatePlaceholderVisibility();
}

void ATDCCharacter::BeginPlay()
{
	Super::BeginPlay();
	RespawnLocation = GetActorLocation();
	bAutoRun = IsPlayerControlled() && FParse::Param(FCommandLine::Get(), TEXT("TDCAutoRun"));
	const ETDCPlaytestMode PlaytestMode = IsPlayerControlled() ? ATDCPlaytestRunner::ModeFromCommandLine() : ETDCPlaytestMode::None;
	if (PlaytestMode != ETDCPlaytestMode::None && GetWorld())
	{
		FActorSpawnParameters Params;
		Params.Owner = this;
		if (ATDCPlaytestRunner* Runner = GetWorld()->SpawnActor<ATDCPlaytestRunner>(Params))
		{
			Runner->Init(this, PlaytestMode);
			PlaytestRunner = Runner;
		}
		if (PlaytestMode != ETDCPlaytestMode::Record && CameraBoom)
		{
			// Replay y bot: cámara cerca y centrada, para que las capturas se lean (como -TDCAutoRun).
			CameraBoom->TargetArmLength = 900.f;
			CameraBoom->TargetOffset = FVector::ZeroVector;
		}
	}
	if (IsPlayerControlled())
	{
		// "¡Acción!" lo dice ATDCPlayerController al empezar la toma (después del menú, T-0035).
		TDC::PlaySfx2D(this, TEXT("amb_lluvia_ciudad"), 0.25f);  // ambiente (loop desde import_audio.py)
	}
	LoadCharacterAssets();
	UpdatePlaceholderVisibility();

	if (UMaterialInstanceDynamic* Mat = HasCharacterMesh() ? nullptr : TDC::CreateReadColorMID(Body))
	{
		Mat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.9f, 0.25f, 0.2f));
	}

	if (GEngine && IsPlayerControlled())
	{
		// Los controles se muestran siempre desde Tick (ShowControls).
	}
}

void ATDCCharacter::BuildDefaultInput()
{
	auto MakeAction = [this](TObjectPtr<UInputAction>& Action, const TCHAR* Name, EInputActionValueType Type)
	{
		if (!Action)
		{
			Action = NewObject<UInputAction>(this, Name);
			Action->ValueType = Type;
		}
	};
	MakeAction(MoveAction, TEXT("IA_Move_Default"), EInputActionValueType::Axis1D);
	MakeAction(JumpAction, TEXT("IA_Jump_Default"), EInputActionValueType::Boolean);
	MakeAction(LightsAction, TEXT("IA_Lights_Default"), EInputActionValueType::Boolean);
	MakeAction(RewindAction, TEXT("IA_Rewind_Default"), EInputActionValueType::Boolean);
	MakeAction(ClapAction, TEXT("IA_Clap_Default"), EInputActionValueType::Boolean);

	if (MappingContext)
	{
		return;
	}
	MappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Default"));

	auto MapNegated = [this](UInputAction* Action, const FKey& Key)
	{
		FEnhancedActionKeyMapping& Mapping = MappingContext->MapKey(Action, Key);
		Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(MappingContext));
	};

	MappingContext->MapKey(MoveAction, EKeys::D);
	MappingContext->MapKey(MoveAction, EKeys::Right);
	MappingContext->MapKey(MoveAction, EKeys::Gamepad_LeftX);
	MapNegated(MoveAction, EKeys::A);
	MapNegated(MoveAction, EKeys::Left);

	MappingContext->MapKey(JumpAction, EKeys::SpaceBar);
	MappingContext->MapKey(JumpAction, EKeys::Gamepad_FaceButton_Bottom);

	MappingContext->MapKey(LightsAction, EKeys::E);
	MappingContext->MapKey(LightsAction, EKeys::Gamepad_FaceButton_Left);

	MappingContext->MapKey(RewindAction, EKeys::Q);
	MappingContext->MapKey(RewindAction, EKeys::Gamepad_LeftTrigger);

	MappingContext->MapKey(ClapAction, EKeys::F);
	MappingContext->MapKey(ClapAction, EKeys::LeftMouseButton);
	MappingContext->MapKey(ClapAction, EKeys::Gamepad_FaceButton_Right);
}

void ATDCCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	BuildDefaultInput();

	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MappingContext, 0);
		}
	}

	UEnhancedInputComponent* Input = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATDCCharacter::Move);
	Input->BindAction(MoveAction, ETriggerEvent::Completed, this, &ATDCCharacter::MoveCompleted);
	Input->BindAction(JumpAction, ETriggerEvent::Started, this, &ATDCCharacter::OnJumpPressed);
	Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &ATDCCharacter::OnJumpReleased);
	Input->BindAction(LightsAction, ETriggerEvent::Started, this, &ATDCCharacter::OnLightsPressed);
	Input->BindAction(RewindAction, ETriggerEvent::Started, this, &ATDCCharacter::OnRewindPressed);
	Input->BindAction(RewindAction, ETriggerEvent::Completed, this, &ATDCCharacter::OnRewindReleased);
	Input->BindAction(RewindAction, ETriggerEvent::Canceled, this, &ATDCCharacter::OnRewindReleased);
	Input->BindAction(ClapAction, ETriggerEvent::Started, this, &ATDCCharacter::OnClap);
}

void ATDCCharacter::MoveCompleted() { PlayerMoveAxis = 0.f; }
void ATDCCharacter::OnJumpPressed() { PerformAction(ETDCPlayAction::Jump, 1.f); }
void ATDCCharacter::OnJumpReleased() { PerformAction(ETDCPlayAction::Jump, 0.f); }
void ATDCCharacter::OnLightsPressed() { PerformAction(ETDCPlayAction::Lights, 1.f); }
void ATDCCharacter::OnRewindPressed() { PerformAction(ETDCPlayAction::Rewind, 1.f); }
void ATDCCharacter::OnRewindReleased() { PerformAction(ETDCPlayAction::Rewind, 0.f); }
void ATDCCharacter::OnClap() { PerformAction(ETDCPlayAction::Clap, 1.f); }

void ATDCCharacter::PerformAction(ETDCPlayAction Action, float Value)
{
	switch (Action)
	{
	case ETDCPlayAction::Move:
		ScriptedMoveAxis = Value; // solo replay/bot: el jugador mueve con Move()
		break;
	case ETDCPlayAction::Jump:
		if (Value > 0.5f) { Jump(); } else { StopJumping(); }
		break;
	case ETDCPlayAction::Lights:
		if (Director) { Director->ToggleNearestLight(); }
		break;
	case ETDCPlayAction::Rewind:
		if (Director) { if (Value > 0.5f) { Director->BeginRewind(); } else { Director->EndRewind(); } }
		break;
	case ETDCPlayAction::Clap:
		if (PlaytestRunner.IsValid())
		{
			PlaytestRunner->NotifyPreClap(GetActorLocation()); // antes del golpe: después el común ya no cuenta
		}
		Value = DoClap() ? 1.f : 0.f; // 0 = en cooldown, no hubo golpe
		break;
	case ETDCPlayAction::Teleport:
		break; // PlaytestTeleport
	}
	if (PlaytestRunner.IsValid())
	{
		PlaytestRunner->NotifyAction(Action, Value, GetActorLocation());
	}
}

void ATDCCharacter::PlaytestTeleport(const FVector& Location, float FacingX)
{
	GetCharacterMovement()->StopMovementImmediately();
	SetActorLocationAndRotation(Location, FRotator(0.f, FacingX >= 0.f ? 0.f : 180.f, 0.f), false, nullptr,
		ETeleportType::TeleportPhysics);
	RespawnLocation = Location; // si algo lo tira, vuelve al beat actual y no al inicio
	if (PlaytestRunner.IsValid())
	{
		PlaytestRunner->NotifyAction(ETDCPlayAction::Teleport, FacingX, Location);
	}
}

bool ATDCCharacter::DoClap()
{
	if (!Director || !Director->Clap())
	{
		return false;
	}
	if (GetWorld())
	{
		ClapPunchStart = GetWorld()->GetTimeSeconds();
	}
	if (ClapAnim && GetWorld())
	{
		ClapAnimUntil = GetWorld()->GetTimeSeconds() + ClapDuration;
		AnimState = EAnimState::None; // fuerza el cambio de estado en el próximo UpdateAnimState
		UpdateAnimState();
	}
	return true;
}

void ATDCCharacter::Move(const FInputActionValue& Value)
{
	PlayerMoveAxis = Value.Get<float>();
	AddMovementInput(FVector::ForwardVector, PlayerMoveAxis);
}

void ATDCCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();
	bJumpedSinceGrounded = true;
	TDC::PlaySfx(this, TEXT("sfx_jump"), GetActorLocation(), 0.6f);
}

void ATDCCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	TDC::PlaySfx(this, TEXT("sfx_land"), GetActorLocation(), 0.6f);
	bJumpedSinceGrounded = false;
	// Jump buffer: si apretó saltar justo antes de tocar el piso (y ya lo soltó), saltar ahora.
	if (!bPressedJump && GetWorld() && GetWorld()->GetTimeSeconds() - LastJumpPressTime <= JumpBufferTime)
	{
		Super::Jump();
		FTimerHandle Release;
		GetWorldTimerManager().SetTimer(Release, FTimerDelegate::CreateWeakLambda(this, [this]() { StopJumping(); }), 0.05f, false);
	}
	if (const AActor* Floor = Hit.GetActor())
	{
		if (Floor->ActorHasTag(TEXT("TDC.Safe")))
		{
			RespawnLocation = GetActorLocation();
		}
	}
}

void ATDCCharacter::Respawn()
{
	GetCharacterMovement()->StopMovementImmediately();
	SetActorLocation(RespawnLocation, false, nullptr, ETeleportType::ResetPhysics);
}

void ATDCCharacter::FellOutOfWorld(const UDamageType& DamageType)
{
	// No destruir al protagonista: volver al último punto seguro.
	Respawn();
}

void ATDCCharacter::ApplyClapPunch()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp || !GetWorld())
	{
		return;
	}
	const float Phase = static_cast<float>(GetWorld()->GetTimeSeconds() - ClapPunchStart) / FMath::Max(ClapDuration, 0.05f);
	const float Punch = (Phase >= 0.f && Phase < 1.f) ? FMath::Sin(Phase * PI) : 0.f;
	if (Punch <= 0.f && !bClapPunchActive)
	{
		return;
	}
	bClapPunchActive = Punch > 0.f;
	// Se ensancha y se agacha en el golpe (squash), y vuelve: el claquetazo se lee aunque la pose del clip sea sutil.
	MeshComp->SetRelativeScale3D(FVector(AnimatedMeshScale) * FVector(1.f + 0.1f * Punch, 1.f + 0.1f * Punch, 1.f - 0.08f * Punch));
	if (Clapper && Clapper->IsVisible())
	{
		// La claqueta "cierra": giro seco y vuelta.
		FTransform Snap = ClapperOffset;
		Snap.ConcatenateRotation(FRotator(0.f, 0.f, -35.f * Punch).Quaternion());
		Clapper->SetRelativeTransform(Snap);
	}
}

void ATDCCharacter::Jump()
{
	if (GetWorld())
	{
		LastJumpPressTime = GetWorld()->GetTimeSeconds();
	}
	Super::Jump();
}

bool ATDCCharacter::CanJumpInternal_Implementation() const
{
	if (Super::CanJumpInternal_Implementation())
	{
		return true;
	}
	// Coyote time: cayendo por haber salido de un borde (sin haber saltado), unos ms todavía cuenta como "en el piso".
	const UCharacterMovementComponent* Move = GetCharacterMovement();
	return Move && Move->IsFalling() && !bJumpedSinceGrounded && GetWorld()
		&& GetWorld()->GetTimeSeconds() - LastGroundedTime <= CoyoteTime;
}
