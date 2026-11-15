#include "Director/TDCDirectorComponent.h"
#include "Stage/TDCStageLight.h"
#include "Stage/TDCStageSubsystem.h"
#include "TheDirectorsCut.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UTDCDirectorComponent::UTDCDirectorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

ATDCStageLight* UTDCDirectorComponent::FindNearestLight() const
{
	const UTDCStageSubsystem* Stage = GetWorld()->GetSubsystem<UTDCStageSubsystem>();
	if (!Stage || !GetOwner())
	{
		return nullptr;
	}

	const FVector From = GetOwner()->GetActorLocation();
	ATDCStageLight* Best = nullptr;
	float BestDistSq = FMath::Square(LightReach);
	for (const TWeakObjectPtr<ATDCStageLight>& Light : Stage->GetLights())
	{
		if (!Light.IsValid())
		{
			continue;
		}
		const float DistSq = FVector::DistSquared(From, Light->GetActorLocation());
		if (DistSq <= BestDistSq)
		{
			BestDistSq = DistSq;
			Best = Light.Get();
		}
	}
	return Best;
}

void UTDCDirectorComponent::ToggleNearestLight()
{
	if (!HasPower(ETDCPower::Lights))
	{
		return;
	}
	if (ATDCStageLight* Light = FindNearestLight())
	{
		Light->Toggle();
		UE_LOG(LogTDC, Log, TEXT("Luces: %s -> %s"), *Light->GetName(), Light->IsOn() ? TEXT("ON") : TEXT("OFF"));
	}
}

void UTDCDirectorComponent::BeginRewind()
{
	if (!HasPower(ETDCPower::Rewind) || bRewinding)
	{
		return;
	}
	bRewinding = true;
	if (UTDCStageSubsystem* Stage = GetWorld()->GetSubsystem<UTDCStageSubsystem>())
	{
		Stage->SetTimeMode(ETDCTimeMode::Rewind);
	}
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(/*Key*/ 7001, 600.f, FColor::Cyan, TEXT("<< REBOBINANDO"));
	}
}

void UTDCDirectorComponent::EndRewind()
{
	if (!bRewinding)
	{
		return;
	}
	bRewinding = false;
	if (UTDCStageSubsystem* Stage = GetWorld()->GetSubsystem<UTDCStageSubsystem>())
	{
		Stage->SetTimeMode(ETDCTimeMode::Play);
	}
	if (GEngine)
	{
		GEngine->RemoveOnScreenDebugMessage(7001);
	}
}
