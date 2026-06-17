#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SmokeCloud.generated.h"

class USceneComponent;

UCLASS()
class ORERUSH_API ASmokeCloud : public AActor
{
	GENERATED_BODY()

public:
	ASmokeCloud();

	void ServerInit(float Duration);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Smoke")
	TObjectPtr<USceneComponent> Root;

	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Smoke")
	void OnSmokeFX();
};
