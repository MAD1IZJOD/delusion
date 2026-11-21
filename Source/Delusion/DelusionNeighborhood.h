#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DelusionNeighborhood.generated.h"

class UMaterialInterface;

/** A compact, fixed-layout greybox district built from simple colliding geometry. */
UCLASS(Blueprintable)
class DELUSION_API ADelusionNeighborhood : public AActor
{
    GENERATED_BODY()

public:
    ADelusionNeighborhood();
    virtual void OnConstruction(const FTransform& Transform) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Appearance")
    TObjectPtr<UMaterialInterface> SurfaceMaterial;

protected:
    virtual void BeginPlay() override;

private:
    void ApplySurfaceMaterials();
};
