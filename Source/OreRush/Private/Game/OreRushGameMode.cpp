// Ore Rush — maç akışı (implementation).

#include "Game/OreRushGameMode.h"
#include "Character/OreRushCharacter.h"
#include "Game/OreRushGameState.h"
#include "Player/OreRushPlayerController.h"
#include "Player/OreRushPlayerState.h"
#include "GameFramework/PlayerController.h"

AOreRushGameMode::AOreRushGameMode()
{
	// C++ çekirdek sınıfları varsayılan olarak bağla. BP_OreRushGameMode bu sınıftan
	// türeyip pawn/controller'ı kozmetik BP versiyonlarıyla override edebilir.
	DefaultPawnClass = AOreRushCharacter::StaticClass();
	PlayerControllerClass = AOreRushPlayerController::StaticClass();
	PlayerStateClass = AOreRushPlayerState::StaticClass();
	GameStateClass = AOreRushGameState::StaticClass();
}

void AOreRushGameMode::InitGameState()
{
	Super::InitGameState();

	if (AOreRushGameState* GS = Cast<AOreRushGameState>(GameState))
	{
		GS->QuotaTarget = QuotaTarget;
	}
}

void AOreRushGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!NewPlayer)
	{
		return;
	}

	if (AOreRushPlayerState* PS = NewPlayer->GetPlayerState<AOreRushPlayerState>())
	{
		const ETeam Assigned = PickTeamForNewPlayer();
		PS->SetTeam(Assigned);

		UE_LOG(LogTemp, Log, TEXT("[OreRush] Team assigned: %s -> %s"),
			*PS->GetPlayerName(),
			(Assigned == ETeam::Red ? TEXT("Red") : TEXT("Blue")));
	}
}

ETeam AOreRushGameMode::PickTeamForNewPlayer()
{
	return (GetNumPlayers() <= 1) ? ETeam::Red : ETeam::Blue;
}

void AOreRushGameMode::CheckWinCondition()
{
	AOreRushGameState* GS = Cast<AOreRushGameState>(GameState);
	if (!GS || GS->IsMatchEnded())
	{
		return;
	}

	const int32 Quota = GS->QuotaTarget;
	if (Quota <= 0)
	{
		return;
	}

	if (GS->RedScore >= Quota)
	{
		GS->EndMatch(ETeam::Red);
	}
	else if (GS->BlueScore >= Quota)
	{
		GS->EndMatch(ETeam::Blue);
	}
}
