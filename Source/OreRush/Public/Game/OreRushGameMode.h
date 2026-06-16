// Ore Rush — maç akışı (server-only): team ata, üretim, kota/kazanan kontrolü.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Core/OreRushTypes.h"
#include "OreRushGameMode.generated.h"

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

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Ore Rush|Match")
	int32 QuotaTarget = 20;

	ETeam PickTeamForNewPlayer();
};
