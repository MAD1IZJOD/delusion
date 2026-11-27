#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "ChaosWheeledVehicleMovementComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/AutomationTest.h"
#include "../DelusionCharacter.h"
#include "../DelusionPlayerController.h"
#include "../DelusionVehicle.h"

namespace DelusionDriveBenchmark
{
class FDriveCommand final : public IAutomationLatentCommand
{
public:
    explicit FDriveCommand(FAutomationTestBase* InTest) : Test(InTest), Started(FPlatformTime::Seconds()) {}

    virtual bool Update() override
    {
        if (FPlatformTime::Seconds() - Started > 180.0)
        {
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
                for (TActorIterator<ADelusionVehicle> It(World); It; ++It) { Car = *It; break; }
                if (Player.IsValid() && Car.IsValid()) { break; }
                Player.Reset();
            }
            if (!Player.IsValid()) { return false; }

            // Place the vehicle in a large open area so it can drive continuously.
            Car->GetDrivingMovement()->ResetVehicle();
            Car->GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);
            Car->GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
            Car->SetActorLocationAndRotation(FVector(-4000.0f, 4000.0f, 100.0f), FRotator::ZeroRotator, false, nullptr, ETeleportType::TeleportPhysics);

            const FRotator Yaw(0.0f, Car->GetActorRotation().Yaw, 0.0f);
            const FVector FootLocation = Car->GetActorLocation() + Yaw.RotateVector(FVector(0.0f, -190.0f, 0.0f)) + FVector(0.0f, 0.0f, 92.0f);
            if (APawn* Pawn = Player->GetPawn())
            {
                Pawn->SetActorLocation(FootLocation, false, nullptr, ETeleportType::TeleportPhysics);
                Player->SetControlRotation(FRotator(-10.0f, Yaw.Yaw + 90.0f, 0.0f));
            }

            Player->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::E, IE_Pressed, 1.0f));
            EntryTime = FPlatformTime::Seconds();
            return false;
        }

        if (!bEntered && FPlatformTime::Seconds() - EntryTime > 0.6)
        {
            Player->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::E, IE_Released, 0.0f));
            Player->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::W, IE_Pressed, 1.0f));
            Player->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::D, IE_Pressed, 1.0f));
            bEntered = true;
            DriveStart = FPlatformTime::Seconds();
            return false;
        }

        if (bEntered && FPlatformTime::Seconds() - DriveStart > 90.0)
        {
            Player->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::W, IE_Released, 0.0f));
            Player->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::D, IE_Released, 0.0f));
            Cleanup();
            return true;
        }

        return false;
    }

private:
    void Cleanup() {}

    FAutomationTestBase* Test;
    TWeakObjectPtr<ADelusionPlayerController> Player;
    TWeakObjectPtr<ADelusionVehicle> Car;
    double Started = 0.0;
    double EntryTime = 0.0;
    double DriveStart = 0.0;
    bool bEntered = false;
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelusionDriveBenchmark, "Perf.DelusionDrive",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FDelusionDriveBenchmark::RunTest(const FString& Parameters)
{
    ADD_LATENT_AUTOMATION_COMMAND(DelusionDriveBenchmark::FDriveCommand(this));
    return true;
}

#endif