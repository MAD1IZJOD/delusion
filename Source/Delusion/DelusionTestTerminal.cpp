#include "DelusionTestTerminal.h"

#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Pawn.h"
#include "UObject/ConstructorHelpers.h"

ADelusionTestTerminal::ADelusionTestTerminal()
{
    PrimaryActorTick.bCanEverTick = false;
    TerminalBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TerminalBody"));
    RootComponent = TerminalBody;
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    TerminalBody->SetStaticMesh(Cube.Object);
    TerminalBody->SetRelativeScale3D(FVector(0.7f, 0.7f, 1.8f));
    TerminalBody->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    StatusLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StatusLabel"));
    StatusLabel->SetupAttachment(RootComponent);
    StatusLabel->SetRelativeLocation(FVector(-51, 0, 12));
    StatusLabel->SetRelativeRotation(FRotator(0, 180, 0));
    StatusLabel->SetHorizontalAlignment(EHTA_Center);
    StatusLabel->SetWorldSize(15.0f);
    StatusLabel->SetTextRenderColor(FColor::Cyan);
    StatusLabel->SetText(NSLOCTEXT("Delusion", "TerminalOffline", "TERMINAL\nOFFLINE"));
    StatusLabel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

bool ADelusionTestTerminal::CanInteract_Implementation(APawn* Interactor) const
{
    return IsValid(Interactor) && FVector::DistSquared(Interactor->GetActorLocation(), GetActorLocation()) <= FMath::Square(260.0f);
}

FText ADelusionTestTerminal::GetInteractionPrompt_Implementation() const
{
    return bActivated
        ? NSLOCTEXT("Delusion", "ResetTerminal", "[E] Reset terminal")
        : NSLOCTEXT("Delusion", "ActivateTerminal", "[E] Activate terminal");
}

bool ADelusionTestTerminal::Interact_Implementation(APawn* Interactor)
{
    if (!CanInteract_Implementation(Interactor)) { return false; }
    bActivated = !bActivated;
    StatusLabel->SetText(bActivated
        ? NSLOCTEXT("Delusion", "TerminalOnline", "TERMINAL\nONLINE")
        : NSLOCTEXT("Delusion", "TerminalOffline", "TERMINAL\nOFFLINE"));
    StatusLabel->SetTextRenderColor(bActivated ? FColor::Green : FColor::Cyan);
    UE_LOG(LogTemp, Display, TEXT("DELUSION terminal %s"), bActivated ? TEXT("activated") : TEXT("reset"));
    return true;
}
