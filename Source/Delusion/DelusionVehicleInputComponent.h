#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DelusionVehicleInputComponent.generated.h"

class UEnhancedInputComponent;
class UEnhancedInputLocalPlayerSubsystem;
class UInputComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

/** Driver-only mapping and input routing; the Chaos component owns simulation. */
UCLASS(ClassGroup=(Delusion), meta=(BlueprintSpawnableComponent))
class DELUSION_API UDelusionVehicleInputComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    void BindDriverInput(UInputComponent* DriverInput);
    void ClearDriverInput();
    void ResetControls(bool bPark);

protected:
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(EditAnywhere, Category="Camera", meta=(ClampMin="0.01"))
    float MouseSensitivity = 0.13f;

private:
    void Drive(const FInputActionValue& Value);
    void Steer(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void Handbrake(const FInputActionValue& Value);

    UPROPERTY(Transient)
    TObjectPtr<UInputMappingContext> DriverMapping;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UInputAction>> DriverActions;

    TWeakObjectPtr<UEnhancedInputLocalPlayerSubsystem> Subsystem;
    TWeakObjectPtr<UEnhancedInputComponent> BoundInput;
};