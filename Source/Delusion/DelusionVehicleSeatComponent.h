#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DelusionVehicleSeatComponent.generated.h"

class ADelusionCharacter;
class APlayerController;
class APawn;

UENUM()
enum class EDelusionVehicleTransitionState : uint8
{
    Idle,
    Entering,
    Exiting
};

/** Preserves the on-foot pawn while a controller possesses this seat's vehicle. */
UCLASS(ClassGroup=(Delusion), meta=(BlueprintSpawnableComponent))
class DELUSION_API UDelusionVehicleSeatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDelusionVehicleSeatComponent();

    bool CanEnter(APawn* Interactor) const;
    bool TryEnter(APawn* Interactor);
    bool TryExit(APlayerController* Driver);
    bool FindSafeExit(FVector& OutLocation) const;
    bool IsOccupied() const;
    bool IsTransitioning() const { return TransitionState != EDelusionVehicleTransitionState::Idle; }
    ADelusionCharacter* GetOccupant() const { return Occupant; }
    float GetSpeedKmh() const;
    float GetMaximumExitSpeed() const { return MaximumExitSpeedKmh; }

protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(EditAnywhere, Category="Seat", meta=(ClampMin="0"))
    float MaximumExitSpeedKmh = 5.0f;

    UPROPERTY(EditAnywhere, Category="Seat", meta=(ClampMin="0"))
    float MaximumEntrySpeedKmh = 3.0f;

    UPROPERTY(EditAnywhere, Category="Seat", meta=(ClampMin="0"))
    float EntryDistance = 260.0f;

    UPROPERTY(EditAnywhere, Category="Seat", meta=(ClampMin="0.05", ClampMax="1.0"))
    float TransitionDuration = 0.25f;

private:
    void CompleteEntry();
    void CompleteExit();
    void CancelTransition();

    UPROPERTY(Transient)
    EDelusionVehicleTransitionState TransitionState = EDelusionVehicleTransitionState::Idle;

    UPROPERTY(Transient)
    TObjectPtr<ADelusionCharacter> Occupant;

    TWeakObjectPtr<APlayerController> DriverController;
    FTransform OnFootTransform;
    FVector ExitLocation = FVector::ZeroVector;
    float TransitionTimer = 0.0f;
};