#include "DelusionCharacter.h"

#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/StaticMesh.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "UObject/ConstructorHelpers.h"

ADelusionCharacter::ADelusionCharacter()
{
    GetCapsuleComponent()->InitCapsuleSize(34.0f, 92.0f);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Prefer the imported mannequin skeletal mesh and its animation blueprint.
    // Keep the primitive parts as a fallback if the asset is unavailable.
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> MannequinMesh(TEXT("/Game/Mannequin/Character/Mesh/SK_Mannequin.SK_Mannequin"));
    const bool bHasMannequin = MannequinMesh.Succeeded();
    if (bHasMannequin)
    {
        GetMesh()->SetSkeletalMeshAsset(MannequinMesh.Object);
        static ConstructorHelpers::FClassFinder<UAnimInstance> MannequinAnimBP(TEXT("/Game/Mannequin/Animations/ThirdPerson_AnimBP.ThirdPerson_AnimBP_C"));
        if (MannequinAnimBP.Class) { GetMesh()->SetAnimInstanceClass(MannequinAnimBP.Class); }
        GetMesh()->SetHiddenInGame(false);
    }
    else
    {
        GetMesh()->SetHiddenInGame(true);
    }

    UCharacterMovementComponent* Movement = GetCharacterMovement();
    Movement->bOrientRotationToMovement = true;
    Movement->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    Movement->MaxWalkSpeed = WalkSpeed;
    Movement->JumpZVelocity = 540.0f;
    Movement->AirControl = 0.3f;
    Movement->BrakingDecelerationWalking = 1800.0f;
    Movement->MaxAcceleration = 1800.0f;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 360.0f;
    CameraBoom->SocketOffset = FVector(0.0f, 45.0f, 55.0f);
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bDoCollisionTest = true;
    CameraBoom->ProbeSize = 12.0f;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 16.0f;
    CameraBoom->bEnableCameraRotationLag = true;
    CameraBoom->CameraRotationLagSpeed = 20.0f;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
    FollowCamera->FieldOfView = 85.0f;

    // A small original mannequin made from engine primitives needs no asset pack.
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    auto AddBodyPart = [this, bHasMannequin](const TCHAR* Name, UStaticMesh* BodyMesh, FVector Position, FVector Scale)
    {
        UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(Name);
        Part->SetupAttachment(RootComponent);
        Part->SetStaticMesh(BodyMesh);
        Part->SetRelativeLocation(Position);
        Part->SetRelativeScale3D(Scale);
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        // Keep the primitive fallback available but hidden when the skeletal mesh is used.
        Part->SetHiddenInGame(bHasMannequin);
    };
    AddBodyPart(TEXT("Torso"), Cube.Object, FVector(0, 0, 5), FVector(0.38f, 0.52f, 0.72f));
    AddBodyPart(TEXT("Head"), Sphere.Object, FVector(0, 0, 62), FVector(0.38f));
    AddBodyPart(TEXT("LeftLeg"), Cube.Object, FVector(0, -14, -60), FVector(0.24f, 0.20f, 0.64f));
    AddBodyPart(TEXT("RightLeg"), Cube.Object, FVector(0, 14, -60), FVector(0.24f, 0.20f, 0.64f));
    AddBodyPart(TEXT("LeftArm"), Cube.Object, FVector(0, -36, 2), FVector(0.18f, 0.16f, 0.68f));
    AddBodyPart(TEXT("RightArm"), Cube.Object, FVector(0, 36, 2), FVector(0.18f, 0.16f, 0.68f));
}

void ADelusionCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    APlayerController* PlayerController = Cast<APlayerController>(Controller);
    ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;
    if (!Input || !LocalPlayer)
    {
        UE_LOG(LogTemp, Error, TEXT("DELUSION requires a local player with Enhanced Input."));
        return;
    }
    if (InputSubsystem.IsValid() && MovementMapping)
    {
        InputSubsystem->RemoveMappingContext(MovementMapping);
    }
    MovementMapping = NewObject<UInputMappingContext>(this);
    MoveAction = NewObject<UInputAction>(this);
    MoveAction->ValueType = EInputActionValueType::Axis2D;
    LookAction = NewObject<UInputAction>(this);
    LookAction->ValueType = EInputActionValueType::Axis2D;
    JumpAction = NewObject<UInputAction>(this);
    SprintAction = NewObject<UInputAction>(this);

    auto MapAxis = [this](UInputAction* Action, FKey Key, bool bNegate, bool bSwizzle)
    {
        FEnhancedActionKeyMapping& Mapping = MovementMapping->MapKey(Action, Key);
        if (bNegate)
        {
            Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(MovementMapping));
        }
        if (bSwizzle)
        {
            UInputModifierSwizzleAxis* Swizzle = NewObject<UInputModifierSwizzleAxis>(MovementMapping);
            Swizzle->Order = EInputAxisSwizzle::YXZ;
            Mapping.Modifiers.Add(Swizzle);
        }
    };
    MapAxis(MoveAction, EKeys::W, false, true);
    MapAxis(MoveAction, EKeys::S, true, true);
    MapAxis(MoveAction, EKeys::D, false, false);
    MapAxis(MoveAction, EKeys::A, true, false);
    MapAxis(LookAction, EKeys::MouseX, false, false);
    MapAxis(LookAction, EKeys::MouseY, true, true);
    MovementMapping->MapKey(JumpAction, EKeys::SpaceBar);
    MovementMapping->MapKey(SprintAction, EKeys::LeftShift);

    Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADelusionCharacter::Move);
    Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADelusionCharacter::Look);
    Input->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
    Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
    Input->BindAction(JumpAction, ETriggerEvent::Canceled, this, &ACharacter::StopJumping);
    Input->BindAction(SprintAction, ETriggerEvent::Started, this, &ADelusionCharacter::StartSprint);
    Input->BindAction(SprintAction, ETriggerEvent::Completed, this, &ADelusionCharacter::StopSprint);
    Input->BindAction(SprintAction, ETriggerEvent::Canceled, this, &ADelusionCharacter::StopSprint);
    InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
    if (InputSubsystem.IsValid())
    {
        InputSubsystem->AddMappingContext(MovementMapping, 0);
    }
}

void ADelusionCharacter::Move(const FInputActionValue& Value)
{
    if (!Controller) { return; }
    const FVector2D Axes = Value.Get<FVector2D>().GetClampedToMaxSize(1.0);
    const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
    AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Axes.Y);
    AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Axes.X);
}

void ADelusionCharacter::Look(const FInputActionValue& Value)
{
    const FVector2D Axes = Value.Get<FVector2D>();
    // Mouse input already contains the frame's accumulated mouse delta.
    AddControllerYawInput(Axes.X * MouseSensitivity);
    AddControllerPitchInput(Axes.Y * MouseSensitivity);
}

void ADelusionCharacter::SetSprinting(bool bSprinting)
{
    GetCharacterMovement()->MaxWalkSpeed = bSprinting ? SprintSpeed : WalkSpeed;
}

void ADelusionCharacter::StartSprint() { SetSprinting(true); }
void ADelusionCharacter::StopSprint() { SetSprinting(false); }

void ADelusionCharacter::UnPossessed()
{
    if (InputSubsystem.IsValid() && MovementMapping)
    {
        InputSubsystem->RemoveMappingContext(MovementMapping);
    }
    MovementMapping = nullptr;
    InputSubsystem.Reset();
    SetSprinting(false);
    StopJumping();
    Super::UnPossessed();
}

void ADelusionCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (InputSubsystem.IsValid() && MovementMapping)
    {
        InputSubsystem->RemoveMappingContext(MovementMapping);
    }
    Super::EndPlay(EndPlayReason);
}
