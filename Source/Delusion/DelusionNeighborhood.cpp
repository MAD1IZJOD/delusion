#include "DelusionNeighborhood.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/TextureCube.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ADelusionNeighborhood::ADelusionNeighborhood()
{
    PrimaryActorTick.bCanEverTick = false;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DistrictRoot"));
    RootComponent->SetMobility(EComponentMobility::Static);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    auto AddBox = [this](const FString& PartName, FVector Center, FVector Size, FName Surface)
    {
        UStaticMeshComponent* Box = CreateDefaultSubobject<UStaticMeshComponent>(FName(*PartName));
        Box->SetupAttachment(RootComponent);
        Box->SetMobility(EComponentMobility::Static);
        Box->SetStaticMesh(Cube.Object);
        Box->SetRelativeLocation(Center);
        Box->SetRelativeScale3D(Size / 100.0f);
        Box->SetCollisionProfileName(TEXT("BlockAll"));
        Box->ComponentTags.Add(Surface);
        if (Surface == TEXT("Marking") || Surface == TEXT("Spawn"))
        {
            Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Box->SetCastShadow(false);
        }
    };

    AddBox(TEXT("Ground"), FVector(0, 0, -50), FVector(14000, 14000, 100), TEXT("Ground"));
    AddBox(TEXT("EastWestRoad"), FVector(0, 0, 1), FVector(14000, 1000, 2), TEXT("Road"));
    AddBox(TEXT("NorthSouthRoad"), FVector(0, 0, 2), FVector(1000, 14000, 2), TEXT("Road"));
    for (int32 Side = -1; Side <= 1; Side += 2)
    {
        for (int32 Half = -1; Half <= 1; Half += 2)
        {
            AddBox(FString::Printf(TEXT("WalkX_%d_%d"), Side, Half), FVector(Half * 3800, Side * 650, 8), FVector(6400, 300, 16), TEXT("Sidewalk"));
            AddBox(FString::Printf(TEXT("WalkY_%d_%d"), Side, Half), FVector(Side * 650, Half * 3800, 8), FVector(300, 6400, 16), TEXT("Sidewalk"));
        }
    }
    for (int32 Lane = -6; Lane <= 6; ++Lane)
    {
        if (Lane == 0) { continue; }
        AddBox(FString::Printf(TEXT("LaneX_%d"), Lane), FVector(Lane * 1000, 0, 4), FVector(360, 12, 1), TEXT("Marking"));
        AddBox(FString::Printf(TEXT("LaneY_%d"), Lane), FVector(0, Lane * 1000, 4), FVector(12, 360, 1), TEXT("Marking"));
    }
    for (int32 Stripe = 0; Stripe < 7; ++Stripe)
    {
        AddBox(FString::Printf(TEXT("Crosswalk_%d"), Stripe), FVector(-900, -360 + Stripe * 120, 4), FVector(260, 55, 1), TEXT("Marking"));
    }

    // Explicit building lots leave the northeast plaza open for exploration.
    const FVector Lots[] = {
        FVector(-2300, -2300, 900), FVector(-5000, -2500, 1400), FVector(-2500, -5100, 650),
        FVector(2400, -2300, 1100), FVector(5000, -2400, 700), FVector(2800, -5100, 1600),
        FVector(-2400, 2300, 1500), FVector(-5000, 2500, 800), FVector(-2700, 5200, 1000),
        FVector(5000, 2600, 1200), FVector(2600, 5300, 850)
    };
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(Lots); ++Index)
    {
        const FVector& Lot = Lots[Index];
        const FVector Size(1500 + (Index % 3) * 180, 1400 + (Index % 2) * 300, Lot.Z);
        AddBox(FString::Printf(TEXT("Building_%02d"), Index), FVector(Lot.X, Lot.Y, Lot.Z / 2), Size, TEXT("Building"));
        AddBox(FString::Printf(TEXT("Roof_%02d"), Index), FVector(Lot.X, Lot.Y, Lot.Z + 25), FVector(Size.X + 70, Size.Y + 70, 50), TEXT("Sidewalk"));
        AddBox(FString::Printf(TEXT("Facade_%02d"), Index), FVector(Lot.X, Lot.Y - Size.Y / 2 - 3, Lot.Z * 0.6f), FVector(Size.X * 0.75f, 6, 130), TEXT("Road"));
    }
    AddBox(TEXT("CentralPlaza"), FVector(2300, 2200, 6), FVector(2500, 2400, 12), TEXT("Sidewalk"));
    AddBox(TEXT("PlazaMonument"), FVector(2300, 2300, 140), FVector(150, 150, 280), TEXT("Spawn"));
    AddBox(TEXT("BenchSeat"), FVector(1500, 1900, 45), FVector(60, 220, 90), TEXT("Barrier"));
    AddBox(TEXT("LowObstacle"), FVector(-1700, 1000, 40), FVector(180, 320, 80), TEXT("Barrier"));
    AddBox(TEXT("TallObstacle"), FVector(-2300, 900, 100), FVector(160, 160, 200), TEXT("Barrier"));
    AddBox(TEXT("SpawnPad"), FVector(-1800, -300, 4), FVector(220, 220, 1), TEXT("Spawn"));
    AddBox(TEXT("NorthBoundary"), FVector(0, 6950, 150), FVector(14000, 100, 300), TEXT("Barrier"));
    AddBox(TEXT("SouthBoundary"), FVector(0, -6950, 150), FVector(14000, 100, 300), TEXT("Barrier"));
    AddBox(TEXT("EastBoundary"), FVector(6950, 0, 150), FVector(100, 14000, 300), TEXT("Barrier"));
    AddBox(TEXT("WestBoundary"), FVector(-6950, 0, 150), FVector(100, 14000, 300), TEXT("Barrier"));

    UDirectionalLightComponent* Sun = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Sun"));
    Sun->SetupAttachment(RootComponent);
    Sun->SetMobility(EComponentMobility::Movable);
    Sun->SetRelativeRotation(FRotator(-45, -35, 0));
    Sun->SetIntensity(4.0f);
    Sun->SetLightColor(FLinearColor(1.0f, 0.94f, 0.84f));
    Sun->SetAtmosphereSunLight(true);
    Sun->DynamicShadowDistanceMovableLight = 6000.0f;

    USkyAtmosphereComponent* Atmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("Sky"));
    Atmosphere->SetupAttachment(RootComponent);
    USkyLightComponent* SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("AmbientLight"));
    SkyLight->SetupAttachment(RootComponent);
    SkyLight->SetMobility(EComponentMobility::Movable);
    SkyLight->SourceType = SLS_SpecifiedCubemap;
    SkyLight->bRealTimeCapture = false;
    static ConstructorHelpers::FObjectFinder<UTextureCube> SkyCube(TEXT("/Engine/EngineMaterials/DefaultCubemap.DefaultCubemap"));
    SkyLight->SetCubemap(SkyCube.Object);
    SkyLight->SetIntensity(0.8f);
}

