#include "DelusionVehicleInputComponent.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"

void UDelusionVehicleInputComponent::BindDriverInput(UInputComponent* DriverInput)
{
    ClearDriverInput();
    APawn* VehiclePawn = Cast<APawn>(GetOwner());
    APlayerController* Driver = VehiclePawn ? Cast<APlayerController>(VehiclePawn->GetController()) : nullptr;
    ULocalPlayer* LocalPlayer = Driver ? Driver->GetLocalPlayer() : nullptr;
    BoundInput = Cast<UEnhancedInputComponent>(DriverInput);
    Subsystem = LocalPlayer ? LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;
    if (!BoundInput.IsValid() || !Subsystem.IsValid()) { return; }

    DriverMapping = NewObject<UInputMappingContext>(this);
    auto Action = [this](EInputActionValueType Type)
    {
        UInputAction* Result = NewObject<UInputAction>(this);
        Result->ValueType = Type;
        Result->AccumulationBehavior = EInputActionAccumulationBehavior::Cumulative;
        DriverActions.Add(Result);
        return Result;
    };
    UInputAction* DriveAction = Action(EInputActionValueType::Axis1D);
    UInputAction* SteerAction = Action(EInputActionValueType::Axis1D);
    UInputAction* LookAction = Action(EInputActionValueType::Axis2D);
    UInputAction* HandbrakeAction = Action(EInputActionValueType::Boolean);
    auto MapAxis = [this](UInputAction* InputAction, FKey Key, bool bNegate, bool bSwizzle = false)
    {
        FEnhancedActionKeyMapping& Mapping = DriverMapping->MapKey(InputAction, Key);
        if (bNegate) { Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(DriverMapping)); }
        if (bSwizzle)
        {
            UInputModifierSwizzleAxis* Modifier = NewObject<UInputModifierSwizzleAxis>(DriverMapping);
            Modifier->Order = EInputAxisSwizzle::YXZ;
            Mapping.Modifiers.Add(Modifier);
        }
    };
    MapAxis(DriveAction, EKeys::W, false);
    MapAxis(DriveAction, EKeys::S, true);
    MapAxis(SteerAction, EKeys::D, false);
    MapAxis(SteerAction, EKeys::A, true);
    MapAxis(LookAction, EKeys::MouseX, false);
    MapAxis(LookAction, EKeys::MouseY, true, true);
    DriverMapping->MapKey(HandbrakeAction, EKeys::SpaceBar);
    for (ETriggerEvent Event : {ETriggerEvent::Triggered, ETriggerEvent::Completed, ETriggerEvent::Canceled})
    {
        BoundInput->BindAction(DriveAction, Event, this, &UDelusionVehicleInputComponent::Drive);
        BoundInput->BindAction(SteerAction, Event, this, &UDelusionVehicleInputComponent::Steer);
        BoundInput->BindAction(HandbrakeAction, Event, this, &UDelusionVehicleInputComponent::Handbrake);
    }
    BoundInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &UDelusionVehicleInputComponent::Look);
    Subsystem->AddMappingContext(DriverMapping, 0);
    ResetControls(false);
}

void UDelusionVehicleInputComponent::Drive(const FInputActionValue& Value)
{
    if (UChaosWheeledVehicleMovementComponent* Movement = GetOwner()->FindComponentByClass<UChaosWheeledVehicleMovementComponent>())
    {
        const float Axis = FMath::Clamp(Value.Get<float>(), -1.0f, 1.0f);
        Movement->SetThrottleInput(FMath::Max(Axis, 0.0f));
        Movement->SetBrakeInput(FMath::Max(-Axis, 0.0f));
    }
}

void UDelusionVehicleInputComponent::Steer(const FInputActionValue& Value)
{
    if (UChaosWheeledVehicleMovementComponent* Movement = GetOwner()->FindComponentByClass<UChaosWheeledVehicleMovementComponent>())
    {
        Movement->SetSteeringInput(Value.Get<float>());
    }
}

void UDelusionVehicleInputComponent::Look(const FInputActionValue& Value)
{
    if (APawn* VehiclePawn = Cast<APawn>(GetOwner()))
    {
        const FVector2D Axes = Value.Get<FVector2D>();
        VehiclePawn->AddControllerYawInput(Axes.X * MouseSensitivity);
        VehiclePawn->AddControllerPitchInput(Axes.Y * MouseSensitivity);
    }
}

void UDelusionVehicleInputComponent::Handbrake(const FInputActionValue& Value)
{
    if (UChaosWheeledVehicleMovementComponent* Movement = GetOwner()->FindComponentByClass<UChaosWheeledVehicleMovementComponent>())
    {
        Movement->SetHandbrakeInput(Value.Get<bool>());
    }
}

void UDelusionVehicleInputComponent::ResetControls(bool bPark)
{
    if (UChaosWheeledVehicleMovementComponent* Movement = GetOwner()->FindComponentByClass<UChaosWheeledVehicleMovementComponent>())
    {
        Movement->SetThrottleInput(0.0f);
        Movement->SetBrakeInput(0.0f);
        Movement->SetSteeringInput(0.0f);
        Movement->SetHandbrakeInput(bPark);
    }
}

void UDelusionVehicleInputComponent::ClearDriverInput()
{
    if (BoundInput.IsValid()) { BoundInput->ClearBindingsForObject(this); }
    if (Subsystem.IsValid() && DriverMapping) { Subsystem->RemoveMappingContext(DriverMapping); }
    BoundInput.Reset();
    Subsystem.Reset();
    DriverMapping = nullptr;
    DriverActions.Reset();
    ResetControls(true);
}

void UDelusionVehicleInputComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ClearDriverInput();
    Super::EndPlay(EndPlayReason);
}