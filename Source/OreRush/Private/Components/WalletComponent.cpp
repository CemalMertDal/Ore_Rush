// Ore Rush — cüzdan (implementation).

#include "Components/WalletComponent.h"
#include "Net/UnrealNetwork.h"

UWalletComponent::UWalletComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

int32* UWalletComponent::CountPtr(EOreType Type)
{
	switch (Type)
	{
	case EOreType::Iron:    return &IronCount;
	case EOreType::Gold:    return &GoldCount;
	case EOreType::Diamond: return &DiamondCount;
	default:                return nullptr;
	}
}

int32 UWalletComponent::GetCount(EOreType Type) const
{
	switch (Type)
	{
	case EOreType::Iron:    return IronCount;
	case EOreType::Gold:    return GoldCount;
	case EOreType::Diamond: return DiamondCount;
	default:                return 0;
	}
}

int32 UWalletComponent::GetTotalWorth() const
{
	return IronCount * OreWorth(EOreType::Iron)
		+ GoldCount * OreWorth(EOreType::Gold)
		+ DiamondCount * OreWorth(EOreType::Diamond);
}

float UWalletComponent::GetSpeedMultiplier() const
{
	if (Capacity <= 0)
	{
		return 1.f;
	}
	const float Load = FMath::Clamp(static_cast<float>(GetTotalUnits()) / Capacity, 0.f, 1.f);
	return FMath::Lerp(1.f, FullSpeedMultiplier, Load);
}

bool UWalletComponent::ServerAddOre(EOreType Type)
{
	if (GetOwnerRole() != ROLE_Authority || IsFull())
	{
		return false;
	}

	int32* Slot = CountPtr(Type);
	if (Slot == nullptr)
	{
		return false;
	}

	(*Slot)++;
	OnRep_Wallet(); // RepNotify client'larda; sunucuda elle tetikle.
	return true;
}

int32 UWalletComponent::ServerClear()
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return 0;
	}

	const int32 Worth = GetTotalWorth();
	if (Worth > 0 || GetTotalUnits() > 0)
	{
		IronCount = 0;
		GoldCount = 0;
		DiamondCount = 0;
		OnRep_Wallet();
	}
	return Worth;
}

bool UWalletComponent::ServerSpendWorth(int32 Cost)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}
	if (Cost <= 0)
	{
		return true;
	}
	if (GetTotalWorth() < Cost)
	{
		return false;
	}

	int32 Remaining = Cost;
	while (Remaining > 0 && IronCount > 0)
	{
		IronCount--;
		Remaining -= OreWorth(EOreType::Iron);
	}
	while (Remaining > 0 && GoldCount > 0)
	{
		GoldCount--;
		Remaining -= OreWorth(EOreType::Gold);
	}
	while (Remaining > 0 && DiamondCount > 0)
	{
		DiamondCount--;
		Remaining -= OreWorth(EOreType::Diamond);
	}

	OnRep_Wallet();
	return true;
}

void UWalletComponent::ServerTakeAll(int32& OutIron, int32& OutGold, int32& OutDiamond)
{
	OutIron = 0;
	OutGold = 0;
	OutDiamond = 0;

	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	OutIron = IronCount;
	OutGold = GoldCount;
	OutDiamond = DiamondCount;

	if (IronCount + GoldCount + DiamondCount > 0)
	{
		IronCount = 0;
		GoldCount = 0;
		DiamondCount = 0;
		OnRep_Wallet();
	}
}

void UWalletComponent::OnRep_Wallet()
{
	OnWalletChanged.Broadcast();
}

void UWalletComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWalletComponent, IronCount);
	DOREPLIFETIME(UWalletComponent, GoldCount);
	DOREPLIFETIME(UWalletComponent, DiamondCount);
}
