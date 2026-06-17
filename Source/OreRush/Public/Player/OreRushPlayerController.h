// Ore Rush — oyuncu controller'ı: input + UI sahipliği (HUD/menü/end-screen, step 9).

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OreRushPlayerController.generated.h"

/**
 * AOreRushPlayerController
 * Şimdilik iskelet. Enhanced Input mapping context karakterde ekleniyor (standart pattern).
 * UMG sahipliği (MainMenu/HUD/Tutorial/EndScreen) step 9'da buraya gelecek.
 */
UCLASS()
class ORERUSH_API AOreRushPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(Client, Reliable)
	void ClientShowAlarm(const FString& Message);

	/** Menüden seçilen kozmetik rengi sunucuya bildir. */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Ore Rush|Cosmetic")
	void ServerSetColorIndex(uint8 NewIndex);

	/** EndScreen "tekrar oyna" → sunucudan maçı yeniden başlat. */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Ore Rush|Match")
	void ServerRequestRestart();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Alarm")
	void OnAlarm(const FString& Message);
};
