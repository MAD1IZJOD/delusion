#include "DelusionVehicle.h"

#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DelusionVehicleInputComponent.h"
#include "DelusionVehicleSeatComponent.h"
#include "DelusionVehicleWheel.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "SynthComponents/SynthComponentToneGenerator.h"
#include "UObject/ConstructorHelpers.h"

ADelusionVehicle::ADelusionVehicle()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PostPhysics;
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
    SetReplicates(false);
    SetReplicateMovement(false);

    static ConstructorHelpers::FObjectFinder<USkeletalMesh> Rig(TEXT("/Game/Vehicles/SportsCar/SKM_SportsCar.SKM_SportsCar"));
    GetMesh()->SetSkeletalMeshAsset(Rig.Object);
    static ConstructorHelpers::FClassFinder<UAnimInstance> VehicleAnimBP(TEXT("/Game/Vehicles/SportsCar/ABP_SportsCar.ABP_SportsCar_C"));
    if (VehicleAnimBP.Class) { GetMesh()->SetAnimInstanceClass(VehicleAnimBP.Class); }
    GetMesh()->SetCollisionProfileName(TEXT("Vehicle"));
    GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetEnableGravity(true);
    GetMesh()->SetRenderInMainPass(false);
    GetMesh()->SetCastShadow(false);
    GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

    UChaosWheeledVehicleMovementComponent* Movement = GetDrivingMovement();
    Movement->SetIsReplicated(false);
    // Process the explicitly reset parking brake even while there is no driver.
    Movement->SetRequiresControllerForInputs(false);
    Movement->Mass = 1250.0f;
    Movement->bEnableCenterOfMassOverride = true;
    Movement->CenterOfMassOverride = FVector(0, 0, -15);
    Movement->ChassisWidth = 180.0f;
    Movement->ChassisHeight = 120.0f;
    Movement->DragCoefficient = 0.35f;
    Movement->DownforceCoefficient = 0.3f;
    Movement->bReverseAsBrake = true;
    Movement->bThrottleAsBrake = true;

    Movement->WheelSetups.SetNum(4);
    const FName WheelBones[] = {TEXT("Phys_Wheel_FL"), TEXT("Phys_Wheel_FR"), TEXT("Phys_Wheel_BL"), TEXT("Phys_Wheel_BR")};
    for (int32 Index = 0; Index < 4; ++Index)
    {
        Movement->WheelSetups[Index].BoneName = WheelBones[Index];
        Movement->WheelSetups[Index].WheelClass = Index < 2
            ? UDelusionFrontWheel::StaticClass() : UDelusionRearWheel::StaticClass();
    }
    Movement->DifferentialSetup.DifferentialType = EVehicleDifferential::AllWheelDrive;
    Movement->DifferentialSetup.FrontRearSplit = 0.5f;
    Movement->EngineSetup.MaxTorque = 320.0f;
    Movement->EngineSetup.MaxRPM = 6000.0f;
    Movement->EngineSetup.EngineIdleRPM = 850.0f;
    Movement->EngineSetup.EngineBrakeEffect = 0.15f;
    FRichCurve* Torque = Movement->EngineSetup.TorqueCurve.GetRichCurve();
    Torque->Reset();
    Torque->AddKey(0, 0.5f);
    Torque->AddKey(1000, 0.7f);
    Torque->AddKey(2500, 1.0f);
    Torque->AddKey(4500, 0.9f);
    Torque->AddKey(6000, 0.5f);
    Movement->TransmissionSetup.bUseAutomaticGears = true;
    Movement->TransmissionSetup.bUseAutoReverse = true;
    Movement->TransmissionSetup.FinalRatio = 3.1f;
    Movement->TransmissionSetup.ForwardGearRatios = {3.2f, 2.1f, 1.4f, 1.0f, 0.8f};
    Movement->TransmissionSetup.ReverseGearRatios = {3.0f};
    Movement->TransmissionSetup.ChangeUpRPM = 5000.0f;
    Movement->TransmissionSetup.ChangeDownRPM = 1800.0f;
    Movement->TransmissionSetup.GearChangeTime = 0.2f;
    Movement->TransmissionSetup.TransmissionEfficiency = 0.9f;
    Movement->SteeringSetup.SteeringType = ESteeringType::Ackermann;
    FRichCurve* Steering = Movement->SteeringSetup.SteeringCurve.GetRichCurve();
    Steering->Reset();
    Steering->AddKey(0, 1.0f);
    Steering->AddKey(20, 0.65f);
    Steering->AddKey(50, 0.35f);
    Steering->AddKey(80, 0.25f);

    Seat = CreateDefaultSubobject<UDelusionVehicleSeatComponent>(TEXT("DriverSeat"));
    DriverInput = CreateDefaultSubobject<UDelusionVehicleInputComponent>(TEXT("DriverInput"));
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("VehicleCameraBoom"));
    CameraBoom->SetupAttachment(GetMesh());
    CameraBoom->SetRelativeLocation(FVector(0, 0, 80));
    CameraBoom->TargetArmLength = 600.0f;
    CameraBoom->SocketOffset = FVector(0, 0, 70);
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bInheritRoll = false;
    CameraBoom->bDoCollisionTest = true;
    CameraBoom->ProbeSize = 16.0f;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 10.0f;
    CameraBoom->CameraLagMaxDistance = 100.0f;
    CameraBoom->bEnableCameraRotationLag = true;
    CameraBoom->CameraRotationLagSpeed = 12.0f;
    ChaseCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCamera"));
    ChaseCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    ChaseCamera->bUsePawnControlRotation = false;
    ChaseCamera->FieldOfView = 80.0f;

    EngineSynth = CreateDefaultSubobject<USynthComponentToneGenerator>(TEXT("EngineSynth"));
    EngineSynth->SetupAttachment(GetMesh());
    EngineSynth->SetFrequency(80.0f);
    EngineSynth->SetVolume(0.15f);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Game/Materials/M_PrototypeSurface.M_PrototypeSurface"));
    SurfaceMaterial = Material.Object;
    auto AddBody = [this](const TCHAR* PartName, FVector Position, FVector Size, FName ColorTag)
    {
        UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(PartName);
        Part->SetupAttachment(GetMesh());
        Part->SetStaticMesh(Cube.Object);
        Part->SetRelativeLocation(Position);
        Part->SetRelativeScale3D(Size / 100.0f);
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Part->ComponentTags.Add(ColorTag);
        Bodywork.Add(Part);
    };
    AddBody(TEXT("Body"), FVector(0, 0, 55), FVector(420, 180, 55), TEXT("Paint"));
    AddBody(TEXT("Cabin"), FVector(-25, 0, 110), FVector(165, 155, 60), TEXT("Glass"));
    AddBody(TEXT("Hood"), FVector(125, 0, 90), FVector(120, 175, 18), TEXT("Paint"));
    AddBody(TEXT("Trunk"), FVector(-160, 0, 90), FVector(90, 175, 18), TEXT("Paint"));
    AddBody(TEXT("LeftHeadlight"), FVector(212, -60, 65), FVector(5, 35, 15), TEXT("Light"));
    AddBody(TEXT("RightHeadlight"), FVector(212, 60, 65), FVector(5, 35, 15), TEXT("Light"));
    for (int32 Index = 0; Index < 4; ++Index)
    {
        UStaticMeshComponent* Tire = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("WheelVisual%d"), Index));
        Tire->SetupAttachment(GetMesh());
        Tire->SetStaticMesh(Cylinder.Object);
        const float Radius = Index < 2 ? 39.0f : 40.0f;
        Tire->SetRelativeScale3D(FVector(Radius / 50, Radius / 50, 0.3f));
        Tire->SetRelativeRotation(FRotator(0, 0, 90));
        Tire->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Tire->ComponentTags.Add(TEXT("Tire"));
        WheelVisuals.Add(Tire);
    }
}

