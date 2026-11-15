#include "Stage/TDCStageTriggers.h"
#include "Engine/Engine.h"
#include "Core/TDCVisuals.h"
#include "Stage/TDCSetPiece.h"
#include "Character/TDCCharacter.h"
#include "Components/BoxComponent.h"

ATDCFallTrigger::ATDCFallTrigger()
{
	PrimaryActorTick.bCanEverTick = false;
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetBoxExtent(FVector(100.f, 200.f, 200.f));
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Box->SetGenerateOverlapEvents(true);
	RootComponent = Box;
	Box->OnComponentBeginOverlap.AddDynamic(this, &ATDCFallTrigger::OnOverlap);
}

void ATDCFallTrigger::OnOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (bFired || !Cast<ATDCCharacter>(OtherActor))
	{
		return;
	}
	bFired = true;
	for (ATDCSetPiece* Piece : Targets)
	{
		if (Piece)
		{
			Piece->Release();
		}
	}
	TDC::PlaySfx(this, TEXT("sfx_bridge_fall"), GetActorLocation(), 0.9f);
	// Legibilidad: el puente cae a propósito; hay que decir cómo se dirige la escena.
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(7020, 6.f, FColor(110, 200, 255),
			TEXT("¡Se cayó el puente! Lo celeste está \"en toma\": mantené Q para rebobinarlo y cruzá."));
	}
}

ATDCRespawnVolume::ATDCRespawnVolume()
{
	PrimaryActorTick.bCanEverTick = false;
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetBoxExtent(FVector(500.f, 300.f, 100.f));
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Box->SetGenerateOverlapEvents(true);
	RootComponent = Box;
	Box->OnComponentBeginOverlap.AddDynamic(this, &ATDCRespawnVolume::OnOverlap);
}

void ATDCRespawnVolume::OnOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (ATDCCharacter* Character = Cast<ATDCCharacter>(OtherActor))
	{
		Character->Respawn();
	}
}
