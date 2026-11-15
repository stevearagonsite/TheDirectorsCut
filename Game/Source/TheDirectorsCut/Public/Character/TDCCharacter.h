#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TDCCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
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
	bool HasCharacterMesh() const;
	void UpdatePlaceholderVisibility();
	void BuildDefaultInput();
	void Move(const FInputActionValue& Value);

	FVector RespawnLocation = FVector::ZeroVector;
};
