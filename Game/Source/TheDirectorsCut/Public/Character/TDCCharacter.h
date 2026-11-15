#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TDCCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class USkeletalMesh;
class UAnimSequence;
class UBlendSpace1D;
class UInputAction;
class UInputMappingContext;
class UTDCDirectorComponent;
struct FInputActionValue;

/**
 * Protagonista 2.5D: se mueve en el plano XZ (X = horizontal, Z = vertical), cámara lateral fija mirando desde +Y.
 * Si no se asignan assets de input, construye un mapeo por defecto en C++ (teclado + gamepad).
 */
UCLASS()
class THEDIRECTORSCUT_API ATDCCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATDCCharacter();

	/** Vuelve al último piso seguro (tag TDC.Safe) o al punto de inicio. */
	UFUNCTION(BlueprintCallable, Category = "TDC|Character")
	void Respawn();

	UTDCDirectorComponent* GetDirector() const { return Director; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void FellOutOfWorld(const UDamageType& DamageType) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TDC|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TDC|Camera")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TDC|Character")
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TDC|Character")
	TObjectPtr<UStaticMeshComponent> Nose;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TDC|Director")
	TObjectPtr<UTDCDirectorComponent> Director;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TDC|Character")
	TObjectPtr<UStaticMeshComponent> Clapper;

	/**
	 * Mesh y animaciones finales del director. Se cargan por ruta en BeginPlay; si no existen, queda el greybox.
	 * Sin AnimBP: el estado (Idle / Run / Jump) se elige en Tick y se reproduce en modo Single Node.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "TDC|Character")
	FSoftObjectPath CharacterMeshPath = FSoftObjectPath(TEXT("/Game/Characters/Director/SK_Director.SK_Director"));

	UPROPERTY(EditDefaultsOnly, Category = "TDC|Character")
	FSoftObjectPath IdleAnimPath = FSoftObjectPath(TEXT("/Game/Characters/Director/AS_Director_Idle.AS_Director_Idle"));

	UPROPERTY(EditDefaultsOnly, Category = "TDC|Character")
	FSoftObjectPath RunAnimPath = FSoftObjectPath(TEXT("/Game/Characters/Director/AS_Director_Run.AS_Director_Run"));

	/** Golpe con la claqueta (opcional: sin animación, el claquetazo funciona igual). */
	UPROPERTY(EditDefaultsOnly, Category = "TDC|Character")
	FSoftObjectPath ClapAnimPath = FSoftObjectPath(TEXT("/Game/Characters/Director/AS_Director_Clap.AS_Director_Clap"));

	/** Claqueta en la mano (opcional). Se engancha al socket/hueso ClapperSocket del mesh. */
	UPROPERTY(EditDefaultsOnly, Category = "TDC|Character")
	FSoftObjectPath ClapperMeshPath = FSoftObjectPath(TEXT("/Game/Characters/Director/SM_Clapperboard.SM_Clapperboard"));

	UPROPERTY(EditDefaultsOnly, Category = "TDC|Character")
	FName ClapperSocket = TEXT("RightHand");

	/** Ajuste de la claqueta respecto del hueso de la mano (ubicación en cm, rotación, escala). Se edita sin recompilar. */
	UPROPERTY(EditDefaultsOnly, Category = "TDC|Character")
	FTransform ClapperOffset = FTransform::Identity;

	UPROPERTY(EditDefaultsOnly, Category = "TDC|Character")
	FSoftObjectPath WalkAnimPath = FSoftObjectPath(TEXT("/Game/Characters/Director/AS_Director_Walk.AS_Director_Walk"));

	UPROPERTY(EditDefaultsOnly, Category = "TDC|Character")
	FSoftObjectPath SprintAnimPath = FSoftObjectPath(TEXT("/Game/Characters/Director/AS_Director_Sprint.AS_Director_Sprint"));

	/** Si existe (fix_director_look.py), la locomoción usa este blendspace por velocidad: transiciones suaves. */
	UPROPERTY(EditDefaultsOnly, Category = "TDC|Character")
	FSoftObjectPath LocomotionBlendSpacePath = FSoftObjectPath(TEXT("/Game/Characters/Director/BS_Director_Locomotion.BS_Director_Locomotion"));

	/** Velocidad (cm/s) a la que cada animación no patina: el play rate se escala con Speed / esta velocidad. */
	UPROPERTY(EditDefaultsOnly, Category = "TDC|Character")
	float WalkAnimSpeed = 160.f;

	UPROPERTY(EditDefaultsOnly, Category = "TDC|Character")
	float RunAnimSpeed = 380.f;

	UPROPERTY(EditDefaultsOnly, Category = "TDC|Character")
	float SprintAnimSpeed = 600.f;

	/** Pose de salto fija (fracción de AS_Director_Jump) subiendo y cayendo; evita el anticipo en el piso del clip. */
	UPROPERTY(EditDefaultsOnly, Category = "TDC|Character")
	float JumpRisePose = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category = "TDC|Character")
	float JumpFallPose = 0.6f;

	UPROPERTY(EditDefaultsOnly, Category = "TDC|Character")
	FSoftObjectPath JumpAnimPath = FSoftObjectPath(TEXT("/Game/Characters/Director/AS_Director_Jump.AS_Director_Jump"));

	/**
	 * Escala del mesh cargado por ruta. Con los FBX re-exportados en cm (T-0012) vale 1. Si un FBX trae escala 0.01 en la pista del root
	 * ("Armature"), 100 la compensa.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "TDC|Character")
	float AnimatedMeshScale = 1.f;

	/** Rotación del mesh respecto de la cápsula (el FBX mira a -Y; el personaje avanza en +X). */
	UPROPERTY(EditDefaultsOnly, Category = "TDC|Character")
	float MeshYaw = -90.f;

	/** Velocidad horizontal (cm/s) a partir de la cual se reproduce Run. */
	UPROPERTY(EditDefaultsOnly, Category = "TDC|Character")
	float RunSpeedThreshold = 10.f;

	/** Velocidad de reproducción de Run a MaxWalkSpeed (para que los pies no patinen; ajustar en playtest). */
	UPROPERTY(EditDefaultsOnly, Category = "TDC|Character")
	float RunPlayRate = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "TDC|Input")
	TObjectPtr<UInputMappingContext> MappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "TDC|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "TDC|Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "TDC|Input")
	TObjectPtr<UInputAction> LightsAction;

	UPROPERTY(EditDefaultsOnly, Category = "TDC|Input")
	TObjectPtr<UInputAction> RewindAction;

	UPROPERTY(EditDefaultsOnly, Category = "TDC|Input")
	TObjectPtr<UInputAction> ClapAction;

