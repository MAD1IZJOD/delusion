#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DelusionPlayerController.generated.h"

class UDelusionHUDWidget;
class UDelusionInteractionComponent;
class UInputAction;
class UInputMappingContext;

UCLASS()
class DELUSION_API ADelusionPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ADelusionPlayerController();

    UFUNCTION(BlueprintCallable, Category="Pause")
    void TogglePause();

    UFUNCTION(BlueprintCallable, Category="Pause")
    void ResumeGame();

    UFUNCTION(BlueprintCallable, Category="Pause")
    void QuitGame();

    UFUNCTION(BlueprintPure, Category="Interaction")
    UDelusionInteractionComponent* GetInteraction() const { return Interaction; }

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    void Interact();

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UDelusionInteractionComponent> Interaction;

    UPROPERTY(Transient)
    TObjectPtr<UDelusionHUDWidget> HUDWidget;

    UPROPERTY(Transient)
    TObjectPtr<UInputMappingContext> ActionMapping;

    UPROPERTY(Transient)
    TObjectPtr<UInputAction> InteractAction;

    UPROPERTY(Transient)
    TObjectPtr<UInputAction> PauseAction;
};
