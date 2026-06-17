#include "Player/OreRushPlayerController.h"
#include "Engine/Engine.h"

void AOreRushPlayerController::ClientShowAlarm_Implementation(const FString& Message)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Orange, Message);
	}
	OnAlarm(Message);
}