private:
	enum class EAnimState : uint8 { None, Idle, Walk, Run, Sprint, Locomotion, JumpRise, JumpFall, Clap };

	void UpdateLightPrompt();

	/** Modo demo para evidencia sin jugador (-TDCAutoRun): corre a la derecha, salta cada 2.5 s y captura 1 frame/s. */
	void TickAutoRun(float DeltaSeconds);
	bool bAutoRun = false;
	float AutoRunTime = 0.f;
	float NextAutoJump = 1.5f;
	float NextAutoShot = 0.5f;
	int32 AutoShotIndex = 0;

	void OnClap();

	void LoadCharacterAssets();
	void UpdateAnimState();
	bool HasCharacterMesh() const;
	void UpdatePlaceholderVisibility();
	void BuildDefaultInput();
	void Move(const FInputActionValue& Value);

	FVector RespawnLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> IdleAnim;

	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> RunAnim;

	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> JumpAnim;

	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> WalkAnim;

	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> SprintAnim;

	UPROPERTY(Transient)
	TObjectPtr<UBlendSpace1D> LocomotionBlendSpace;

	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> ClapAnim;

	/** Hasta cuándo (segundos de mundo) la animación del claquetazo tiene prioridad. */
	double ClapAnimUntil = 0.0;

	EAnimState AnimState = EAnimState::None;
};
