#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DelusionInteractable.generated.h"

class APawn;

UINTERFACE(BlueprintType)
class DELUSION_API UDelusionInteractable : public UInterface
{
    GENERATED_BODY()
};

/** Shared contract for objects that a nearby player can activate. */
class DELUSION_API IDelusionInteractable
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
    bool CanInteract(APawn* Interactor) const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
    FText GetInteractionPrompt() const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
    bool Interact(APawn* Interactor);
};
