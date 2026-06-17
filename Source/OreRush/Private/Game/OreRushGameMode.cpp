// Ore Rush — maç akışı (implementation).

#include "Game/OreRushGameMode.h"
#include "Character/OreRushCharacter.h"
#include "Game/OreRushGameState.h"
#include "Game/DepotZone.h"
#include "Player/OreRushPlayerController.h"
#include "Player/OreRushPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "EngineUtils.h"

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

		FMath::RandInit(static_cast<int32>(FPlatformTime::Cycles()));
		GS->MapSeed = FMath::RandRange(1, MAX_int32 - 1);
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

		PlacePlayerAtDepot(NewPlayer);
	}
}

ETeam AOreRushGameMode::PickTeamForNewPlayer()
{
	return (GetNumPlayers() <= 1) ? ETeam::Red : ETeam::Blue;
}

ADepotZone* AOreRushGameMode::FindDepotForTeam(ETeam Team) const
{
	if (Team == ETeam::None)
	{
		return nullptr;
	}

	for (TActorIterator<ADepotZone> It(GetWorld()); It; ++It)
	{
		if (It->Team == Team)
		{
			return *It;
		}
	}
	return nullptr;
}

AActor* AOreRushGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	if (Player)
	{
		if (const AOreRushPlayerState* PS = Player->GetPlayerState<AOreRushPlayerState>())
		{
			if (ADepotZone* Depot = FindDepotForTeam(PS->GetTeam()))
			{
				return Depot;
			}
		}
	}
	return Super::ChoosePlayerStart_Implementation(Player);
}

void AOreRushGameMode::PlacePlayerAtDepot(APlayerController* PC)
{
	if (PC == nullptr)
	{
		return;
	}

	APawn* Pawn = PC->GetPawn();
	if (Pawn == nullptr)
	{
		return;
	}

	ETeam Team = ETeam::None;
	if (const AOreRushPlayerState* PS = PC->GetPlayerState<AOreRushPlayerState>())
	{
		Team = PS->GetTeam();
	}

	if (ADepotZone* Depot = FindDepotForTeam(Team))
	{
		Pawn->SetActorLocation(Depot->GetActorLocation() + FVector(0.f, 0.f, 150.f), false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void AOreRushGameMode::PlacePlayersAtDepots()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		PlacePlayerAtDepot(It->Get());
	}
}

void AOreRushGameMode::RestartMatch()
{
	if (UWorld* World = GetWorld())
	{
		World->ServerTravel(TEXT("?restart"), false);
	}
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
