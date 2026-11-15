#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TDCCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class USkeletalMesh;
class UAnimSequence;
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

	UPROPERTY(EditDefaultsOnly, Category = "TDC|Character")
	FSoftObjectPath JumpAnimPath = FSoftObjectPath(TEXT("/Game/Characters/Director/AS_Director_Jump.AS_Director_Jump"));

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

private:
	enum class EAnimState : uint8 { None, Idle, Run, Jump };

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

	EAnimState AnimState = EAnimState::None;
};
