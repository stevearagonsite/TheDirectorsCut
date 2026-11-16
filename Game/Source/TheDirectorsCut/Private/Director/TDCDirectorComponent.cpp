#include "Director/TDCDirectorComponent.h"
#include "Stage/TDCStageLight.h"
#include "Stage/TDCStageSubsystem.h"
#include "Combat/TDCExtra.h"
#include "Core/TDCFlipbookFx.h"
#include "Core/TDCVisuals.h"
#include "UI/TDCHUD.h"
#include "Components/AudioComponent.h"
#include "Engine/OverlapResult.h"
#include "CollisionShape.h"
#include "TheDirectorsCut.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Containers/Ticker.h"

namespace
{
	/** Hit-stop (game feel): congela casi todo unos ms al conectar el golpe. Se restaura en tiempo real (core ticker). */
	void HitStop(UWorld* World, float Seconds)
	{
		if (!World || Seconds <= 0.f || World->GetNetMode() != NM_Standalone)
		{
			return;
		}
		UGameplayStatics::SetGlobalTimeDilation(World, 0.05f);
		TWeakObjectPtr<UWorld> WeakWorld(World);
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([WeakWorld](float)
		{
			if (UWorld* W = WeakWorld.Get())
			{
				UGameplayStatics::SetGlobalTimeDilation(W, 1.f);
			}
			return false; // una sola vez
		}), Seconds);
	}
}

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
		TDC::PlaySfx(this, Light->IsOn() ? TEXT("sfx_light_on") : TEXT("sfx_light_off"), Light->GetActorLocation());
		ATDCFlipbookFx::Spawn(GetWorld(), TDC::Fx::LightToggle, Light->GetActorLocation() + FVector(0.f, 60.f, 0.f), 220.f);
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
	if (AActor* Owner = GetOwner())
	{
		RewindAudio = TDC::PlaySfxAttached(TEXT("sfx_rewind_loop"), Owner->GetRootComponent(), 0.8f);
		RewindFx = ATDCFlipbookFx::Spawn(GetWorld(), TDC::Fx::Rewind, Owner->GetActorLocation() + FVector(0.f, 60.f, 130.f),
			150.f, false, Owner->GetRootComponent());
	}
	if (UTDCStageSubsystem* Stage = GetWorld()->GetSubsystem<UTDCStageSubsystem>())
	{
		Stage->SetTimeMode(ETDCTimeMode::Rewind);
	}
	TDC::HudMessage(this, 7001, TEXT("<<  REBOBINANDO"), FLinearColor(0.35f, 0.85f, 1.f), -1.f, ETDCHudSlot::Banner);
}

void UTDCDirectorComponent::EndRewind()
{
	if (!bRewinding)
	{
		return;
	}
	bRewinding = false;
	if (RewindFx.IsValid())
	{
		RewindFx->Destroy();
	}
	if (RewindAudio.IsValid())
	{
		RewindAudio->Stop();
	}
	RewindFx.Reset();
	if (UTDCStageSubsystem* Stage = GetWorld()->GetSubsystem<UTDCStageSubsystem>())
	{
		Stage->SetTimeMode(ETDCTimeMode::Play);
	}
	TDC::HudClear(this, 7001);
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

	// Impacto del golpe (VFX hacia la cámara, espejado si mira a -X).
	TDC::PlaySfx(this, TEXT("sfx_claquetazo"), Center);
	ATDCFlipbookFx::Spawn(World, TDC::Fx::ClapImpact, Center + FVector(0.f, 60.f, 10.f), 170.f, Facing < 0.f);

	FString Feedback = TEXT("¡Claquetazo!");
	float StopSeconds = 0.f; // el resultado más fuerte del golpe decide el hit-stop
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
		case ETDCClapResult::Defeated:
			Feedback = TEXT("¡Corten! Extra fuera de la toma");
			StopSeconds = FMath::Max(StopSeconds, 0.09f);
			break;
		case ETDCClapResult::Stunned:
			Feedback = TEXT("¡Claquetazo! (aturdido)");
			StopSeconds = FMath::Max(StopSeconds, 0.06f);
			break;
		default:
			StopSeconds = FMath::Max(StopSeconds, 0.04f);
			Feedback = TEXT("Rebota: es un extra de guion. Dirigí la escena.");
			TDC::PlaySfx(this, TEXT("sfx_bounce"), Extra->GetActorLocation());
			ATDCFlipbookFx::Spawn(World, TDC::Fx::ScriptedBounce, Extra->GetActorLocation() + FVector(-Facing * 30.f, 70.f, 20.f), 200.f, Facing < 0.f);
			break;
		}
	}
	HitStop(World, StopSeconds);
	TDC::HudMessage(this, 7002, Feedback, FLinearColor(1.f, 0.9f, 0.6f), 1.2f, ETDCHudSlot::Toast);
	return true;
}
