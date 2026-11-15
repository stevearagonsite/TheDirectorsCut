#include "Stage/TDCStageSubsystem.h"
#include "Stage/TDCStageLight.h"

void UTDCStageSubsystem::RegisterLight(ATDCStageLight* Light)
{
	if (Light)
	{
		Lights.AddUnique(Light);
		NotifyLightingChanged();
	}
}

void UTDCStageSubsystem::UnregisterLight(ATDCStageLight* Light)
{
	Lights.Remove(Light);
	NotifyLightingChanged();
}

void UTDCStageSubsystem::NotifyLightingChanged()
{
	OnLightingChanged.Broadcast();
}

void UTDCStageSubsystem::GatherLightSamples(TArray<FTDCLightSample>& OutSamples) const
{
	OutSamples.Reset(Lights.Num());
	for (const TWeakObjectPtr<ATDCStageLight>& Light : Lights)
	{
		if (Light.IsValid())
		{
			OutSamples.Add(Light->GetSample());
		}
	}
}

void UTDCStageSubsystem::SetTimeMode(ETDCTimeMode NewMode)
{
	if (TimeMode != NewMode)
	{
		TimeMode = NewMode;
		OnTimeModeChanged.Broadcast(TimeMode);
	}
}
