// Ore Rush — oyuncu durumu (implementation).

#include "Player/OreRushPlayerState.h"
#include "Net/UnrealNetwork.h"

void AOreRushPlayerState::SetTeam(ETeam NewTeam)
{
	if (!HasAuthority() || Team == NewTeam)
	{
		return;
	}

	Team = NewTeam;
	OnRep_Team(); // RepNotify client'larda; sunucuda elle tetikle.
}

void AOreRushPlayerState::OnRep_Team()
{
	OnTeamChanged();
}

void AOreRushPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOreRushPlayerState, Team);
}