UChaosWheeledVehicleMovementComponent* ADelusionVehicle::GetDrivingMovement() const
{
    return CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());
}

float ADelusionVehicle::GetSpeedKmh() const { return Seat->GetSpeedKmh(); }

void ADelusionVehicle::BeginPlay()
{
    Super::BeginPlay();
    if (EngineSynth) { EngineSynth->Activate(); }
    DriverInput->ResetControls(true);
    if (SurfaceMaterial)
    {
        TArray<TObjectPtr<UStaticMeshComponent>> Parts = Bodywork;
        Parts.Append(WheelVisuals);
        for (UStaticMeshComponent* Part : Parts)
        {
            FLinearColor Color(0.025f, 0.22f, 0.65f);
            if (Part->ComponentHasTag(TEXT("Glass"))) { Color = FLinearColor(0.12f, 0.25f, 0.3f); }
            else if (Part->ComponentHasTag(TEXT("Tire"))) { Color = FLinearColor(0.012f, 0.015f, 0.018f); }
            else if (Part->ComponentHasTag(TEXT("Light"))) { Color = FLinearColor(0.9f, 0.9f, 0.65f); }
            if (UMaterialInstanceDynamic* Paint = Part->CreateDynamicMaterialInstance(0, SurfaceMaterial))
            {
                Paint->SetVectorParameterValue(TEXT("Color"), Color);
            }
        }
    }
    UE_LOG(LogTemp, Display, TEXT("DELUSION Chaos car spawned: four wheel setups, chassis physics %s"),
        GetMesh()->IsSimulatingPhysics() ? TEXT("active") : TEXT("inactive"));
}

