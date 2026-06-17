#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuildComponent.generated.h"

class ATrapBase;

USTRUCT(BlueprintType)
struct FTrapCatalogEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Build")
	TSubclassOf<ATrapBase> TrapClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Build", meta = (ClampMin = "0"))
	int32 Cost = 3;
};

UCLASS(ClassGroup = (OreRush), meta = (BlueprintSpawnableComponent))
class ORERUSH_API UBuildComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBuildComponent();

	void ServerTryPlace(int32 Index);

	void ServerCycleSelection();

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Build")
	int32 GetSelectedIndex() const { return SelectedIndex; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Build")
	TArray<FTrapCatalogEntry> Catalog;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Build", meta = (ClampMin = "1"))
	int32 ActiveTrapLimit = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Build", meta = (ClampMin = "0.0"))
	float PlaceCooldown = 2.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Build", meta = (ClampMin = "0.0"))
	float PlaceDistance = 150.f;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_Selected();

private:
	void PrunePlaced();

	TArray<TWeakObjectPtr<ATrapBase>> PlacedTraps;
	float LastPlaceTime = -1000.f;

	UPROPERTY(ReplicatedUsing = OnRep_Selected)
	int32 SelectedIndex = 0;
};
