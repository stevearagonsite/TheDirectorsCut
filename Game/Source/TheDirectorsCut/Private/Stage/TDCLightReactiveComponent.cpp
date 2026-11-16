#include "Stage/TDCLightReactiveComponent.h"
#include "Stage/TDCStageSubsystem.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

UTDCLightReactiveComponent::UTDCLightReactiveComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTDCLightReactiveComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UTDCStageSubsystem* Stage = GetWorld()->GetSubsystem<UTDCStageSubsystem>())
	{
		LightingHandle = Stage->OnLightingChanged.AddUObject(this, &UTDCLightReactiveComponent::Evaluate);
	}
	Evaluate();
}

void UTDCLightReactiveComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UTDCStageSubsystem* Stage = GetWorld()->GetSubsystem<UTDCStageSubsystem>())
	{
		Stage->OnLightingChanged.Remove(LightingHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void UTDCLightReactiveComponent::Evaluate()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}
	if (Presence == ETDCLightPresence::Always)
	{
		ApplyPresence(true);
		return;
	}

	TArray<FTDCLightSample> Samples;
	if (const UTDCStageSubsystem* Stage = GetWorld()->GetSubsystem<UTDCStageSubsystem>())
	{
		Stage->GatherLightSamples(Samples);
	}
	// bNonColliding=true: las cajas cuentan aunque la pieza esté sin colisión (ausente).
	// Bounds del root (cubo en SetPiece, cápsula en el extra): los componentes visuales (puppet animado con escala
	// compensada, cornisas del kit) inflaban la caja y una pieza "solo a oscuras" nunca podía estar presente (T-0049).
	const USceneComponent* Root = OwnerActor->GetRootComponent();
	const FBox Bounds = Root ? Root->Bounds.GetBox() : OwnerActor->GetComponentsBoundingBox(/*bNonColliding*/ true);
	ApplyPresence(TDC::IsPresent(Presence, TDC::IsLit(Bounds, Samples)));
}

void UTDCLightReactiveComponent::ApplyPresence(bool bNewPresent)
{
	if (bApplied && bPresent == bNewPresent)
	{
		return;
	}
	bApplied = true;
	bPresent = bNewPresent;

	AActor* OwnerActor = GetOwner();
	OwnerActor->SetActorHiddenInGame(bHideWhenAbsent && !bPresent);
	OwnerActor->SetActorEnableCollision(bPresent);
	OnPresenceChanged.Broadcast(bPresent);
}
