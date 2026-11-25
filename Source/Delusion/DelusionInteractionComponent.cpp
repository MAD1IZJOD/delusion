#include "DelusionInteractionComponent.h"

#include "DelusionInteractable.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"

UDelusionInteractionComponent::UDelusionInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
}

APawn* UDelusionInteractionComponent::GetInteractor() const
{
    if (APawn* PawnOwner = Cast<APawn>(GetOwner())) { return PawnOwner; }
    const AController* ControllerOwner = Cast<AController>(GetOwner());
    return ControllerOwner ? ControllerOwner->GetPawn() : nullptr;
}

bool UDelusionInteractionComponent::IsValidTarget(AActor* Target, APawn* Interactor) const
{
    if (!IsValid(Target) || !IsValid(Interactor) || !GetWorld() ||
        !Target->Implements<UDelusionInteractable>() ||
        FVector::DistSquared(Target->GetActorLocation(), Interactor->GetActorLocation()) > FMath::Square(MaxInteractionDistance) ||
        !IDelusionInteractable::Execute_CanInteract(Target, Interactor))
    {
        return false;
    }
    FCollisionQueryParams Query(SCENE_QUERY_STAT(DelusionInteractionVisibility), false, Interactor);
    Query.AddIgnoredActor(Target);
    return !GetWorld()->LineTraceTestByChannel(Interactor->GetPawnViewLocation(), Target->GetActorLocation(), ECC_Visibility, Query);
}

void UDelusionInteractionComponent::RefreshTarget()
{
    FocusedTarget.Reset();
    APawn* Interactor = GetInteractor();
    if (!Interactor || !GetWorld()) { return; }
    TArray<FOverlapResult> Nearby;
    FCollisionObjectQueryParams ObjectTypes;
    ObjectTypes.AddObjectTypesToQuery(ECC_WorldStatic);
    ObjectTypes.AddObjectTypesToQuery(ECC_WorldDynamic);
    ObjectTypes.AddObjectTypesToQuery(ECC_Vehicle);
    FCollisionQueryParams Query(SCENE_QUERY_STAT(DelusionInteractionOverlap), false, Interactor);
    GetWorld()->OverlapMultiByObjectType(Nearby, Interactor->GetActorLocation(), FQuat::Identity,
        ObjectTypes, FCollisionShape::MakeSphere(MaxInteractionDistance), Query);
    FVector Forward = Interactor->GetControlRotation().Vector();
    Forward.Z = 0;
    Forward.Normalize();
    double BestDistance = FMath::Square(static_cast<double>(MaxInteractionDistance));
    for (const FOverlapResult& Overlap : Nearby)
    {
        AActor* Candidate = Overlap.GetActor();
        if (!IsValidTarget(Candidate, Interactor)) { continue; }
        FVector Offset = Candidate->GetActorLocation() - Interactor->GetActorLocation();
        const double Distance = Offset.SizeSquared();
        Offset.Z = 0;
        if (FVector::DotProduct(Forward, Offset.GetSafeNormal()) >= 0.1 && Distance <= BestDistance)
        {
            BestDistance = Distance;
            FocusedTarget = Candidate;
        }
    }
}

void UDelusionInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    RefreshTarget();
}

FText UDelusionInteractionComponent::GetPrompt() const
{
    AActor* Target = FocusedTarget.Get();
    return IsValidTarget(Target, GetInteractor())
        ? IDelusionInteractable::Execute_GetInteractionPrompt(Target) : FText::GetEmpty();
}

bool UDelusionInteractionComponent::TryInteract()
{
    // Refresh on the actual key press and recheck range and occlusion.
    RefreshTarget();
    AActor* Target = FocusedTarget.Get();
    APawn* Interactor = GetInteractor();
    return IsValidTarget(Target, Interactor) && IDelusionInteractable::Execute_Interact(Target, Interactor);
}