void ADelusionVehicle::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UChaosWheeledVehicleMovementComponent* Movement = GetDrivingMovement();
    for (int32 Index = 0; Index < WheelVisuals.Num() && Index < Movement->Wheels.Num(); ++Index)
    {
        const UChaosVehicleWheel* Wheel = Movement->Wheels[Index];
        if (!Wheel) { continue; }
        const FVector Rest = GetMesh()->GetSocketTransform(Movement->WheelSetups[Index].BoneName, RTS_Component).GetLocation();
        WheelVisuals[Index]->SetRelativeLocation(Rest - Wheel->GetSuspensionAxis() * Wheel->GetSuspensionOffset());
        WheelVisuals[Index]->SetRelativeRotation(FRotator(-Wheel->GetRotationAngle(), Wheel->GetSteerAngle(), 90));
    }

    // Speed-sensitive camera look-ahead: shift the chase camera target forward at speed.
    const float ForwardSpeed = Movement->GetForwardSpeed();
    const float LookAhead = FMath::Clamp(ForwardSpeed * 0.05f, -150.0f, 250.0f);
    CameraBoom->TargetOffset = FVector(LookAhead, 0.0f, 0.0f);

    // Update engine audio based on vehicle state.
    if (EngineSynth)
    {
        const float SpeedKmh = FMath::Abs(ForwardSpeed) * 0.036f;
        const float Throttle = Movement->GetThrottleInput();
        const float TargetFreq = 80.0f + SpeedKmh * 6.0f;
        const float TargetVol = 0.08f + Throttle * 0.12f + (SpeedKmh > 1.0f ? 0.05f : 0.0f);
        EngineSynth->SetFrequency(TargetFreq);
        EngineSynth->SetVolume(FMath::Clamp(TargetVol, 0.05f, 0.35f));
    }
}

void ADelusionVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    DriverInput->BindDriverInput(PlayerInputComponent);
}

void ADelusionVehicle::UnPossessed()
{
    DriverInput->ClearDriverInput();
    Super::UnPossessed();
}

bool ADelusionVehicle::CanInteract_Implementation(APawn* Interactor) const { return Seat->CanEnter(Interactor); }
FText ADelusionVehicle::GetInteractionPrompt_Implementation() const
{
    return NSLOCTEXT("Delusion", "EnterVehicle", "E  Enter vehicle");
}
bool ADelusionVehicle::Interact_Implementation(APawn* Interactor) { return Seat->TryEnter(Interactor); }
