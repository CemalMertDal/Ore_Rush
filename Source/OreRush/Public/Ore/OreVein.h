// Ore Rush — cevher damarı: kazılabilir kaynak (demir sınırsız / altın-elmas sınırlı).

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/OreRushTypes.h"
#include "OreVein.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

/**
 * AOreVein
 * Kazılabilir cevher damarı. Demir sınırsız (bUnlimited), altın/elmas sınırlı (RemainingUnits,
 * replicated). Karakter bakıp kazma tuşunu basılı tutunca sunucu MineTime aralıklarıyla
 * ServerExtractOne çağırır; sınırlı damar tükenince yok olur. Server-authoritative.
 */
UCLASS()
class ORERUSH_API AOreVein : public AActor
{
	GENERATED_BODY()

public:
	AOreVein();

	/** Cevher türü (worth = enum değeri). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ore Rush|Vein")
	EOreType OreType = EOreType::Iron;

	/** True ise sınırsız kazılır (demir). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ore Rush|Vein")
	bool bUnlimited = true;

	/** Kalan birim (yalnız sınırlı damarlarda; altın/elmas). */
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_RemainingUnits, BlueprintReadOnly, Category = "Ore Rush|Vein", meta = (ClampMin = "0", EditCondition = "!bUnlimited"))
	int32 RemainingUnits = 10;

	/** Bir birim kazma süresi (saniye). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ore Rush|Vein", meta = (ClampMin = "0.05"))
	float MineTime = 1.0f;

	/** Bu damar daha kazılabilir mi? (sunucu mantığı + UI) */
	UFUNCTION(BlueprintPure, Category = "Ore Rush|Vein")
	bool CanBeMined() const { return !bDepleted && (bUnlimited || RemainingUnits > 0); }

	/** Server-only: 1 birim çıkar. Çıkan türü döner; tükendiyse EOreType::None. */
	EOreType ServerExtractOne();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Etkileşim/çarpışma kök bileşeni (mesh atanmasa da trace edilebilir). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Vein")
	TObjectPtr<UBoxComponent> InteractBox;

	/** Görsel mesh (kozmetik — BP'de atanır). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Vein")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UFUNCTION()
	void OnRep_RemainingUnits();

	/** Kazma vuruşu efekti (herkes — demir dahil). */
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastMineHit();

	/** FX hook'ları (yalnız BP: Niagara/SFX — mantık değil). */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Vein")
	void OnMineHitFX();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Vein")
	void OnDepletedFX();

private:
	/** Server: damarı tüket → FX + kısa gecikmeyle yok et. */
	void Deplete();

	bool bDepleted = false;
};
