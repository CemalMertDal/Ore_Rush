// Ore Rush — maç akışı (server-only): team ata, üretim, kota/kazanan kontrolü.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Core/OreRushTypes.h"
#include "OreRushGameMode.generated.h"

class ADepotZone;

/**
 * AOreRushGameMode
 * Sunucu-otorite maç yönetimi. Şimdilik default sınıfları (pawn/controller/state) bağlar.
 * Step 4: team ataması, kota kontrolü ve kazanan ilanı buraya gelecek.
 */
UCLASS()
class ORERUSH_API AOreRushGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AOreRushGameMode();

	virtual void InitGameState() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	void CheckWinCondition();

	/** Oyuncuları kendi takım depolarına taşır; depo/pawn hazır değilse kısa retry ile garanti eder. */
	void EnsurePlayersAtDepots();

	/** Maçı aynı haritada yeniden başlat (yeni seed). Listen server'ı korur. */
	void RestartMatch();

protected:
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;

	UPROPERTY(EditDefaultsOnly, Category = "Ore Rush|Match")
	int32 QuotaTarget = 50;

	ETeam PickTeamForNewPlayer();

	ADepotZone* FindDepotForTeam(ETeam Team) const;

private:
	void RunPlacementPass();

	TSet<TWeakObjectPtr<AController>> PlacedControllers;
	FTimerHandle PlacementRetryTimer;
	int32 PlacementAttempts = 0;
};
