#include "Character/TDCCharacter.h"
#include "Director/TDCDirectorComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
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

	Director = CreateDefaultSubobject<UTDCDirectorComponent>(TEXT("Director"));
}

void ATDCCharacter::BeginPlay()
{
	Super::BeginPlay();
	RespawnLocation = GetActorLocation();

	if (UMaterialInstanceDynamic* Mat = Body->CreateDynamicMaterialInstance(0))
	{
		Mat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.9f, 0.25f, 0.2f));
	}

	if (GEngine && IsPlayerControlled())
	{
		GEngine->AddOnScreenDebugMessage(7000, 15.f, FColor::White,
			TEXT("A/D mover  ·  Espacio saltar  ·  E luces  ·  Q (mantener) rebobinar"));
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
