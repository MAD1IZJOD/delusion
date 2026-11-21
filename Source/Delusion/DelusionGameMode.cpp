#include "DelusionGameMode.h"

#include "DelusionCharacter.h"
#include "DelusionPlayerController.h"

ADelusionGameMode::ADelusionGameMode()
{
    DefaultPawnClass = ADelusionCharacter::StaticClass();
    PlayerControllerClass = ADelusionPlayerController::StaticClass();
    bStartPlayersAsSpectators = false;
    bPauseable = true;
}
