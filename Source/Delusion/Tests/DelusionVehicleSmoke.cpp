#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Blueprint/WidgetTree.h"
#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TextBlock.h"
#include "../DelusionCharacter.h"
#include "../DelusionHUDWidget.h"
#include "../DelusionInteractionComponent.h"
#include "../DelusionPlayerController.h"
#include "../DelusionVehicle.h"
#include "../DelusionVehicleInputComponent.h"
#include "../DelusionVehicleSeatComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputKeyEventArgs.h"
#include "Misc/AutomationTest.h"
#include "UObject/UObjectIterator.h"

namespace DelusionVehicleSmoke
{
struct FStep
{
    FString Name;
    double Duration;
    TFunction<void()> Enter;
    TFunction<void()> Check;
    bool bRealTime;
};

class FVehicleCommand final : public IAutomationLatentCommand
{
public:
    explicit FVehicleCommand(FAutomationTestBase* InTest) : Test(InTest), Started(FPlatformTime::Seconds()) {}

    virtual bool Update() override
    {
        if (FPlatformTime::Seconds() - Started > 150)
        {
            Test->AddError(TEXT("Vehicle gameplay test timed out."));
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
                Foot = Player.IsValid() ? Cast<ADelusionCharacter>(Player->GetPawn()) : nullptr;
                for (TActorIterator<ADelusionVehicle> It(World); It; ++It) { Car = *It; break; }
                if (Player.IsValid() && Foot.IsValid() && Car.IsValid()) { break; }
                Player.Reset();
            }
            if (!Player.IsValid()) { return false; }
            OriginalFoot = Foot->GetActorTransform();
            OriginalCar = Car->GetActorTransform();
            BuildSteps();
            StepIndex = 0;
            EnterStep();
        }
        if (!Car.IsValid() || !Foot.IsValid())
        {
            Test->AddError(TEXT("A preserved gameplay pawn disappeared."));
            Cleanup();
            return true;
        }
        PeakHeight = FMath::Max(PeakHeight, Foot->GetActorLocation().Z);
        if (bRecordBraking && !bStopped && Car->GetDrivingMovement()->GetForwardSpeed() <= 0)
        {
            bStopped = true;
            BrakingDistance = FVector::Dist(BrakeOrigin, Car->GetActorLocation());
        }
        const FStep& Step = Steps[StepIndex];
        const double Elapsed = Step.bRealTime ? FPlatformTime::Seconds() - RealStart
            : Car->GetWorld()->GetTimeSeconds() - GameStart;
        if (Elapsed < Step.Duration) { return false; }
        if (Step.Check) { Step.Check(); }
        if (Test->HasAnyErrors()) { Cleanup(); return true; }
        if (++StepIndex >= Steps.Num())
        {
            Cleanup();
            Test->AddInfo(TEXT("Completed native input, Chaos physics, safe possession and on-foot regression checks."));
            return true;
        }
        EnterStep();
        return false;
    }

private:
    void Add(const TCHAR* Name, double Duration, TFunction<void()> Enter, TFunction<void()> Check = {}, bool bRealTime = false)
    {
        Steps.Add({Name, Duration, MoveTemp(Enter), MoveTemp(Check), bRealTime});
    }
    void EnterStep()
    {
        Test->AddInfo(Steps[StepIndex].Name);
        if (Steps[StepIndex].Enter) { Steps[StepIndex].Enter(); }
        RealStart = FPlatformTime::Seconds();
        GameStart = Car->GetWorld()->GetTimeSeconds();
    }
    void Key(FKey Input, EInputEvent Event, float Amount = 1)
    {
        Player->InputKey(FInputKeyEventArgs::CreateSimulated(Input, Event, Amount));
    }
    void PressE(const TCHAR* Name, TFunction<void()> Check)
    {
        Add(Name, 0.4, [this] { Key(EKeys::E, IE_Pressed); }, MoveTemp(Check));
        Add(TEXT("Release interaction"), 0.15, [this] { Key(EKeys::E, IE_Released, 0); });
    }
    bool HasText(const FString& Label) const
    {
        for (TObjectIterator<UDelusionHUDWidget> It; It; ++It)
        {
            if (It->GetOwningPlayer() != Player.Get() || !It->IsInViewport() || !It->WidgetTree) { continue; }
            TArray<UWidget*> Widgets;
            It->WidgetTree->GetAllWidgets(Widgets);
            for (const UWidget* Widget : Widgets)
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
        }
        return false;
    }
    void PlaceFoot(FVector Location, float Yaw)
    {
        Player->FlushPressedKeys();
        Foot->GetCharacterMovement()->StopMovementImmediately();
        Foot->SetActorLocation(Location, false, nullptr, ETeleportType::TeleportPhysics);
        Player->SetControlRotation(FRotator(-10, Yaw, 0));
    }
    void ResetCar(FVector Location, float Yaw)
    {
        // Fixture placement only; every driving step below uses native input and Chaos simulation.
        Player->FlushPressedKeys();
        Car->GetDrivingMovement()->ResetVehicle();
        Car->GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);
        Car->GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        Car->SetActorLocationAndRotation(Location, FRotator(0, Yaw, 0), false, nullptr, ETeleportType::TeleportPhysics);
        Car->GetDriverInput()->ResetControls(Player->GetPawn() != Car.Get());
        Player->SetControlRotation(FRotator(-12, Yaw, 0));
    }
    int32 Contacts() const
    {
        int32 Count = 0;
        for (int32 Index = 0; Index < Car->GetDrivingMovement()->Wheels.Num(); ++Index)
        {
            Count += Car->GetDrivingMovement()->GetWheelState(Index).bInContact ? 1 : 0;
        }
        return Count;
    }
    void RemoveBlockers()
    {
        for (TWeakObjectPtr<AActor>& Blocker : Blockers) { if (Blocker.IsValid()) { Blocker->Destroy(); } }
        Blockers.Reset();
    }
    void BlockExits()
    {
        const FVector Offsets[] = {FVector(0,-190,0), FVector(0,190,0), FVector(-300,0,0),
            FVector(300,0,0), FVector(-180,-220,0), FVector(-180,220,0)};
        const FRotator Yaw(0, Car->GetActorRotation().Yaw, 0);
        for (FVector Offset : Offsets)
        {
            AActor* Blocker = Car->GetWorld()->SpawnActor<AActor>();
            Blocker->SetFlags(RF_Transient);
            UBoxComponent* Box = NewObject<UBoxComponent>(Blocker);
            Blocker->SetRootComponent(Box);
            Box->SetBoxExtent(FVector(25,25,150));
            Box->SetCollisionProfileName(TEXT("BlockAll"));
            Box->RegisterComponent();
            Blocker->SetActorLocation(Car->GetActorLocation() + Yaw.RotateVector(Offset) + FVector(0,0,100));
            Blockers.Add(Blocker);
        }
    }
    void BuildSteps()
    {
        Add(TEXT("Settle vehicle"), 2.0, [] {}, [this]
        {
            Test->TestTrue(TEXT("Chassis uses live physics"), Car->GetMesh()->IsSimulatingPhysics());
            Test->TestEqual(TEXT("Four Chaos wheels"), Car->GetDrivingMovement()->Wheels.Num(), 4);
            Test->TestEqual(TEXT("All wheels contact ground at rest"), Contacts(), 4);
            Test->TestTrue(TEXT("Stable resting height"), Car->GetActorLocation().Z > 0 && Car->GetActorLocation().Z < 150);
            Test->TestTrue(TEXT("Upright at rest"), Car->GetActorUpVector().Z > 0.95);
            Test->AddInfo(FString::Printf(TEXT("Rest Z %.2f cm; mass %.0f kg; speed %.3f km/h"),
                Car->GetActorLocation().Z, Car->GetMesh()->GetMass(), Car->GetSpeedKmh()));
        });
        Add(TEXT("Reject distant entry"), 0.3, [this]
        {
            PlaceFoot(Car->GetActorLocation() + FVector(0,-600,95), 90);
        }, [this]
        {
            Test->TestFalse(TEXT("Distant vehicle entry is rejected"), Car->GetSeat()->TryEnter(Foot.Get()));
            Test->TestFalse(TEXT("No distant entry prompt"), Player->GetInteraction()->GetPrompt().ToString().Contains(TEXT("Enter vehicle")));
            PlaceFoot(Car->GetActorLocation() + FVector(0,-380,95), 90);
        });
        Add(TEXT("Ground approach fixture"), 0.3, [] {});
        Add(TEXT("Walk toward car"), 0.75, [this]
        {
            Origin = Foot->GetActorLocation();
            Key(EKeys::W, IE_Pressed);
        }, [this]
        {
            Key(EKeys::W, IE_Released, 0);
            Test->TestTrue(TEXT("On-foot movement approaches vehicle"), FVector::Dist(Origin, Foot->GetActorLocation()) > 100);
            Test->TestTrue(TEXT("Enter prompt appears"), HasText(TEXT("Enter vehicle")));
        });
        PressE(TEXT("Enter through interaction"), [this]
        {
            Test->TestTrue(TEXT("Controller possesses car"), Player->GetPawn() == Car.Get());
            Test->TestTrue(TEXT("Same character is preserved"), Car->GetSeat()->GetOccupant() == Foot.Get());
            Test->TestTrue(TEXT("Character hidden and noncolliding"), Foot->IsHidden() && !Foot->GetActorEnableCollision());
            Test->TestTrue(TEXT("Vehicle camera is active"), Player->GetViewTarget() == Car.Get());
            Test->TestTrue(TEXT("Speed HUD is visible"), HasText(TEXT("km/h")));
        });
        Add(TEXT("Accelerate"), 1.5, [this]
        {
            Origin = Car->GetActorLocation();
            Key(EKeys::W, IE_Pressed);
        }, [this]
        {
            Test->TestTrue(TEXT("Throttle moves physical chassis"), FVector::Dist(Origin, Car->GetActorLocation()) > 100);
            Test->TestTrue(TEXT("Car gains driving speed"), Car->GetSpeedKmh() > 5);
            Test->TestTrue(TEXT("Wheels remain in contact"), Contacts() >= 3);
            Test->TestTrue(TEXT("Driven wheels rotate"), FMath::Abs(Car->GetDrivingMovement()->Wheels[0]->GetRotationAngularVelocity()) > 1);
            Test->AddInfo(FString::Printf(TEXT("Acceleration: %.2f m, %.2f km/h"),
                FVector::Dist(Origin, Car->GetActorLocation())/100, Car->GetSpeedKmh()));
        });
        PressE(TEXT("Reject fast exit"), [this]
        {
            Test->TestTrue(TEXT("Moving exit retains vehicle possession"), Player->GetPawn() == Car.Get());
            Test->TestTrue(TEXT("Fast exit explains restriction"), HasText(TEXT("Slow down")));
        });
        Add(TEXT("Brake and reverse"), 2.0, [this]
        {
            Key(EKeys::W, IE_Released, 0);
            BrakeOrigin = Car->GetActorLocation();
            bRecordBraking = true;
            Key(EKeys::S, IE_Pressed);
        }, [this]
        {
            Key(EKeys::S, IE_Released, 0);
            bRecordBraking = false;
            Test->TestTrue(TEXT("Braking crosses a complete stop"), bStopped);
            Test->TestTrue(TEXT("Braking distance stays within 12 m"), BrakingDistance < 1200);
            Test->TestTrue(TEXT("S engages reverse"), Car->GetDrivingMovement()->GetCurrentGear() < 0);
            Test->TestTrue(TEXT("Reverse has physical backward velocity"), Car->GetDrivingMovement()->GetForwardSpeed() < -100);
            Test->AddInfo(FString::Printf(TEXT("Brake distance %.2f m; reverse speed %.2f km/h"),
                BrakingDistance/100, Car->GetDrivingMovement()->GetForwardSpeed()*0.036f));
        });
        Add(TEXT("Handbrake"), 1.3, [this] { Key(EKeys::SpaceBar, IE_Pressed); }, [this]
        {
            Test->TestTrue(TEXT("Handbrake input is active"), Car->GetDrivingMovement()->GetHandbrakeInput());
            Test->TestTrue(TEXT("Handbrake stops the car"), Car->GetSpeedKmh() < 3);
            Key(EKeys::SpaceBar, IE_Released, 0);
        });
        Add(TEXT("Steer right at low speed"), 1.1, [this]
        {
            Heading = Car->GetActorRotation().Yaw;
            Key(EKeys::W, IE_Pressed);
            Key(EKeys::D, IE_Pressed);
        }, [this]
        {
            Test->TestTrue(TEXT("D turns vehicle right"), FMath::FindDeltaAngleDegrees(Heading, Car->GetActorRotation().Yaw) > 1);
            Test->TestTrue(TEXT("Front wheels steer right"), Car->GetDrivingMovement()->Wheels[0]->GetSteerAngle() > 5);
            Heading = Car->GetActorRotation().Yaw;
            Key(EKeys::D, IE_Released, 0);
        });
        Add(TEXT("Steer left"), 1.2, [this] { Key(EKeys::A, IE_Pressed); }, [this]
        {
            Test->TestTrue(TEXT("A turns vehicle left"), FMath::FindDeltaAngleDegrees(Heading, Car->GetActorRotation().Yaw) < -1);
            Test->TestTrue(TEXT("Front wheels steer left"), Car->GetDrivingMovement()->Wheels[0]->GetSteerAngle() < -5);
            Key(EKeys::A, IE_Released, 0);
        });
        Add(TEXT("Steer at modest driving speed"), 0.6, [this] { Key(EKeys::D, IE_Pressed); }, [this]
        {
            Test->TestTrue(TEXT("Car stays upright while turning"), Car->GetActorUpVector().Z > 0.8);
            Test->AddInfo(FString::Printf(TEXT("Turning speed %.2f km/h; steering %.2f degrees"),
                Car->GetSpeedKmh(), Car->GetDrivingMovement()->Wheels[0]->GetSteerAngle()));
            Key(EKeys::W, IE_Released, 0);
            Key(EKeys::D, IE_Released, 0);
        });
        Add(TEXT("Stop after turning"), 1.5, [this] { Key(EKeys::SpaceBar, IE_Pressed); }, [this]
        {
            Key(EKeys::SpaceBar, IE_Released, 0);
            Test->TestTrue(TEXT("Car settles after turning"), Car->GetSpeedKmh() < 3);
        });
        Add(TEXT("Vehicle mouse camera"), 0.3, [this]
        {
            Heading = Player->GetControlRotation().Yaw;
            Key(EKeys::MouseX, IE_Axis, 80);
        }, [this]
        {
            Test->TestTrue(TEXT("Mouse rotates vehicle camera"), FMath::Abs(FMath::FindDeltaAngleDegrees(Heading, Player->GetControlRotation().Yaw)) > 8);
            Test->TestTrue(TEXT("Chase camera collision enabled"), Car->FindComponentByClass<USpringArmComponent>()->bDoCollisionTest);
        });
        Add(TEXT("Pause inside vehicle"), 0.25, [this] { Key(EKeys::Escape, IE_Pressed); }, [this]
        {
            Key(EKeys::Escape, IE_Released, 0);
            Test->TestTrue(TEXT("Vehicle gameplay pauses"), Player->IsPaused());
            PausedTime = Car->GetWorld()->GetTimeSeconds();
        }, true);
        Add(TEXT("Verify paused clock"), 0.25, [] {}, [this]
        {
            Test->TestTrue(TEXT("Physics game clock remains frozen"), FMath::IsNearlyEqual(PausedTime, Car->GetWorld()->GetTimeSeconds(), 0.01));
        }, true);
        Add(TEXT("Resume driving"), 0.3, [this] { Key(EKeys::Escape, IE_Pressed); }, [this]
        {
            Key(EKeys::Escape, IE_Released, 0);
            Test->TestFalse(TEXT("Escape resumes driving"), Player->IsPaused());
            BlockExits();
        }, true);
        Add(TEXT("Block all safe exits"), 0.3, [] {});
        PressE(TEXT("Reject obstructed exit"), [this]
        {
            Test->TestTrue(TEXT("Blocked exit retains car possession"), Player->GetPawn() == Car.Get());
            Test->TestTrue(TEXT("Blocked exit explains clearance problem"), HasText(TEXT("No clear exit")));
            RemoveBlockers();
        });
        PressE(TEXT("Exit after clearing obstacles"), [this]
        {
            Test->TestTrue(TEXT("Original character regains control"), Player->GetPawn() == Foot.Get());
            Test->TestFalse(TEXT("Character is visible after exit"), Foot->IsHidden());
            Test->TestTrue(TEXT("Character collision restored"), Foot->GetActorEnableCollision());
            Test->TestFalse(TEXT("Vehicle HUD hidden on foot"), HasText(TEXT("km/h")));
        });
        Add(TEXT("Walk after exit"), 0.4, [this]
        {
            Origin = Foot->GetActorLocation();
            Key(EKeys::W, IE_Pressed);
        }, [this]
        {
            Key(EKeys::W, IE_Released, 0);
            Test->TestTrue(TEXT("Walking input returns to character"), FVector::Dist(Origin, Foot->GetActorLocation()) > 30);
            Key(EKeys::LeftShift, IE_Pressed);
            Key(EKeys::SpaceBar, IE_Pressed);
            PeakHeight = Foot->GetActorLocation().Z;
            Origin = Foot->GetActorLocation();
        });
        Add(TEXT("Sprint and jump after exit"), 0.5, [] {}, [this]
        {
            Test->TestTrue(TEXT("On-foot sprint mapping restored"), Foot->GetCharacterMovement()->MaxWalkSpeed > 600);
            Test->TestTrue(TEXT("On-foot jump mapping restored"), PeakHeight > Origin.Z + 40);
            Key(EKeys::LeftShift, IE_Released, 0);
            Key(EKeys::SpaceBar, IE_Released, 0);
        });
        Add(TEXT("Land before re-entry"), 1.0, [] {});
        for (int32 Cycle = 0; Cycle < 3; ++Cycle)
        {
            Add(TEXT("Prepare repeated entry"), 0.4, [this]
            {
                const FRotator Yaw(0, Car->GetActorRotation().Yaw, 0);
                PlaceFoot(Car->GetActorLocation() + Yaw.RotateVector(FVector(0,-190,95)), Yaw.Yaw + 90);
            });
            PressE(TEXT("Repeat entry"), [this] { Test->TestTrue(TEXT("Repeated entry transfers control"), Player->GetPawn() == Car.Get()); });
            PressE(TEXT("Repeat exit"), [this] { Test->TestTrue(TEXT("Repeated exit preserves the character"), Player->GetPawn() == Foot.Get()); });
        }
        Add(TEXT("Prepare minor surface change"), 0.4, [this]
        {
            PlaceFoot(Car->GetActorLocation() + FRotator(0,Car->GetActorRotation().Yaw,0).RotateVector(FVector(0,-190,95)), Car->GetActorRotation().Yaw + 90);
        });
        PressE(TEXT("Enter for road physics checks"), [this] { Test->TestTrue(TEXT("Physics test driver seated"), Player->GetPawn() == Car.Get()); });
        Add(TEXT("Place curb fixture"), 1.0, [this] { ResetCar(FVector(300,3000,80), 0); });
        Add(TEXT("Drive over sidewalk edge"), 1.7, [this] { Key(EKeys::W, IE_Pressed); }, [this]
        {
            Test->TestTrue(TEXT("Car crosses the sidewalk edge"), Car->GetActorLocation().X > 600);
            Test->TestTrue(TEXT("Minor step preserves stable chassis"), Car->GetActorUpVector().Z > 0.8 && Car->GetActorLocation().Z > 0 && Car->GetActorLocation().Z < 150);
            Key(EKeys::W, IE_Released, 0);
            ResetCar(FVector(-800,-2200,80), 180);
        });
        Add(TEXT("Settle building collision fixture"), 1.0, [] {});
        Add(TEXT("Drive into building"), 3.0, [this] { Key(EKeys::W, IE_Pressed); }, [this]
        {
            Key(EKeys::W, IE_Released, 0);
            Test->TestTrue(TEXT("Solid building stops vehicle penetration"), Car->GetActorLocation().X > -1550);
            Test->TestTrue(TEXT("Chassis remains upright after collision"), Car->GetActorUpVector().Z > 0.7);
            Test->TestTrue(TEXT("Impact does not produce runaway velocity"), Car->GetSpeedKmh() < 15);
            Test->AddInfo(FString::Printf(TEXT("Building stop X %.1f cm; speed %.2f km/h"), Car->GetActorLocation().X, Car->GetSpeedKmh()));
        });
    }
    void Cleanup()
    {
        RemoveBlockers();
        if (!Player.IsValid()) { return; }
        Player->ResumeGame();
        Player->FlushPressedKeys();
        if (Car.IsValid())
        {
            ResetCar(OriginalCar.GetLocation(), OriginalCar.Rotator().Yaw);
            Car->GetSeat()->TryExit(Player.Get());
            Car->GetDriverInput()->ResetControls(true);
        }
        if (Foot.IsValid() && Player->GetPawn() == Foot.Get())
        {
            Foot->SetSprinting(false);
            Foot->StopJumping();
            PlaceFoot(OriginalFoot.GetLocation(), 0);
        }
    }

    FAutomationTestBase* Test;
    TWeakObjectPtr<ADelusionPlayerController> Player;
    TWeakObjectPtr<ADelusionCharacter> Foot;
    TWeakObjectPtr<ADelusionVehicle> Car;
    TArray<FStep> Steps;
    TArray<TWeakObjectPtr<AActor>> Blockers;
    FTransform OriginalFoot;
    FTransform OriginalCar;
    FVector Origin = FVector::ZeroVector;
    FVector BrakeOrigin = FVector::ZeroVector;
    double Started;
    double RealStart = 0;
    double PeakHeight = 0;
    double BrakingDistance = 0;
    float GameStart = 0;
    float PausedTime = 0;
    float Heading = 0;
    int32 StepIndex = 0;
    bool bRecordBraking = false;
    bool bStopped = false;
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelusionVehicleSmoke, "Delusion.Sprint1.Vehicle",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FDelusionVehicleSmoke::RunTest(const FString& Parameters)
{
    ADD_LATENT_AUTOMATION_COMMAND(DelusionVehicleSmoke::FVehicleCommand(this));
    return true;
}

#endif
