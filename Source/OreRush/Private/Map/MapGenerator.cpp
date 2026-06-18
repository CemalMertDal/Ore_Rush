#include "Map/MapGenerator.h"
#include "Ore/OreVein.h"
#include "Game/DepotZone.h"
#include "Game/OreRushGameState.h"
#include "Game/OreRushGameMode.h"
#include "PowerUp/PowerUpBase.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"

AMapGenerator::AMapGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	AreaViz = CreateDefaultSubobject<UBoxComponent>(TEXT("AreaViz"));
	SetRootComponent(AreaViz);
	AreaViz->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AreaViz->SetBoxExtent(FVector(AreaExtent.X, AreaExtent.Y, 200.f));
	AreaViz->ShapeColor = FColor::Green;
	AreaViz->SetHiddenInGame(true);
	AreaViz->bIsEditorOnly = true;

	IronVeinClass = AOreVein::StaticClass();
	GoldVeinClass = AOreVein::StaticClass();
	DiamondVeinClass = AOreVein::StaticClass();
	DepotClass = ADepotZone::StaticClass();
}

void AMapGenerator::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (AreaViz)
	{
		AreaViz->SetBoxExtent(FVector(FMath::Max(1.f, AreaExtent.X), FMath::Max(1.f, AreaExtent.Y), 200.f));
	}
}

void AMapGenerator::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	int32 Seed = SeedOverride;
	if (!bOverrideSeed)
	{
		if (const AOreRushGameState* GS = GetWorld()->GetGameState<AOreRushGameState>())
		{
			Seed = GS->MapSeed;
		}
	}
	if (Seed == 0)
	{
		Seed = 1;
	}

	Generate(Seed);
}

bool AMapGenerator::PickPoint(FRandomStream& Rng, FVector& OutPoint)
{
	const FVector Origin = GetActorLocation();

	for (int32 Attempt = 0; Attempt < 40; ++Attempt)
	{
		const float X = Rng.FRandRange(-AreaExtent.X, AreaExtent.X);
		const float Y = Rng.FRandRange(-AreaExtent.Y, AreaExtent.Y);
		const FVector Candidate(Origin.X + X, Origin.Y + Y, Origin.Z + SpawnHeight);

		bool bOk = true;
		for (const FVector& Used : UsedPoints)
		{
			if (FVector::DistSquared2D(Candidate, Used) < MinDistance * MinDistance)
			{
				bOk = false;
				break;
			}
		}

		if (bOk)
		{
			OutPoint = Candidate;
			UsedPoints.Add(Candidate);
			return true;
		}
	}
	return false;
}

AOreVein* AMapGenerator::SpawnVein(TSubclassOf<AOreVein> Cls, EOreType Type, bool bUnlimited, int32 Units, const FVector& Location)
{
	if (!Cls)
	{
		Cls = AOreVein::StaticClass();
	}

	const FTransform SpawnTransform(FRotator::ZeroRotator, Location);
	AOreVein* Vein = GetWorld()->SpawnActorDeferred<AOreVein>(Cls, SpawnTransform, this, nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (Vein)
	{
		Vein->OreType = Type;
		Vein->bUnlimited = bUnlimited;
		Vein->RemainingUnits = Units;
		Vein->FinishSpawning(SpawnTransform);
	}
	return Vein;
}

void AMapGenerator::Generate(int32 Seed)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	FRandomStream Rng(Seed);
	const FVector Origin = GetActorLocation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (DepotClass)
	{
		const FVector RedLoc(Origin.X - DepotEdgeOffset, Origin.Y, Origin.Z + SpawnHeight);
		const FVector BlueLoc(Origin.X + DepotEdgeOffset, Origin.Y, Origin.Z + SpawnHeight);

		const FTransform RedT(FRotator::ZeroRotator, RedLoc);
		if (ADepotZone* Red = World->SpawnActorDeferred<ADepotZone>(DepotClass, RedT, this, nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
		{
			Red->Team = ETeam::Red;
			Red->FinishSpawning(RedT);
		}

		const FTransform BlueT(FRotator::ZeroRotator, BlueLoc);
		if (ADepotZone* Blue = World->SpawnActorDeferred<ADepotZone>(DepotClass, BlueT, this, nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
		{
			Blue->Team = ETeam::Blue;
			Blue->FinishSpawning(BlueT);
		}

		UsedPoints.Add(RedLoc);
		UsedPoints.Add(BlueLoc);
	}

	for (int32 i = 0; i < IronCount; ++i)
	{
		FVector P;
		if (PickPoint(Rng, P))
		{
			SpawnVein(IronVeinClass, EOreType::Iron, true, 0, P);
		}
	}

	for (int32 i = 0; i < GoldCount; ++i)
	{
		FVector P;
		if (PickPoint(Rng, P))
		{
			SpawnVein(GoldVeinClass, EOreType::Gold, false, GoldUnits, P);
		}
	}

	for (int32 i = 0; i < DiamondCount; ++i)
	{
		FVector P;
		if (PickPoint(Rng, P))
		{
			SpawnVein(DiamondVeinClass, EOreType::Diamond, false, DiamondUnits, P);
		}
	}

	if (PowerUpClasses.Num() > 0)
	{
		for (int32 i = 0; i < PowerUpCount; ++i)
		{
			FVector P;
			if (!PickPoint(Rng, P))
			{
				continue;
			}
			const int32 Idx = Rng.RandRange(0, PowerUpClasses.Num() - 1);
			if (TSubclassOf<APowerUpBase> Cls = PowerUpClasses[Idx])
			{
				World->SpawnActor<APowerUpBase>(Cls, P, FRotator::ZeroRotator, SpawnParams);
			}
		}
	}

	if (DecorClasses.Num() > 0)
	{
		for (int32 i = 0; i < DecorCount; ++i)
		{
			FVector P;
			if (!PickPoint(Rng, P))
			{
				continue;
			}
			const int32 Idx = Rng.RandRange(0, DecorClasses.Num() - 1);
			if (TSubclassOf<AActor> Cls = DecorClasses[Idx])
			{
				const FRotator Yaw(0.f, Rng.FRandRange(0.f, 360.f), 0.f);
				World->SpawnActor<AActor>(Cls, P, Yaw, SpawnParams);
			}
		}
	}

	if (AOreRushGameMode* GM = World->GetAuthGameMode<AOreRushGameMode>())
	{
		GM->EnsurePlayersAtDepots();
	}
}
