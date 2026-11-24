#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "../DelusionCharacter.h"
#include "../DelusionHUDWidget.h"
#include "../DelusionInteractionComponent.h"
#include "../DelusionPlayerController.h"
#include "../DelusionTestTerminal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputKeyEventArgs.h"
#include "Misc/AutomationTest.h"
#include "UObject/UObjectIterator.h"

namespace DelusionSmoke
{
struct FStep
{
    double Duration;
    TFunction<void()> Enter;
    TFunction<void()> Check;
    bool bWallClock;
};

class FPlayableCommand final : public IAutomationLatentCommand
{
public:
    explicit FPlayableCommand(FAutomationTestBase* InTest) : Test(InTest), Started(FPlatformTime::Seconds()) {}

    virtual bool Update() override
    {
        if (FPlatformTime::Seconds() - Started > 120.0)
        {
            Test->AddError(TEXT("Timed out waiting for the prototype runtime or input sequence."));
            Cleanup();
            return true;
        }
        if (!Player.IsValid())
        {
            for (const FWorldContext& Context : GEngine->GetWorldContexts())
            {
                UWorld* World = Context.World();
                if (!World || !World->IsGameWorld() || !World->GetMapName().Contains(TEXT("Prototype"))) { continue; }
                Player = Cast<ADelusionPlayerController>(World->GetFirstPlayerController());
                Pawn = Player.IsValid() ? Cast<ADelusionCharacter>(Player->GetPawn()) : nullptr;
                if (Pawn.IsValid())
                {
                    for (TActorIterator<ADelusionTestTerminal> It(World); It; ++It) { Terminal = *It; break; }
                    break;
                }
            }
            if (!Player.IsValid() || !Pawn.IsValid()) { Player.Reset(); return false; }
            if (!Terminal.IsValid()) { Test->AddError(TEXT("The level has no test terminal.")); return true; }
            BuildSteps();
            Advance();
        }
        if (!Pawn.IsValid()) { Test->AddError(TEXT("The player disappeared during the test.")); Cleanup(); return true; }
        PeakHeight = FMath::Max(PeakHeight, Pawn->GetActorLocation().Z);
        const FStep& Step = Steps[StepIndex];
        const double Elapsed = Step.bWallClock ? FPlatformTime::Seconds() - StepRealStart
            : Pawn->GetWorld()->GetTimeSeconds() - StepGameStart;
        if (Elapsed < Step.Duration) { return false; }
        if (Step.Check) { Step.Check(); }
        if (++StepIndex == Steps.Num())
        {
            Cleanup();
            Test->AddInfo(TEXT("Completed the live prototype input, physics, interaction and pause checks."));
            return true;
        }
        EnterStep();
        return false;
    }

private:
    void Add(double Duration, TFunction<void()> Enter, TFunction<void()> Check = {}, bool bWallClock = false)
    {
        Steps.Add({Duration, MoveTemp(Enter), MoveTemp(Check), bWallClock});
    }
    void Advance() { StepIndex = 0; EnterStep(); }
    void EnterStep()
    {
        if (Steps[StepIndex].Enter) { Steps[StepIndex].Enter(); }
        StepRealStart = FPlatformTime::Seconds();
        StepGameStart = Pawn->GetWorld()->GetTimeSeconds();
    }
    void Key(FKey InputKey, EInputEvent Event, float Amount = 1.0f)
    {
        Player->InputKey(FInputKeyEventArgs::CreateSimulated(InputKey, Event, Amount));
    }
    void Place(FVector Position, float Yaw = 0.0f)
    {
        Player->FlushPressedKeys();
        Pawn->GetCharacterMovement()->StopMovementImmediately();
        Pawn->SetActorLocation(Position, false, nullptr, ETeleportType::TeleportPhysics);
        Player->SetControlRotation(FRotator(-10, Yaw, 0));
    }
    UDelusionHUDWidget* HUD() const
    {
        for (TObjectIterator<UDelusionHUDWidget> It; It; ++It)
        {
            if (It->GetOwningPlayer() == Player.Get() && It->IsInViewport()) { return *It; }
        }
        return nullptr;
    }
    bool HasVisibleText(const FString& Label) const
    {
        UDelusionHUDWidget* Interface = HUD();
        if (!Interface || !Interface->WidgetTree) { return false; }
        TArray<UWidget*> Widgets;
        Interface->WidgetTree->GetAllWidgets(Widgets);
        for (UWidget* Widget : Widgets)
        {
            const UTextBlock* Text = Cast<UTextBlock>(Widget);
            if (!Text || !Text->GetText().ToString().Contains(Label)) { continue; }
            bool bVisible = true;
            for (const UWidget* Parent = Widget; Parent; Parent = Parent->GetParent())
            {
                bVisible &= Parent->GetVisibility() != ESlateVisibility::Collapsed && Parent->GetVisibility() != ESlateVisibility::Hidden;
            }
            if (bVisible) { return true; }
        }
        return false;
    }
    void ClickResume()
    {
        UDelusionHUDWidget* Interface = HUD();
        if (!Interface || !Interface->WidgetTree) { Test->AddError(TEXT("HUD is missing.")); return; }
        TArray<UWidget*> Widgets;
        Interface->WidgetTree->GetAllWidgets(Widgets);
        for (UWidget* Widget : Widgets)
        {
            UButton* Button = Cast<UButton>(Widget);
            const UTextBlock* Text = Button ? Cast<UTextBlock>(Button->GetContent()) : nullptr;
            if (Text && Text->GetText().ToString() == TEXT("Resume"))
            {
                Test->TestTrue(TEXT("Resume button is bound"), Button->OnClicked.IsBound());
                Button->OnClicked.Broadcast();
                return;
            }
        }
        Test->AddError(TEXT("Resume button is missing."));
    }
    void BuildSteps()
    {
        Add(0.5, [this] { Place(FVector(1200, -250, 105)); }, [this]
        {
            Test->TestTrue(TEXT("Character lands on the road"), Pawn->GetCharacterMovement()->IsMovingOnGround());
            Test->TestTrue(TEXT("Native HUD displays DELUSION"), HasVisibleText(TEXT("DELUSION")));
            Origin = Pawn->GetActorLocation();
        });
        Add(1.0, [this] { Key(EKeys::W, IE_Pressed); }, [this]
        {
            WalkDistance = FVector::Dist2D(Origin, Pawn->GetActorLocation());
            Test->AddInfo(FString::Printf(TEXT("Walk distance over one game second: %.1f cm"), WalkDistance));
            Test->TestTrue(TEXT("W moves the character forward"), WalkDistance > 200 && WalkDistance < 600);
            Key(EKeys::W, IE_Released, 0);
            Place(FVector(1200, -250, 105));
        });
        Add(0.3, [] {}, [this] { Origin = Pawn->GetActorLocation(); });
        Add(1.0, [this] { Key(EKeys::LeftShift, IE_Pressed); Key(EKeys::W, IE_Pressed); }, [this]
        {
            const double SprintDistance = FVector::Dist2D(Origin, Pawn->GetActorLocation());
            Test->AddInfo(FString::Printf(TEXT("Sprint distance over one game second: %.1f cm"), SprintDistance));
            Test->TestTrue(TEXT("Shift makes movement faster"), SprintDistance > WalkDistance * 1.35);
            Key(EKeys::W, IE_Released, 0); Key(EKeys::LeftShift, IE_Released, 0);
            Place(FVector(1200, -250, 105));
        });
        Add(0.3, [] {}, [this] { Origin = Pawn->GetActorLocation(); PeakHeight = Origin.Z; });
        Add(0.2, [this] { Key(EKeys::SpaceBar, IE_Pressed); }, [this] { Key(EKeys::SpaceBar, IE_Released, 0); });
        Add(1.5, [] {}, [this]
        {
            Test->TestTrue(TEXT("Space produces a real jump"), PeakHeight > Origin.Z + 40);
            Test->TestTrue(TEXT("Gravity returns the character to ground"), Pawn->GetCharacterMovement()->IsMovingOnGround());
        });
        Add(0.2, [this] { Player->SetControlRotation(FRotator(-10, 0, 0)); Key(EKeys::MouseX, IE_Axis, 80); Key(EKeys::MouseY, IE_Axis, 10000); }, [this]
        {
            const FRotator Rotation = Player->GetControlRotation().GetNormalized();
            Test->AddInfo(FString::Printf(TEXT("Mouse input result: yaw=%.3f pitch=%.3f"), Rotation.Yaw, Rotation.Pitch));
            Test->TestTrue(TEXT("Mouse changes camera yaw"), FMath::Abs(Rotation.Yaw) > 1);
            Test->TestTrue(TEXT("Mouse changes camera pitch"), FMath::Abs(Rotation.Pitch + 10) > 1);
            Test->TestTrue(TEXT("Camera pitch remains constrained"), Rotation.Pitch >= -60.1 && Rotation.Pitch <= 35.1);
            Place(FVector(-1400, -2300, 105), 180);
        });
        Add(0.3, [] {});
        Add(1.0, [this] { Key(EKeys::W, IE_Pressed); }, [this]
        {
            Key(EKeys::W, IE_Released, 0);
            Test->TestTrue(TEXT("A building blocks walking through it"), Pawn->GetActorLocation().X > -1540 && Pawn->GetActorLocation().X < -1440);
            Player->SetControlRotation(FRotator(-10, 0, 0));
        });
        Add(0.3, [] {}, [this]
        {
            const USpringArmComponent* Boom = Pawn->FindComponentByClass<USpringArmComponent>();
            Test->TestTrue(TEXT("Camera retracts against the building"), Boom && Boom->IsCollisionFixApplied());
            Place(FVector(-1800, -300, 105));
        });
        Add(0.3, [] {}, [this]
        {
            Test->TestTrue(TEXT("No interaction prompt outside range"), Player->GetInteraction()->GetPrompt().IsEmpty());
            Test->TestFalse(TEXT("Interaction outside range is rejected"), Player->GetInteraction()->TryInteract());
            Place(FVector(-1490, -450, 105));
        });
        Add(0.3, [] {}, [this]
        {
            Test->TestTrue(TEXT("Nearby terminal is detected"), Player->GetInteraction()->GetFocusedTarget() == Terminal.Get());
            Test->TestTrue(TEXT("HUD shows the interaction prompt"), HasVisibleText(TEXT("Activate terminal")));
        });
        Add(0.2, [this] { Key(EKeys::E, IE_Pressed); }, [this]
        {
            Key(EKeys::E, IE_Released, 0);
            Test->TestTrue(TEXT("E activates the terminal"), Terminal->bActivated);
            Test->TestTrue(TEXT("HUD shows interaction feedback"), HasVisibleText(TEXT("Terminal updated")));
        });
        Add(0.2, [this] { Key(EKeys::Escape, IE_Pressed); }, [this]
        {
            Test->TestTrue(TEXT("Escape pauses gameplay"), Player->IsPaused());
            Test->TestTrue(TEXT("Pause menu displays Resume and Quit"), HasVisibleText(TEXT("Resume")) && HasVisibleText(TEXT("Quit")));
            Key(EKeys::Escape, IE_Released, 0);
            PausedTime = Pawn->GetWorld()->GetTimeSeconds();
        }, true);
        Add(0.25, [] {}, [this]
        {
            Test->TestTrue(TEXT("Paused game time stays frozen"), FMath::IsNearlyEqual(PausedTime, Pawn->GetWorld()->GetTimeSeconds(), 0.01));
            ClickResume();
            Test->TestFalse(TEXT("Resume button resumes gameplay"), Player->IsPaused());
        }, true);
        Add(0.2, [] {});
        Add(0.2, [this] { Key(EKeys::Escape, IE_Pressed); }, [this] { Key(EKeys::Escape, IE_Released, 0); }, true);
        Add(0.2, [] {}, [this] { Key(EKeys::Escape, IE_Pressed); }, true);
        Add(0.2, [] {}, [this]
        {
            Key(EKeys::Escape, IE_Released, 0);
            Test->TestFalse(TEXT("Escape also resumes gameplay"), Player->IsPaused());
        }, true);
    }
    void Cleanup()
    {
        if (Player.IsValid()) { Player->ResumeGame(); Player->FlushPressedKeys(); }
        if (Pawn.IsValid() && Player.IsValid())
        {
            Pawn->SetSprinting(false);
            Pawn->StopJumping();
            Place(FVector(-1800, -300, 105));
        }
    }

    FAutomationTestBase* Test;
    TWeakObjectPtr<ADelusionPlayerController> Player;
    TWeakObjectPtr<ADelusionCharacter> Pawn;
    TWeakObjectPtr<ADelusionTestTerminal> Terminal;
    TArray<FStep> Steps;
    int32 StepIndex = INDEX_NONE;
    double Started;
    double StepRealStart = 0;
    float StepGameStart = 0;
    float PausedTime = 0;
    double PeakHeight = 0;
    double WalkDistance = 0;
    FVector Origin = FVector::ZeroVector;
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelusionPlayableSmoke, "Delusion.Sprint0.Playable",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FDelusionPlayableSmoke::RunTest(const FString& Parameters)
{
    ADD_LATENT_AUTOMATION_COMMAND(DelusionSmoke::FPlayableCommand(this));
    return true;
}

#endif
