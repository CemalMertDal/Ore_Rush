#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/OreRushTypes.h"
#include "MapGenerator.generated.h"

class AOreVein;
class ADepotZone;
class APowerUpBase;
class UBoxComponent;

UCLASS()
class ORERUSH_API AMapGenerator : public AActor
{
	GENERATED_BODY()

public:
	AMapGenerator();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map|Area")
	TObjectPtr<UBoxComponent> AreaViz;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Classes")
	TSubclassOf<AOreVein> VeinClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Classes")
	TSubclassOf<ADepotZone> DepotClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Classes")
	TArray<TSubclassOf<APowerUpBase>> PowerUpClasses;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Classes")
	TArray<TSubclassOf<AActor>> DecorClasses;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Counts", meta = (ClampMin = "0"))
	int32 IronCount = 12;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Counts", meta = (ClampMin = "0"))
	int32 GoldCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Counts", meta = (ClampMin = "0"))
	int32 DiamondCount = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Counts", meta = (ClampMin = "1"))
	int32 GoldUnits = 6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Counts", meta = (ClampMin = "1"))
	int32 DiamondUnits = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Counts", meta = (ClampMin = "0"))
	int32 PowerUpCount = 4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Counts", meta = (ClampMin = "0"))
	int32 DecorCount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Counts")
	bool bScaleWithArea = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Counts")
	FVector2D ReferenceArea = FVector2D(2500.f, 2500.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Area")
	FVector2D AreaExtent = FVector2D(2500.f, 2500.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Area", meta = (ClampMin = "0.0"))
	float MinDistance = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Area")
	float SpawnHeight = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Area")
	float DepotEdgeOffset = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Seed")
	bool bOverrideSeed = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Seed")
	int32 SeedOverride = 12345;

private:
	void Generate(int32 Seed);

	AOreVein* SpawnVein(EOreType Type, bool bUnlimited, int32 Units, const FVector& Location);

	int32 ScaledCount(int32 Base) const;

	bool PickPoint(FRandomStream& Rng, FVector& OutPoint);

	TArray<FVector> UsedPoints;
};
