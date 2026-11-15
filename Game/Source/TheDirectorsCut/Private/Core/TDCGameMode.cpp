#include "Core/TDCGameMode.h"
#include "Character/TDCCharacter.h"

ATDCGameMode::ATDCGameMode()
{
	DefaultPawnClass = ATDCCharacter::StaticClass();
}
