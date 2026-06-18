// Ore Rush — paylaşılan tipler (enum/struct). Mantık içermez.

#pragma once

#include "CoreMinimal.h"
#include "OreRushTypes.generated.h"

/** Takım kimliği. Kırmızı vs Mavi madenciler. */
UENUM(BlueprintType)
enum class ETeam : uint8
{
	None UMETA(DisplayName = "None"),
	Red  UMETA(DisplayName = "Red"),
	Blue UMETA(DisplayName = "Blue")
};

/** Cevher türü. Enum değeri = cevher değeri (worth): demir 1x, altın 2x, elmas 5x. */
UENUM(BlueprintType)
enum class EOreType : uint8
{
	None    = 0 UMETA(DisplayName = "None"),
	Iron    = 1 UMETA(DisplayName = "Iron"),
	Gold    = 2 UMETA(DisplayName = "Gold"),
	Diamond = 5 UMETA(DisplayName = "Diamond")
};

/** Bir cevher biriminin kasa değeri (= enum değeri). */
FORCEINLINE int32 OreWorth(EOreType Type)
{
	return static_cast<int32>(Type);
}

/** Power-up buff türleri (HUD ikonları için). */
UENUM(BlueprintType)
enum class EOreRushBuff : uint8
{
	Speed   UMETA(DisplayName = "Speed"),
	Mining  UMETA(DisplayName = "Mining"),
	Shield  UMETA(DisplayName = "Shield"),
	Reveal  UMETA(DisplayName = "Reveal")
};
