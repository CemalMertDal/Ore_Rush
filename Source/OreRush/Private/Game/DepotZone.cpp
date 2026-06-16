#include "Game/DepotZone.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Character/OreRushCharacter.h"
#include "Components/WalletComponent.h"
#include "Player/OreRushPlayerState.h"
#include "Game/OreRushGameState.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

ADepotZone::ADepotZone()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// OverlapBox kök bileşen olarak atanır.
	OverlapBox = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox"));
	OverlapBox->InitBoxExtent(FVector(150.f, 150.f, 100.f));
	OverlapBox->SetCollisionProfileName(TEXT("Trigger"));
	SetRootComponent(OverlapBox);

	// Görsel mesh.
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(OverlapBox);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ADepotZone::BeginPlay()
{
	Super::BeginPlay();

	// Sunucuda overlap olayını dinle.
	if (HasAuthority())
	{
		OverlapBox->OnComponentBeginOverlap.AddDynamic(this, &ADepotZone::OnOverlapBegin);
	}
}

void ADepotZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
								UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
								bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}

	AOreRushCharacter* Character = Cast<AOreRushCharacter>(OtherActor);
	if (!Character)
	{
		return;
	}

	AOreRushPlayerState* PS = Cast<AOreRushPlayerState>(Character->GetPlayerState());
	if (!PS)
	{
		return;
	}

	// Sadece bu deponun sahibi olan takımın oyuncuları teslimat yapabilir.
	if (PS->GetTeam() != Team || Team == ETeam::None)
	{
		FString Msg = FString::Printf(TEXT("[%s] Yanlis alan veya calinacak mal yok! (Senin takimin: %s, Depo takimi: %s)"),
			*Character->GetName(),
			(PS->GetTeam() == ETeam::Blue ? TEXT("Mavi") : (PS->GetTeam() == ETeam::Red ? TEXT("Kirmizi") : TEXT("None"))),
			(Team == ETeam::Blue ? TEXT("Mavi") : (Team == ETeam::Red ? TEXT("Kirmizi") : TEXT("None"))));
		
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Msg);
		}
		return;
	}

	UWalletComponent* Wallet = Character->GetWallet();
	if (!Wallet)
	{
		return;
	}

	const int32 TotalUnits = Wallet->GetTotalUnits();
	if (TotalUnits <= 0)
	{
		FString Msg = FString::Printf(TEXT("[%s] Depolanacak cevher yok! Cüzdan bos."), *Character->GetName());
		UE_LOG(LogTemp, Log, TEXT("%s"), *Msg);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, Msg);
		}
		return;
	}

	// Cüzdanı boşalt ve kazanılan değeri al.
	const int32 ClearedWorth = Wallet->ServerClear();
	if (ClearedWorth > 0)
	{
		if (AOreRushGameState* GS = GetWorld()->GetGameState<AOreRushGameState>())
		{
			GS->AddScore(Team, ClearedWorth);
		}
		MulticastOnDepositFX(ClearedWorth);

		FString Msg = FString::Printf(TEXT("[%s] %d adet cevher (%d degerinde) basariyla depolandi!"),
			*Character->GetName(), TotalUnits, ClearedWorth);
		
		UE_LOG(LogTemp, Display, TEXT("%s"), *Msg);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, Msg);
		}
	}
}

void ADepotZone::MulticastOnDepositFX_Implementation(int32 Amount)
{
	OnDepositFX(Amount);
}
