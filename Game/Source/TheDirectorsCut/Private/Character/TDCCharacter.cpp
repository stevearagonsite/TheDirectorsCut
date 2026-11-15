#include "Character/TDCCharacter.h"
#include "Core/TDCVisuals.h"
#include "Director/TDCDirectorComponent.h"
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
	CameraBoom->SetRelativeRotation(FRotator(-5.f, -90.f, 0.f));
	CameraBoom->TargetArmLength = 2200.f; // D-012: encuadre amplio para anticipar el tramo que viene
	CameraBoom->SocketOffset = FVector(0.f, 0.f, 120.f);
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
	LocomotionBlendSpace = LoadQuiet<UBlendSpace1D>(LocomotionBlendSpacePath);
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
	if (bAutoRun)
	{
		TickAutoRun(DeltaSeconds);
	}
	UpdateAnimState();
	UpdateLightPrompt();
	ShowControls();
}

void ATDCCharacter::TickAutoRun(float DeltaSeconds)
{
	AutoRunTime += DeltaSeconds;
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
	if (AutoRunTime > NextAutoShot && AutoShotIndex < 12)
	{
		FScreenshotRequest::RequestScreenshot(FString::Printf(TEXT("TDCAutoRun_%02d"), AutoShotIndex++), false, false);
		NextAutoShot += 1.f;
	}
}

void ATDCCharacter::ShowControls()
{
	// Arriba a la izquierda, siempre: los mensajes con clave se reemplazan cada frame (no se acumulan).
	if (!GEngine || !IsPlayerControlled())
	{
		return;
	}
	const FColor Ink(235, 235, 235);
	GEngine->AddOnScreenDebugMessage(7010, 0.f, Ink, TEXT("CONTROLES (teclado / gamepad)"));
	GEngine->AddOnScreenDebugMessage(7011, 0.f, Ink, TEXT("  A / D ........ moverse        (stick)"));
	GEngine->AddOnScreenDebugMessage(7012, 0.f, Ink, TEXT("  Espacio ...... saltar         (A)"));
	GEngine->AddOnScreenDebugMessage(7013, 0.f, Ink, TEXT("  F / clic ..... claquetazo     (B)"));
	GEngine->AddOnScreenDebugMessage(7014, 0.f, Ink, TEXT("  E ............ prender/apagar el reflector cercano  (X)"));
	GEngine->AddOnScreenDebugMessage(7015, 0.f, Ink, TEXT("  Q (mantener) . rebobinar lo celeste  (gatillo izq.)"));
	GEngine->AddOnScreenDebugMessage(7016, 0.f, FColor(150, 150, 150),
		TEXT("  Amarillo = existe con luz · Violeta = a oscuras · Celeste = se rebobina · Rojo/naranja/negro = extras"));
}

void ATDCCharacter::UpdateLightPrompt()
{
	// Legibilidad: si hay un reflector al alcance, decir qué tecla lo dirige.
	if (!GEngine || !IsPlayerControlled() || !Director || !Director->HasPower(ETDCPower::Lights))
	{
		return;
	}
	if (const ATDCStageLight* Light = Director->FindNearestLight())
	{
		GEngine->AddOnScreenDebugMessage(7003, 0.15f, FColor(255, 220, 90),
			Light->IsOn() ? TEXT("[E] ¡Luces!: apagar el reflector") : TEXT("[E] ¡Luces!: prender el reflector"));
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
			MeshComp->PlayAnimation(ClapAnim, /*bLooping*/ false);
			MeshComp->SetPlayRate(1.f);
			break;
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
	Input->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
	Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	Input->BindAction(LightsAction, ETriggerEvent::Started, Director.Get(), &UTDCDirectorComponent::ToggleNearestLight);
	Input->BindAction(RewindAction, ETriggerEvent::Started, Director.Get(), &UTDCDirectorComponent::BeginRewind);
	Input->BindAction(RewindAction, ETriggerEvent::Completed, Director.Get(), &UTDCDirectorComponent::EndRewind);
	Input->BindAction(RewindAction, ETriggerEvent::Canceled, Director.Get(), &UTDCDirectorComponent::EndRewind);
	Input->BindAction(ClapAction, ETriggerEvent::Started, this, &ATDCCharacter::OnClap);
}

void ATDCCharacter::OnClap()
{
	if (!Director || !Director->Clap())
	{
		return;
	}
	if (ClapAnim && GetWorld())
	{
		ClapAnimUntil = GetWorld()->GetTimeSeconds() + FMath::Max(0.2f, static_cast<float>(ClapAnim->GetPlayLength()));
		AnimState = EAnimState::None; // fuerza el cambio de estado en el próximo UpdateAnimState
		UpdateAnimState();
	}
}

void ATDCCharacter::Move(const FInputActionValue& Value)
{
	AddMovementInput(FVector::ForwardVector, Value.Get<float>());
}

void ATDCCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
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
