// Ore Rush — maç durumu (implementation).

#include "Game/OreRushGameState.h"
#include "Net/UnrealNetwork.h"

void AOreRushGameState::OnRep_Scores()
{
	OnScoresChanged();
}

void AOreRushGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOreRushGameState, RedScore);
	DOREPLIFETIME(AOreRushGameState, BlueScore);
	DOREPLIFETIME(AOreRushGameState, QuotaTarget);
	DOREPLIFETIME(AOreRushGameState, MapSeed);
}
