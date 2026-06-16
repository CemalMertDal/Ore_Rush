// Ore Rush — maç akışı (implementation).

#include "Game/OreRushGameMode.h"
#include "Character/OreRushCharacter.h"
#include "Game/OreRushGameState.h"
#include "Player/OreRushPlayerController.h"
#include "Player/OreRushPlayerState.h"

AOreRushGameMode::AOreRushGameMode()
{
	// C++ çekirdek sınıfları varsayılan olarak bağla. BP_OreRushGameMode bu sınıftan
	// türeyip pawn/controller'ı kozmetik BP versiyonlarıyla override edebilir.
	DefaultPawnClass = AOreRushCharacter::StaticClass();
	PlayerControllerClass = AOreRushPlayerController::StaticClass();
	PlayerStateClass = AOreRushPlayerState::StaticClass();
	GameStateClass = AOreRushGameState::StaticClass();
}
