#pragma once

#include "CoreMinimal.h"
#include "ChaosVehicleWheel.h"
#include "DelusionVehicleWheel.generated.h"

/** Axle defaults shared by the first car and future four-wheel configurations. */
UCLASS()
class DELUSION_API UDelusionFrontWheel : public UChaosVehicleWheel
{
    GENERATED_BODY()
public:
    UDelusionFrontWheel();
};

UCLASS()
class DELUSION_API UDelusionRearWheel : public UChaosVehicleWheel
{
    GENERATED_BODY()
public:
    UDelusionRearWheel();
};