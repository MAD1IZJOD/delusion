#include "DelusionPlayerController.h"

#include "Camera/PlayerCameraManager.h"
#include "DelusionCharacter.h"
#include "DelusionHUDWidget.h"
#include "DelusionInteractionComponent.h"
#include "DelusionVehicle.h"
#include "DelusionVehicleInputComponent.h"
#include "DelusionVehicleSeatComponent.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Kismet/KismetSystemLibrary.h"

ADelusionPlayerController::ADelusionPlayerController()
{
    Interaction = CreateDefaultSubobject<UDelusionInteractionComponent>(TEXT("Interaction"));
    bShouldPerformFullTickWhenPaused = true;
}

void ADelusionPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (!IsLocalController()) { return; }
    if (PlayerCameraManager)
    {
        PlayerCameraManager->ViewPitchMin = -60.0f;
        PlayerCameraManager->ViewPitchMax = 35.0f;
    }
    HUDWidget = CreateWidget<UDelusionHUDWidget>(this, UDelusionHUDWidget::StaticClass());
    if (HUDWidget) { HUDWidget->AddToViewport(); }
    SetInputMode(FInputModeGameOnly());
    bShowMouseCursor = false;
}

void ADelusionPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent);
    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    if (!Input || !LocalPlayer) { return; }
    UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
    if (!Subsystem) { return; }
    ActionMapping = NewObject<UInputMappingContext>(this);
    InteractAction = NewObject<UInputAction>(this);
    PauseAction = NewObject<UInputAction>(this);
    PauseAction->bTriggerWhenPaused = true;
    ActionMapping->MapKey(InteractAction, EKeys::E);
    ActionMapping->MapKey(PauseAction, EKeys::Escape);
    Input->BindAction(InteractAction, ETriggerEvent::Started, this, &ADelusionPlayerController::Interact);
    Input->BindAction(PauseAction, ETriggerEvent::Started, this, &ADelusionPlayerController::TogglePause);
    Subsystem->AddMappingContext(ActionMapping, 1);
}

void ADelusionPlayerController::Interact()
{
    if (IsPaused()) { return; }
    if (ADelusionVehicle* ControlledVehicle = Cast<ADelusionVehicle>(GetPawn()))
    {
        if (ControlledVehicle->GetSeat()->IsTransitioning())
        {
            return;
        }
        const bool bExited = ControlledVehicle->GetSeat()->TryExit(this);
        if (HUDWidget)
        {
            HUDWidget->ShowMessage(bExited
                ? NSLOCTEXT("Delusion", "VehicleExited", "Back on foot")
                : ControlledVehicle->GetSpeedKmh() > ControlledVehicle->GetSeat()->GetMaximumExitSpeed()
                    ? NSLOCTEXT("Delusion", "ExitTooFast", "Slow down before exiting")
                    : NSLOCTEXT("Delusion", "ExitBlocked", "No clear exit. Move the vehicle away from obstacles."));
        }
        return;
    }
    if (Interaction->TryInteract() && HUDWidget)
    {
        if (Cast<ADelusionVehicle>(GetPawn()))
        {
            HUDWidget->ShowMessage(NSLOCTEXT("Delusion", "VehicleEntered", "Vehicle entered"));
        }
        else { HUDWidget->ShowInteractionFeedback(); }
    }
}

void ADelusionPlayerController::TogglePause()
{
    const bool bPauseNow = !IsPaused();
    if (!SetPause(bPauseNow)) { return; }
    FlushPressedKeys();
    SetIgnoreMoveInput(bPauseNow);
    SetIgnoreLookInput(bPauseNow);
    if (ADelusionCharacter* ControlledCharacter = Cast<ADelusionCharacter>(GetPawn()))
    {
        ControlledCharacter->SetSprinting(false);
        ControlledCharacter->StopJumping();
    }
    if (ADelusionVehicle* ControlledVehicle = Cast<ADelusionVehicle>(GetPawn()))
    {
        ControlledVehicle->GetDriverInput()->ResetControls(bPauseNow);
    }
    bShowMouseCursor = bPauseNow;
    if (HUDWidget) { HUDWidget->SetPauseMenuVisible(bPauseNow); }
    if (bPauseNow)
    {
        FInputModeGameAndUI InputMode;
        InputMode.SetHideCursorDuringCapture(false);
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        if (HUDWidget) { InputMode.SetWidgetToFocus(HUDWidget->TakeWidget()); }
        SetInputMode(InputMode);
        if (HUDWidget) { HUDWidget->SetKeyboardFocus(); }
    }
    else
    {
        SetInputMode(FInputModeGameOnly());
    }
    UE_LOG(LogTemp, Display, TEXT("DELUSION %s"), bPauseNow ? TEXT("paused") : TEXT("resumed"));
}

void ADelusionPlayerController::ResumeGame()
{
    if (IsPaused()) { TogglePause(); }
}

void ADelusionPlayerController::QuitGame()
{
    UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}

void ADelusionPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            if (ActionMapping) { Subsystem->RemoveMappingContext(ActionMapping); }
        }
    }
    if (HUDWidget) { HUDWidget->RemoveFromParent(); }
    Super::EndPlay(EndPlayReason);
}
