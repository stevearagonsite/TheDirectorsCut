#include "Core/TDCGameMode.h"
#include "Character/TDCCharacter.h"
#include "Core/TDCPlayerController.h"

ATDCGameMode::ATDCGameMode()
{
	DefaultPawnClass = ATDCCharacter::StaticClass();
	PlayerControllerClass = ATDCPlayerController::StaticClass(); // menú inicial y pausa (T-0035)
}

UClass* ATDCGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	// Carga silenciosa: mientras no exista BP_Director, el juego sigue con el greybox sin errores en el log.
	// Para que entre al build empaquetado, /Game/Characters está en DirectoriesToAlwaysCook (DefaultGame.ini).
	if (DirectorClassPath.IsValid())
	{
		if (UClass* DirectorClass = StaticLoadClass(ATDCCharacter::StaticClass(), nullptr, *DirectorClassPath.ToString(),
			nullptr, LOAD_NoWarn | LOAD_Quiet))
		{
			return DirectorClass;
		}
	}
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}
