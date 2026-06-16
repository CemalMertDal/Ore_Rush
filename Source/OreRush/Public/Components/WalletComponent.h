// Ore Rush — cüzdan: taşınan cevher (replicated), kapasite, doluyken yavaşlama.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/OreRushTypes.h"
#include "WalletComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWalletChanged);

/**
 * UWalletComponent
 * Madencinin taşıdığı cevher. Tür bazında sayaç (replicated). Kazılan cevher buraya girer;
 * depoda kasaya yatırılır (skor). Kapasite sınırlı, doluyken hız düşer. Tuzakta düşürülür.
 * Tüm değişiklikler server-authoritative.
 */
UCLASS(ClassGroup = (OreRush), meta = (BlueprintSpawnableComponent))
class ORERUSH_API UWalletComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWalletComponent();

	/** Taşıma kapasitesi (toplam birim). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Wallet", meta = (ClampMin = "1"))
	int32 Capacity = 10;

	/** Cüzdan tam doluyken hız çarpanı (boşken 1.0 → doluyken bu değere lerp). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Wallet", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float FullSpeedMultiplier = 0.8f;

	/** Cüzdan her değiştiğinde (tüm makinelerde RepNotify ile) — HUD + hız güncelleme. */
	UPROPERTY(BlueprintAssignable, Category = "Ore Rush|Wallet")
	FOnWalletChanged OnWalletChanged;

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Wallet")
	int32 GetCount(EOreType Type) const;

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Wallet")
	int32 GetTotalUnits() const { return IronCount + GoldCount + DiamondCount; }

	/** Kasa değeri: demir×1 + altın×2 + elmas×5. */
	UFUNCTION(BlueprintPure, Category = "Ore Rush|Wallet")
	int32 GetTotalWorth() const;

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Wallet")
	bool IsFull() const { return GetTotalUnits() >= Capacity; }

	/** Yüke göre hız çarpanı (1.0 → FullSpeedMultiplier). */
	UFUNCTION(BlueprintPure, Category = "Ore Rush|Wallet")
	float GetSpeedMultiplier() const;

	/** Server-only: 1 birim ekle. Doluysa veya tür geçersizse false. */
	bool ServerAddOre(EOreType Type);

	/** Server-only: cüzdanı boşalt (depoya yatırma / tuzak). Boşaltılan toplam worth döner. */
	int32 ServerClear();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_Wallet, BlueprintReadOnly, Category = "Ore Rush|Wallet")
	int32 IronCount = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Wallet, BlueprintReadOnly, Category = "Ore Rush|Wallet")
	int32 GoldCount = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Wallet, BlueprintReadOnly, Category = "Ore Rush|Wallet")
	int32 DiamondCount = 0;

	UFUNCTION()
	void OnRep_Wallet();

private:
	int32* CountPtr(EOreType Type);
};
