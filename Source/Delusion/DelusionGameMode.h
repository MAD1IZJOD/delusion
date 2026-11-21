#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DelusionGameMode.generated.h"

/** Connects the single-player pawn and controller to the prototype level. */
UCLASS()
class DELUSION_API ADelusionGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ADelusionGameMode();
};
