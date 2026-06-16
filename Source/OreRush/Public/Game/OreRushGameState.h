// Ore Rush — maç durumu: takım kasaları (skor), kota, harita seed'i.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "OreRushGameState.generated.h"

/**
 * AOreRushGameState
 * Takım kasaları = skor (replicated, RepNotify → HUD). Kota hedefi ve harita seed'i
 * tüm client'larda senkron. Kazanan kontrolü GameMode'da (server), sonuç buradan okunur.
 */
UCLASS()
class ORERUSH_API AOreRushGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	/** Kırmızı takım kasası (skor). */
	UPROPERTY(ReplicatedUsing = OnRep_Scores, BlueprintReadOnly, Category = "Ore Rush|Score")
	int32 RedScore = 0;

	/** Mavi takım kasası (skor). */
	UPROPERTY(ReplicatedUsing = OnRep_Scores, BlueprintReadOnly, Category = "Ore Rush|Score")
	int32 BlueScore = 0;

	/** Kazanmak için gereken kota (örn. 50). */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Ore Rush|Score")
	int32 QuotaTarget = 50;

	/** Prosedürel harita seed'i (deterministik üretim için senkron). */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Ore Rush|Map")
	int32 MapSeed = 0;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_Scores();

	/** HUD hook (skor değişti). Kozmetik/sunum. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Score")
	void OnScoresChanged();
};
