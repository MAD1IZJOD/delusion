#pragma once

#include "CoreMinimal.h"
#include "DelusionInteractable.h"
#include "GameFramework/Actor.h"
#include "DelusionTestTerminal.generated.h"

class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS(Blueprintable)
class DELUSION_API ADelusionTestTerminal : public AActor, public IDelusionInteractable
{
    GENERATED_BODY()

public:
    ADelusionTestTerminal();
    virtual bool CanInteract_Implementation(APawn* Interactor) const override;
    virtual FText GetInteractionPrompt_Implementation() const override;
    virtual bool Interact_Implementation(APawn* Interactor) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction")
    bool bActivated = false;

private:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> TerminalBody;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UTextRenderComponent> StatusLabel;
};
