#include "Character/TDCCharacter.h"
#include "Core/TDCVisuals.h"
#include "Director/TDCDirectorComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
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
	UpdateAnimState();
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

	EAnimState NewState = EAnimState::Idle;
	if (ClapAnim && GetWorld() && GetWorld()->GetTimeSeconds() < ClapAnimUntil)
	{
		NewState = EAnimState::Clap;
	}
	else if (bInAir && JumpAnim)
	{
		NewState = EAnimState::Jump;
	}
	else if (Speed > RunSpeedThreshold && RunAnim)
	{
		NewState = EAnimState::Run;
	}

	if (NewState == EAnimState::Run && RunAnim)
	{
		const float MaxSpeed = GetCharacterMovement() ? FMath::Max(1.f, GetCharacterMovement()->MaxWalkSpeed) : 600.f;
		MeshComp->SetPlayRate(RunPlayRate * FMath::Clamp(Speed / MaxSpeed, 0.3f, 1.2f));
	}
	if (NewState == AnimState)
	{
		return;
	}
	AnimState = NewState;
	switch (NewState)
	{
	case EAnimState::Clap:
		MeshComp->PlayAnimation(ClapAnim, /*bLooping*/ false);
		MeshComp->SetPlayRate(1.f);
		break;
	case EAnimState::Jump:
		MeshComp->PlayAnimation(JumpAnim, /*bLooping*/ false);
		break;
	case EAnimState::Run:
		MeshComp->PlayAnimation(RunAnim, /*bLooping*/ true);
		break;
	default:
		if (IdleAnim)
		{
			MeshComp->PlayAnimation(IdleAnim, /*bLooping*/ true);
		}
		MeshComp->SetPlayRate(1.f);
		break;
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
	LoadCharacterAssets();
	UpdatePlaceholderVisibility();

	if (UMaterialInstanceDynamic* Mat = HasCharacterMesh() ? nullptr : TDC::CreateReadColorMID(Body))
	{
		Mat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.9f, 0.25f, 0.2f));
	}

	if (GEngine && IsPlayerControlled())
	{
		GEngine->AddOnScreenDebugMessage(7000, 15.f, FColor::White,
			TEXT("A/D mover  ·  Espacio saltar  ·  F claquetazo  ·  E luces  ·  Q (mantener) rebobinar"));
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
