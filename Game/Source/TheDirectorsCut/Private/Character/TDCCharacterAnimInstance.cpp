#include "Character/TDCCharacterAnimInstance.h"
#include "Character/TDCCharacter.h"
#include "Director/TDCDirectorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

void UTDCCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	const ATDCCharacter* Character = Cast<ATDCCharacter>(TryGetPawnOwner());
	if (!Character)
	{
		return;
	}
	const FVector Velocity = Character->GetVelocity();
	Speed = FVector(Velocity.X, Velocity.Y, 0.f).Size();
	VerticalSpeed = Velocity.Z;
	bIsInAir = Character->GetCharacterMovement() && Character->GetCharacterMovement()->IsFalling();
	bIsRewinding = Character->GetDirector() && Character->GetDirector()->IsRewinding();
}
