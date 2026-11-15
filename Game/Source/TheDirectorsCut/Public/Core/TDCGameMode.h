#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TDCGameMode.generated.h"

UCLASS()
class THEDIRECTORSCUT_API ATDCGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATDCGameMode();

	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	/** Blueprint del director con el mesh y el AnimBP finales. Si no existe, se usa el greybox en C++. */
	UPROPERTY(EditDefaultsOnly, Category = "TDC")
	FSoftClassPath DirectorClassPath = FSoftClassPath(TEXT("/Game/Characters/Director/BP_Director.BP_Director_C"));
};
