// Ore Rush — cevher damarı (implementation).

#include "Ore/OreVein.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

AOreVein::AOreVein()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// Kök = çarpışma kutusu: mesh atanmasa da trace/etkileşim çalışır.
	InteractBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractBox"));
	InteractBox->InitBoxExtent(FVector(50.f, 50.f, 50.f));
	InteractBox->SetCollisionProfileName(TEXT("BlockAll"));
	SetRootComponent(InteractBox);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(InteractBox);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

EOreType AOreVein::ServerExtractOne()
{
	if (!HasAuthority() || bDepleted)
	{
		return EOreType::None;
	}

	if (bUnlimited)
	{
		MulticastMineHit();
		return OreType;
	}

	if (RemainingUnits <= 0)
	{
		return EOreType::None;
	}

	RemainingUnits--;
	MulticastMineHit();
	OnRep_RemainingUnits(); // sunucuda da güncelle (depletion FX vb.)

	if (RemainingUnits <= 0)
	{
		Deplete();
	}

	return OreType;
}

void AOreVein::Deplete()
{
	bDepleted = true;
	SetActorEnableCollision(false);

	// Multicast/RepNotify'ın client'lara ulaşması için kısa gecikmeyle yok et.
	SetLifeSpan(0.2f);
}

void AOreVein::OnRep_RemainingUnits()
{
	if (!bUnlimited && RemainingUnits <= 0)
	{
		OnDepletedFX();
	}
}

void AOreVein::MulticastMineHit_Implementation()
{
	OnMineHitFX();
}

void AOreVein::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOreVein, RemainingUnits);
}
