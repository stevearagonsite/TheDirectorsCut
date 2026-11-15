#include "Stage/TDCTimelineComponent.h"
#include "Stage/TDCStageSubsystem.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

UTDCTimelineComponent::UTDCTimelineComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UTDCTimelineComponent::BeginPlay()
{
	Super::BeginPlay();
	History.SetCapacity(FMath::CeilToInt(RecordSeconds * SamplesPerSecond));

	if (UTDCStageSubsystem* Stage = GetWorld()->GetSubsystem<UTDCStageSubsystem>())
	{
		TimeModeHandle = Stage->OnTimeModeChanged.AddUObject(this, &UTDCTimelineComponent::OnTimeModeChanged);
		Mode = Stage->GetTimeMode();
	}

	// Frame inicial de la toma: rebobinar hasta acá restaura el estado de partida.
	History.Push(Capture());
}

void UTDCTimelineComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UTDCStageSubsystem* Stage = GetWorld()->GetSubsystem<UTDCStageSubsystem>())
	{
		Stage->OnTimeModeChanged.Remove(TimeModeHandle);
	}
	Super::EndPlay(EndPlayReason);
}

UPrimitiveComponent* UTDCTimelineComponent::GetBody() const
{
	return GetOwner() ? Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent()) : nullptr;
}

FTDCTimeFrame UTDCTimelineComponent::Capture() const
{
	FTDCTimeFrame Frame;
	Frame.Transform = GetOwner()->GetActorTransform();
	if (const UPrimitiveComponent* Body = GetBody())
	{
		Frame.bSimulatingPhysics = Body->IsSimulatingPhysics();
		if (Frame.bSimulatingPhysics)
		{
			Frame.LinearVelocity = Body->GetPhysicsLinearVelocity();
			Frame.AngularVelocityDeg = Body->GetPhysicsAngularVelocityInDegrees();
		}
	}
	return Frame;
}

void UTDCTimelineComponent::Apply(const FTDCTimeFrame& Frame)
{
	GetOwner()->SetActorTransform(Frame.Transform, false, nullptr, ETeleportType::TeleportPhysics);
}

void UTDCTimelineComponent::OnTimeModeChanged(ETDCTimeMode NewMode)
{
	if (!IsActive())
	{
		return;
	}
	if (NewMode == Mode)
	{
		return;
	}
	Mode = NewMode;
	Accumulator = 0.f;
	UPrimitiveComponent* Body = GetBody();

	if (Mode == ETDCTimeMode::Rewind)
	{
		bHasCurrentFrame = false;
		if (Body && Body->IsSimulatingPhysics())
		{
			Body->SetSimulatePhysics(false);
		}
		return;
	}

	// Vuelve a Play: la toma sigue desde el último frame rebobinado.
	if (!bHasCurrentFrame)
	{
		return;
	}
	if (Body && CurrentFrame.bSimulatingPhysics)
	{
		Body->SetSimulatePhysics(true);
		Body->SetPhysicsLinearVelocity(CurrentFrame.LinearVelocity);
		Body->SetPhysicsAngularVelocityInDegrees(CurrentFrame.AngularVelocityDeg);
	}
	if (History.Num() == 0)
	{
		History.Push(CurrentFrame); // nunca perder el frame de partida
	}
}

void UTDCTimelineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const float Step = 1.f / SamplesPerSecond;

	if (Mode == ETDCTimeMode::Play)
	{
		Accumulator += DeltaTime;
		if (Accumulator < Step)
		{
			return;
		}
		Accumulator = FMath::Fmod(Accumulator, Step);

		const FTDCTimeFrame Frame = Capture();
		const FTDCTimeFrame* Last = History.Last();
		const bool bMoved = !Last
			|| !Last->Transform.Equals(Frame.Transform, 0.1f)
			|| Last->bSimulatingPhysics != Frame.bSimulatingPhysics;
		if (bMoved)
		{
			History.Push(Frame);
		}
		return;
	}

	// Rebobinando: consumir frames a RewindSpeed. El último frame (partida) no se saca del buffer.
	Accumulator += DeltaTime * RewindSpeed;
	while (Accumulator >= Step && History.Num() > 0)
	{
		Accumulator -= Step;
		FTDCTimeFrame Frame;
		if (History.Num() == 1)
		{
			Frame = *History.Last();
			Accumulator = 0.f;
		}
		else
		{
			History.Pop(Frame);
		}
		CurrentFrame = Frame;
		bHasCurrentFrame = true;
		Apply(Frame);
		if (History.Num() == 1)
		{
			break;
		}
	}
}