void ADelusionNeighborhood::ApplySurfaceMaterials()
{
    if (!SurfaceMaterial) { return; }
    TInlineComponentArray<UStaticMeshComponent*> Boxes(this);
    for (UStaticMeshComponent* Box : Boxes)
    {
        FLinearColor Color(0.32f, 0.38f, 0.45f);
        if (Box->ComponentHasTag(TEXT("Ground"))) { Color = FLinearColor(0.12f, 0.17f, 0.14f); }
        else if (Box->ComponentHasTag(TEXT("Road"))) { Color = FLinearColor(0.055f, 0.065f, 0.08f); }
        else if (Box->ComponentHasTag(TEXT("Sidewalk"))) { Color = FLinearColor(0.42f, 0.44f, 0.47f); }
        else if (Box->ComponentHasTag(TEXT("Marking"))) { Color = FLinearColor(0.95f, 0.8f, 0.25f); }
        else if (Box->ComponentHasTag(TEXT("Spawn"))) { Color = FLinearColor(0.04f, 0.6f, 0.85f); }
        else if (Box->ComponentHasTag(TEXT("Barrier"))) { Color = FLinearColor(0.36f, 0.29f, 0.22f); }
        UMaterialInstanceDynamic* Material = Box->CreateDynamicMaterialInstance(0, SurfaceMaterial);
        if (Material) { Material->SetVectorParameterValue(TEXT("Color"), Color); }
    }
}

void ADelusionNeighborhood::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    ApplySurfaceMaterials();
}

void ADelusionNeighborhood::BeginPlay()
{
    Super::BeginPlay();
    ApplySurfaceMaterials();
}
