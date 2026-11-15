#include "Director/TDCDirectorComponent.h"
#include "Stage/TDCStageLight.h"
#include "Stage/TDCStageSubsystem.h"
#include "Combat/TDCExtra.h"
#include "Engine/OverlapResult.h"
#include "CollisionShape.h"
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

bool UTDCDirectorComponent::Clap()
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		return false;
	}
	const double Now = World->GetTimeSeconds();
	if (Now - LastClapTime < ClapCooldown)
	{
		return false;
	}
	LastClapTime = Now;

	// En 2.5D el director mira en +X o -X: el golpe sale hacia ese lado, sobre el plano de juego.
	const float Facing = Owner->GetActorForwardVector().X >= 0.f ? 1.f : -1.f;
	const FVector Center = Owner->GetActorLocation() + FVector(Facing * ClapReach, 0.f, 0.f);

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(TDCClap), false, Owner);
	World->OverlapMultiByObjectType(Overlaps, Center, FQuat::Identity,
		FCollisionObjectQueryParams(FCollisionObjectQueryParams::AllDynamicObjects),
		FCollisionShape::MakeSphere(ClapRadius), Params);

	FString Feedback = TEXT("¡Claquetazo!");
	TSet<AActor*> Hit;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		ATDCExtra* Extra = Cast<ATDCExtra>(Overlap.GetActor());
		if (!Extra || Hit.Contains(Extra))
		{
			continue;
		}
		Hit.Add(Extra);
		switch (Extra->ReceiveClap(Owner->GetActorLocation()))
		{
		case ETDCClapResult::Defeated: Feedback = TEXT("¡Corten! Extra fuera de la toma"); break;
		case ETDCClapResult::Stunned:  Feedback = TEXT("¡Claquetazo! (aturdido)"); break;
		default:                       Feedback = TEXT("Rebota: es un extra de guion. Dirigí la escena."); break;
		}
	}
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(7002, 1.2f, FColor::Yellow, Feedback);
	}
	return true;
}
