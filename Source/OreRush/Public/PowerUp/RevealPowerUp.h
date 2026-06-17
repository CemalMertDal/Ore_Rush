#pragma once

#include "CoreMinimal.h"
#include "PowerUp/PowerUpBase.h"
#include "RevealPowerUp.generated.h"

UCLASS()
class ORERUSH_API ARevealPowerUp : public APowerUpBase
{
	GENERATED_BODY()

protected:
	virtual void ServerApplyTo(AOreRushCharacter* User) override;
};
