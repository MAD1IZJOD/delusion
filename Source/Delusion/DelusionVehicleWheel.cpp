#include "DelusionVehicleWheel.h"

namespace
{
void ConfigureWheel(UChaosVehicleWheel& Wheel)
{
    Wheel.WheelWidth = 30.0f;
    Wheel.WheelMass = 20.0f;
    Wheel.FrictionForceMultiplier = 2.8f;
    Wheel.MaxBrakeTorque = 3500.0f;
    Wheel.MaxHandBrakeTorque = 6000.0f;
    // Chaos converts these wheel spring settings from metres to centimetres.
    Wheel.SpringRate = 300.0f;
    Wheel.SpringPreload = 50.0f;
    Wheel.SuspensionMaxRaise = 10.0f;
    Wheel.SuspensionMaxDrop = 15.0f;
    Wheel.SuspensionDampingRatio = 0.65f;
    Wheel.WheelLoadRatio = 0.5f;
    Wheel.RollbarScaling = 0.25f;
    Wheel.SweepShape = ESweepShape::Raycast;
    Wheel.SweepType = ESweepType::SimpleSweep;
    Wheel.bABSEnabled = true;
    Wheel.bTractionControlEnabled = true;
}
}

UDelusionFrontWheel::UDelusionFrontWheel()
{
    ConfigureWheel(*this);
    AxleType = EAxleType::Front;
    WheelRadius = 39.0f;
    MaxSteerAngle = 32.0f;
    bAffectedBySteering = true;
    bAffectedByHandbrake = false;
}

UDelusionRearWheel::UDelusionRearWheel()
{
    ConfigureWheel(*this);
    AxleType = EAxleType::Rear;
    WheelRadius = 40.0f;
    MaxSteerAngle = 0.0f;
    bAffectedBySteering = false;
    bAffectedByHandbrake = true;
}