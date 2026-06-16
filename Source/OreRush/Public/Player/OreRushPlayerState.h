// Ore Rush — oyuncu durumu: takım + kişisel istatistik (ileride).

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Core/OreRushTypes.h"
#include "OreRushPlayerState.generated.h"

/**
 * AOreRushPlayerState
 * Oyuncunun takımını (replicated) tutar. Takım ataması server-only.
 */
UCLASS()
class ORERUSH_API AOreRushPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Ore Rush|Team")
	ETeam GetTeam() const { return Team; }

	/** Server-only: oyuncunun takımını ayarla (GameMode tarafından çağrılır). */
	void SetTeam(ETeam NewTeam);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_Team, BlueprintReadOnly, Category = "Ore Rush|Team")
	ETeam Team = ETeam::None;

	UFUNCTION()
	void OnRep_Team();

	/** UI/kozmetik hook (renk vb.). Mantık değil. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Team")
	void OnTeamChanged();
};
