#include "DelusionVehicleSeatComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DelusionCharacter.h"
#include "DelusionVehicleInputComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "WheeledVehiclePawn.h"

bool UDelusionVehicleSeatComponent::IsOccupied() const
{
    return IsValid(Occupant.Get());
}

float UDelusionVehicleSeatComponent::GetSpeedKmh() const
{
    const AWheeledVehiclePawn* Vehicle = Cast<AWheeledVehiclePawn>(GetOwner());
    return Vehicle ? Vehicle->GetMesh()->GetPhysicsLinearVelocity().Size() * 0.036f : 0.0f;
}

bool UDelusionVehicleSeatComponent::CanEnter(APawn* Interactor) const
{
    const ADelusionCharacter* Pedestrian = Cast<ADelusionCharacter>(Interactor);
    const AWheeledVehiclePawn* Vehicle = Cast<AWheeledVehiclePawn>(GetOwner());
    const APlayerController* Player = Pedestrian ? Cast<APlayerController>(Pedestrian->GetController()) : nullptr;
    if (!Vehicle || !Player || !Player->IsLocalController() || Player->IsPaused() ||
        IsOccupied() || Vehicle->GetController() || !Pedestrian->GetCharacterMovement()->IsMovingOnGround() ||
        GetSpeedKmh() > MaximumEntrySpeedKmh ||
        FVector::DistSquared(Pedestrian->GetActorLocation(), Vehicle->GetActorLocation()) > FMath::Square(EntryDistance))
    {
        return false;
    }
    FCollisionQueryParams Query(SCENE_QUERY_STAT(DelusionVehicleEntry), false, Pedestrian);
    Query.AddIgnoredActor(Vehicle);
    return !GetWorld()->LineTraceTestByChannel(Pedestrian->GetPawnViewLocation(),
        Vehicle->GetActorLocation() + FVector(0, 0, 60), ECC_Visibility, Query);
}

