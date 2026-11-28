#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "DelusionInteractable.h"
#include "DelusionVehicle.generated.h"

class UCameraComponent;
class UChaosWheeledVehicleMovementComponent;
class UDelusionVehicleInputComponent;
class UDelusionVehicleSeatComponent;
class UMaterialInterface;
class USpringArmComponent;
class UStaticMeshComponent;
class USynthComponentToneGenerator;

/** Standard Chaos four-wheel pawn, with independently owned input and seat behavior. */
UCLASS(Blueprintable)
class DELUSION_API ADelusionVehicle : public AWheeledVehiclePawn, public IDelusionInteractable
{
    GENERATED_BODY()

public:
    ADelusionVehicle();
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual void UnPossessed() override;
    virtual bool CanInteract_Implementation(APawn* Interactor) const override;
    virtual FText GetInteractionPrompt_Implementation() const override;
    virtual bool Interact_Implementation(APawn* Interactor) override;

    UDelusionVehicleSeatComponent* GetSeat() const { return Seat; }
    UDelusionVehicleInputComponent* GetDriverInput() const { return DriverInput; }
    UChaosWheeledVehicleMovementComponent* GetDrivingMovement() const;
    float GetSpeedKmh() const;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle")
    TObjectPtr<UDelusionVehicleSeatComponent> Seat;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle")
    TObjectPtr<UDelusionVehicleInputComponent> DriverInput;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    TObjectPtr<UCameraComponent> ChaseCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Audio")
    TObjectPtr<USynthComponentToneGenerator> EngineSynth;

    UPROPERTY(EditDefaultsOnly, Category="Appearance")
    TObjectPtr<UMaterialInterface> SurfaceMaterial;

private:
    UPROPERTY()
    TArray<TObjectPtr<UStaticMeshComponent>> Bodywork;

    UPROPERTY()
    TArray<TObjectPtr<UStaticMeshComponent>> WheelVisuals;
};