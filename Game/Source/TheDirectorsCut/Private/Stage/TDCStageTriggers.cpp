#include "Stage/TDCStageTriggers.h"
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