bool UDelusionVehicleSeatComponent::TryEnter(APawn* Interactor)
{
    if (!CanEnter(Interactor)) { return false; }
    AWheeledVehiclePawn* Vehicle = CastChecked<AWheeledVehiclePawn>(GetOwner());
    ADelusionCharacter* Pedestrian = CastChecked<ADelusionCharacter>(Interactor);
    APlayerController* Player = CastChecked<APlayerController>(Pedestrian->GetController());
    Occupant = Pedestrian;
    DriverController = Player;
    OnFootTransform = Pedestrian->GetActorTransform();
    Player->FlushPressedKeys();
    Pedestrian->SetSprinting(false);
    Pedestrian->StopJumping();
    Pedestrian->GetCharacterMovement()->StopMovementImmediately();

    const bool bAutoCamera = Player->bAutoManageActiveCameraTarget;
    Player->bAutoManageActiveCameraTarget = false;
    Player->SetViewTargetWithBlend(Vehicle, 0.3f, VTBlend_Cubic, 0.0f, true);
    Pedestrian->GetCharacterMovement()->DisableMovement();
    Pedestrian->SetActorEnableCollision(false);
    Pedestrian->SetActorHiddenInGame(true);
    Pedestrian->AttachToComponent(Vehicle->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    Pedestrian->SetActorRelativeLocation(FVector(0, -30, 70));
    Player->Possess(Vehicle);
    Player->bAutoManageActiveCameraTarget = bAutoCamera;
    if (Player->GetPawn() != Vehicle)
    {
        Pedestrian->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        Pedestrian->SetActorTransform(OnFootTransform);
        Pedestrian->SetActorEnableCollision(true);
        Pedestrian->SetActorHiddenInGame(false);
        Pedestrian->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        Player->Possess(Pedestrian);
        Player->SetViewTargetWithBlend(Pedestrian, 0.3f);
        Occupant = nullptr;
        DriverController.Reset();
        return false;
    }
    Player->SetControlRotation(FRotator(-12, Vehicle->GetActorRotation().Yaw, 0));
    UE_LOG(LogTemp, Display, TEXT("DELUSION entered vehicle; on-foot pawn preserved"));
    return true;
}

bool UDelusionVehicleSeatComponent::FindSafeExit(FVector& OutLocation) const
{
    if (!IsOccupied() || !GetOwner() || !GetWorld()) { return false; }
    float Radius, HalfHeight;
    Occupant->GetCapsuleComponent()->GetScaledCapsuleSize(Radius, HalfHeight);
    const FVector Origin = GetOwner()->GetActorLocation();
    const FRotator Yaw(0, GetOwner()->GetActorRotation().Yaw, 0);
    const FVector Offsets[] = {
        FVector(0, -190, 0), FVector(0, 190, 0),
        FVector(-300, 0, 0), FVector(300, 0, 0),
        FVector(-180, -220, 0), FVector(-180, 220, 0)
    };
    FCollisionQueryParams GroundQuery(SCENE_QUERY_STAT(DelusionExitGround), false, Occupant.Get());
    GroundQuery.AddIgnoredActor(GetOwner());
    FCollisionQueryParams ClearanceQuery(SCENE_QUERY_STAT(DelusionExitClearance), false, Occupant.Get());
    for (const FVector& Offset : Offsets)
    {
        const FVector Candidate = Origin + Yaw.RotateVector(Offset);
        FHitResult Floor;
        if (!GetWorld()->LineTraceSingleByChannel(Floor, Candidate + FVector(0, 0, 120),
            Candidate - FVector(0, 0, 220), ECC_Visibility, GroundQuery) ||
            Floor.ImpactNormal.Z < Occupant->GetCharacterMovement()->GetWalkableFloorZ())
        {
            continue;
        }
        const FVector Location = Floor.ImpactPoint + FVector(0, 0, HalfHeight + 4);
        if (GetWorld()->OverlapBlockingTestByChannel(Location, FQuat::Identity, ECC_Pawn,
            FCollisionShape::MakeCapsule(Radius, HalfHeight), ClearanceQuery))
        {
            continue;
        }
        // Reject a position reached through a wall even when its final capsule is clear.
        if (GetWorld()->LineTraceTestByChannel(Origin + FVector(0, 0, HalfHeight),
            Location, ECC_Visibility, GroundQuery))
        {
            continue;
        }
        OutLocation = Location;
        return true;
    }
    return false;
}

bool UDelusionVehicleSeatComponent::TryExit(APlayerController* Player)
{
    AWheeledVehiclePawn* Vehicle = Cast<AWheeledVehiclePawn>(GetOwner());
    FVector ExitLocation;
    if (!IsOccupied() || !Player || DriverController.Get() != Player || !Vehicle ||
        Player->GetPawn() != Vehicle || Player->IsPaused() ||
        GetSpeedKmh() > MaximumExitSpeedKmh || !FindSafeExit(ExitLocation))
    {
        return false;
    }
    ADelusionCharacter* Pedestrian = Occupant;
    Player->FlushPressedKeys();
    if (UDelusionVehicleInputComponent* Input = Vehicle->FindComponentByClass<UDelusionVehicleInputComponent>())
    {
        Input->ResetControls(true);
    }
    Pedestrian->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    Pedestrian->SetActorLocationAndRotation(ExitLocation, FRotator(0, Vehicle->GetActorRotation().Yaw, 0),
        false, nullptr, ETeleportType::TeleportPhysics);
    const bool bAutoCamera = Player->bAutoManageActiveCameraTarget;
    Player->bAutoManageActiveCameraTarget = false;
    Player->Possess(Pedestrian);
    Player->bAutoManageActiveCameraTarget = bAutoCamera;
    if (Player->GetPawn() != Pedestrian)
    {
        Pedestrian->AttachToComponent(Vehicle->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        Pedestrian->SetActorRelativeLocation(FVector(0, -30, 70));
        Player->Possess(Vehicle);
        return false;
    }
    Pedestrian->SetActorHiddenInGame(false);
    Pedestrian->SetActorEnableCollision(true);
    Pedestrian->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    Player->SetControlRotation(FRotator(-10, Vehicle->GetActorRotation().Yaw, 0));
    Player->SetViewTargetWithBlend(Pedestrian, 0.3f, VTBlend_Cubic, 0.0f, true);
    Occupant = nullptr;
    DriverController.Reset();
    UE_LOG(LogTemp, Display, TEXT("DELUSION exited vehicle at %s"), *ExitLocation.ToCompactString());
    return true;
}

void UDelusionVehicleSeatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Ordinary exits always use the checks above. Owner removal is not vehicle destruction gameplay.
    if (EndPlayReason == EEndPlayReason::Destroyed && IsOccupied() && DriverController.IsValid())
    {
        FVector SafeLocation;
        if (!FindSafeExit(SafeLocation)) { SafeLocation = OnFootTransform.GetLocation(); }
        float Radius, HalfHeight;
        Occupant->GetCapsuleComponent()->GetScaledCapsuleSize(Radius, HalfHeight);
        FCollisionQueryParams Query(SCENE_QUERY_STAT(DelusionSeatRemoval), false, Occupant.Get());
        Query.AddIgnoredActor(GetOwner());
        if (!GetWorld()->OverlapBlockingTestByChannel(SafeLocation, FQuat::Identity, ECC_Pawn,
            FCollisionShape::MakeCapsule(Radius, HalfHeight), Query))
        {
            Occupant->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
            Occupant->SetActorLocation(SafeLocation);
            Occupant->SetActorHiddenInGame(false);
            Occupant->SetActorEnableCollision(true);
            Occupant->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
            DriverController->Possess(Occupant);
        }
    }
    Super::EndPlay(EndPlayReason);
}