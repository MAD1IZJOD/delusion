#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DelusionVehicleSeatComponent.generated.h"

class ADelusionCharacter;
class APlayerController;
class APawn;

/** Preserves the on-foot pawn while a controller possesses this seat's vehicle. */
UCLASS(ClassGroup=(Delusion), meta=(BlueprintSpawnableComponent))
class DELUSION_API UDelusionVehicleSeatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    bool CanEnter(APawn* Interactor) const;
    bool TryEnter(APawn* Interactor);
    bool TryExit(APlayerController* Driver);
    bool FindSafeExit(FVector& OutLocation) const;
    bool IsOccupied() const;
    ADelusionCharacter* GetOccupant() const { return Occupant; }
    float GetSpeedKmh() const;
    float GetMaximumExitSpeed() const { return MaximumExitSpeedKmh; }

protected:
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(EditAnywhere, Category="Seat", meta=(ClampMin="0"))
    float MaximumExitSpeedKmh = 5.0f;

    UPROPERTY(EditAnywhere, Category="Seat", meta=(ClampMin="0"))
    float MaximumEntrySpeedKmh = 3.0f;

    UPROPERTY(EditAnywhere, Category="Seat", meta=(ClampMin="0"))
    float EntryDistance = 260.0f;

private:
    UPROPERTY(Transient)
    TObjectPtr<ADelusionCharacter> Occupant;

    TWeakObjectPtr<APlayerController> DriverController;
    FTransform OnFootTransform;
};