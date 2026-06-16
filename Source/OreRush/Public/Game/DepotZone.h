// Ore Rush — depo alanı: takım kasası (cüzdan boşaltma ve skor güncelleme).

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/OreRushTypes.h"
#include "DepotZone.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

/**
 * ADepotZone
 * Takım deposu. Kendi takımından bir oyuncu girdiğinde cüzdanındaki cevherleri teslim alarak
 * takım kasasına (GameState skoruna) ekler. Server-authoritative.
 */
UCLASS()
class ORERUSH_API ADepotZone : public AActor
{
	GENERATED_BODY()

public:
	ADepotZone();

	/** Bu deponun ait olduğu takım. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ore Rush|Depot")
	ETeam Team = ETeam::None;

protected:
	virtual void BeginPlay() override;

	/** Overlap/tetikleme alanı. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Depot")
	TObjectPtr<UBoxComponent> OverlapBox;

	/** Görsel depo yapısı (BP tarafında atanabilir). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Depot")
	TObjectPtr<UStaticMeshComponent> Mesh;

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
};
