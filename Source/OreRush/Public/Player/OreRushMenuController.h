#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OreRushMenuController.generated.h"

class UUserWidget;

UCLASS()
class ORERUSH_API AOreRushMenuController : public APlayerController
{
	GENERATED_BODY()

public:
	AOreRushMenuController();

protected:
	virtual void BeginPlay() override;

	/** Menü ekranı (BP'de WBP_MainMenu atanır). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Menu")
	TSubclassOf<UUserWidget> MenuWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Ore Rush|Menu")
	TObjectPtr<UUserWidget> MenuWidget;
};
