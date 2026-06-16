// Ore Rush — maç durumu (implementation).

#include "Game/OreRushGameState.h"
#include "Net/UnrealNetwork.h"

void AOreRushGameState::OnRep_Scores()
{
	OnScoresChanged();
}

void AOreRushGameState::AddScore(ETeam Team, int32 Amount)
{
	if (!HasAuthority() || Amount <= 0)
	{
		return;
	}

	if (Team == ETeam::Red)
	{
		RedScore += Amount;
	}
	else if (Team == ETeam::Blue)
	{
		BlueScore += Amount;
	}

	OnRep_Scores(); // Sunucuda da tetikle (yerel HUD/efekt için)
}

void AOreRushGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOreRushGameState, RedScore);
	DOREPLIFETIME(AOreRushGameState, BlueScore);
	DOREPLIFETIME(AOreRushGameState, QuotaTarget);
	DOREPLIFETIME(AOreRushGameState, MapSeed);
}
