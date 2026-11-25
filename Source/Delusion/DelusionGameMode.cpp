#include "DelusionGameMode.h"

#include "DelusionCharacter.h"
#include "DelusionPlayerController.h"
#include "DelusionVehicle.h"
#include "Engine/World.h"
#include "EngineUtils.h"

ADelusionGameMode::ADelusionGameMode()
{
    DefaultPawnClass = ADelusionCharacter::StaticClass();
    PlayerControllerClass = ADelusionPlayerController::StaticClass();
    bStartPlayersAsSpectators = false;
    bPauseable = true;
}

void ADelusionGameMode::StartPlay()
{
    Super::StartPlay();
    for (TActorIterator<ADelusionVehicle> It(GetWorld()); It; ++It)
    {
        return; // Preserve a designer-placed car instead of adding a duplicate.
    }
    FActorSpawnParameters Spawn;
    Spawn.Name = TEXT("PrototypeCar");
    Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    if (!GetWorld()->SpawnActor<ADelusionVehicle>(ADelusionVehicle::StaticClass(),
        FVector(-1400, 250, 80), FRotator::ZeroRotator, Spawn))
    {
        UE_LOG(LogTemp, Error, TEXT("DELUSION failed to spawn the prototype car"));
    }
}
