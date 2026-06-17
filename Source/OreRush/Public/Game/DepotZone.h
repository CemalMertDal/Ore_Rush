// Ore Rush — depo alanı: takım kasası (cüzdan boşaltma ve skor güncelleme).

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/OreRushTypes.h"
#include "Interaction/OreRushInteractable.h"
#include "DepotZone.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class AOreRushCharacter;

/**
 * ADepotZone
 * Takım deposu. Kendi takımından bir oyuncu girdiğinde cüzdanındaki cevherleri teslim alarak
 * takım kasasına (GameState skoruna) ekler. Server-authoritative.
 */
UCLASS()
class ORERUSH_API ADepotZone : public AActor, public IOreRushInteractable
{
	GENERATED_BODY()

public:
	ADepotZone();

	/** Bu deponun ait olduğu takım. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ore Rush|Depot")
	ETeam Team = ETeam::None;

	virtual bool CanInteract(AOreRushCharacter* User) const override;
	virtual void ServerStartInteract(AOreRushCharacter* User) override;
	virtual void ServerStopInteract(AOreRushCharacter* User) override;

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Depot")
	bool IsBeingRaided() const { return bBeingRaided; }

	/** Baskında saniyede çalınan worth (tick başına). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ore Rush|Depot|Raid", meta = (ClampMin = "1"))
	int32 StealPerTick = 2;

	/** Baskın tick aralığı (saniye). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ore Rush|Depot|Raid", meta = (ClampMin = "0.05"))
	float RaidTickInterval = 0.5f;

	/** Busted sonrası deponun tekrar baskına kapalı kalma süresi (saniye). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ore Rush|Depot|Raid", meta = (ClampMin = "0.0"))
	float RaidGrace = 5.f;

	/** Bu yarıçaptaki her dost savunma, tick başına çalınan worth'ü 1 azaltır (min 1). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ore Rush|Depot|Raid", meta = (ClampMin = "0.0"))
	float ReinforceRadius = 1500.f;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Overlap/tetikleme alanı. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Depot")
	TObjectPtr<UBoxComponent> OverlapBox;

	/** Baskın için bakış-trace hedefi (yalnız Visibility kanalını bloklar, hareketi engellemez). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Depot")
	TObjectPtr<UBoxComponent> InteractBox;

	/** Görsel depo yapısı (BP tarafında atanabilir). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Depot")
	TObjectPtr<UStaticMeshComponent> Mesh;

	/** Baskın altında mı (replicated — sunum/marker). */
	UPROPERTY(ReplicatedUsing = OnRep_BeingRaided, BlueprintReadOnly, Category = "Ore Rush|Depot|Raid")
	bool bBeingRaided = false;

	UFUNCTION()
	void OnRep_BeingRaided();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Depot|Raid")
	void OnRaidStateChanged(bool bRaided);

	/** Overlap başladığında tetiklenir (yalnız sunucu). */
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
						bool bFromSweep, const FHitResult& SweepResult);

	/** Cevher teslimat efekti (herkes). */
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastOnDepositFX(int32 Amount);

	/** FX hook (BP: ses/Niagara için doldurulur). */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Depot")
	void OnDepositFX(int32 Amount);

private:
	void RaidTick();
	void EndRaid(bool bBusted);
	bool IsOwnerPresent() const;
	int32 CountTeamDefensesNear() const;
	void SendAlarmToOwner(const FString& Message) const;
	ADepotZone* FindDepotForTeam(ETeam InTeam) const;

	TWeakObjectPtr<AOreRushCharacter> CurrentRaider;
	int32 StolenWorthThisRaid = 0;
	float RaidClosedUntil = -1000.f;
	FTimerHandle RaidTimerHandle;
};
