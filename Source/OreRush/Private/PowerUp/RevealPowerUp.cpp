#include "PowerUp/RevealPowerUp.h"
#include "Character/OreRushCharacter.h"

void ARevealPowerUp::ServerApplyTo(AOreRushCharacter* User)
{
	if (User)
	{
		User->ServerApplyReveal(BuffDuration);
	}
}
