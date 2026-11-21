#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DelusionInteractionComponent.generated.h"

class APawn;

UCLASS(ClassGroup=(Delusion), meta=(BlueprintSpawnableComponent))
class DELUSION_API UDelusionInteractionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDelusionInteractionComponent();
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category="Interaction")
    bool TryInteract();

    UFUNCTION(BlueprintPure, Category="Interaction")
    FText GetPrompt() const;

    UFUNCTION(BlueprintPure, Category="Interaction")
    AActor* GetFocusedTarget() const { return FocusedTarget.Get(); }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction", meta=(ClampMin="1", ClampMax="1000"))
    float MaxInteractionDistance = 260.0f;

private:
    APawn* GetInteractor() const;
    bool IsValidTarget(AActor* Target, APawn* Interactor) const;
    void RefreshTarget();
    TWeakObjectPtr<AActor> FocusedTarget;
};
