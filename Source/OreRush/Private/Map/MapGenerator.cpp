#include "Map/MapGenerator.h"
#include "Ore/OreVein.h"
#include "Game/DepotZone.h"
#include "Game/OreRushGameState.h"
#include "Game/OreRushGameMode.h"
#include "PowerUp/PowerUpBase.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
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

	VeinClass = AOreVein::StaticClass();
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
		FVector Candidate(Origin.X + X, Origin.Y + Y, Origin.Z);

		FVector GroundLoc;
		FVector GroundNormal;
		if (!GetGroundLocation(Candidate, GroundLoc, GroundNormal, true))
		{
			continue;
		}

		const float CosThreshold = FMath::Cos(FMath::DegreesToRadians(MaxSlopeAngle));
		if (GroundNormal.Z < CosThreshold)
		{
			continue;
		}

		Candidate = GroundLoc;

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

bool AMapGenerator::GetGroundLocation(const FVector& InLocation, FVector& OutGroundLocation, FVector& OutNormal, bool bCheckObstacles) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const FVector Start = FVector(InLocation.X, InLocation.Y, InLocation.Z + MaxTraceHeight);
	const FVector End = FVector(InLocation.X, InLocation.Y, InLocation.Z - MinTraceHeight);

	TArray<FHitResult> Hits;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (World->LineTraceMultiByChannel(Hits, Start, End, ECC_WorldStatic, Params))
	{
		int32 GroundHitIdx = -1;
		for (int32 i = Hits.Num() - 1; i >= 0; --i)
		{
			const FHitResult& Hit = Hits[i];
			if (!Hit.bBlockingHit)
			{
				continue;
			}

			AActor* HitActor = Hit.GetActor();
			UPrimitiveComponent* HitComponent = Hit.GetComponent();

			if (HitActor)
			{
				FString ClassName = HitActor->GetClass()->GetName();
				if (HitActor->IsA(APawn::StaticClass()) ||
					ClassName.Contains(TEXT("OreVein")) ||
					ClassName.Contains(TEXT("DepotZone")) ||
					ClassName.Contains(TEXT("PowerUp")) ||
					ClassName.Contains(TEXT("Character")))
				{
					continue;
				}
			}

			if (HitActor)
			{
				FString ActorName = HitActor->GetName();
				if (ActorName.Contains(TEXT("Tree")) || ActorName.Contains(TEXT("Foliage")) ||
					ActorName.Contains(TEXT("Bush")) || ActorName.Contains(TEXT("Rock")) ||
					ActorName.Contains(TEXT("Branch")) || ActorName.Contains(TEXT("Mushroom")) ||
					ActorName.Contains(TEXT("Plant")) || ActorName.Contains(TEXT("Cloud")))
				{
					continue;
				}
			}

			if (HitComponent)
			{
				FString ComponentName = HitComponent->GetName();
				FString CompClassName = HitComponent->GetClass()->GetName();
				if (CompClassName.Contains(TEXT("Foliage")) ||
					CompClassName.Contains(TEXT("InstancedStaticMesh")) ||
					ComponentName.Contains(TEXT("Foliage")))
				{
					continue;
				}
			}

			GroundHitIdx = i;
			break;
		}

		if (GroundHitIdx == -1)
		{
			for (int32 i = Hits.Num() - 1; i >= 0; --i)
			{
				if (Hits[i].bBlockingHit)
				{
					GroundHitIdx = i;
					break;
				}
			}
		}

		if (GroundHitIdx != -1)
		{
			const FHitResult& GroundHit = Hits[GroundHitIdx];

			if (bCheckObstacles)
			{
				const FHitResult& TopHit = Hits[0];
				if (TopHit.ImpactPoint.Z - GroundHit.ImpactPoint.Z > 80.f)
				{
					AActor* TopActor = TopHit.GetActor();
					if (TopActor)
					{
						FString ActorName = TopActor->GetName();
						if (ActorName.Contains(TEXT("Tree")) || ActorName.Contains(TEXT("Rock")) ||
							ActorName.Contains(TEXT("Bush")) || ActorName.Contains(TEXT("Branch")) ||
							ActorName.Contains(TEXT("Mushroom")) || ActorName.Contains(TEXT("Plant")) ||
							ActorName.Contains(TEXT("Cloud")))
						{
							return false;
						}
					}

					UPrimitiveComponent* TopComp = TopHit.GetComponent();
					if (TopComp)
					{
						FString CompClassName = TopComp->GetClass()->GetName();
						FString CompName = TopComp->GetName();
						if (CompClassName.Contains(TEXT("Foliage")) ||
							CompClassName.Contains(TEXT("InstancedStaticMesh")) ||
							CompName.Contains(TEXT("Foliage")))
						{
							return false;
						}
					}
				}
			}

			OutGroundLocation = GroundHit.ImpactPoint;
			OutNormal = GroundHit.ImpactNormal;
			return true;
		}
	}

	return false;
}

AOreVein* AMapGenerator::SpawnVein(EOreType Type, bool bUnlimited, int32 Units, const FVector& Location)
{
	TSubclassOf<AOreVein> Cls = VeinClass;
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

int32 AMapGenerator::ScaledCount(int32 Base) const
{
	if (!bScaleWithArea || Base <= 0)
	{
		return Base;
	}

	const float RefSize = ReferenceArea.X * ReferenceArea.Y;
	if (RefSize <= 0.f)
	{
		return Base;
	}

	const float Scale = (AreaExtent.X * AreaExtent.Y) / RefSize;
	return FMath::Max(1, FMath::RoundToInt(Base * Scale));
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

	const int32 NumIron = ScaledCount(IronCount);
	const int32 NumGold = ScaledCount(GoldCount);
	const int32 NumDiamond = ScaledCount(DiamondCount);
	const int32 NumPowerUp = ScaledCount(PowerUpCount);
	const int32 NumDecor = ScaledCount(DecorCount);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (DepotClass)
	{
		FVector RedLoc(Origin.X - DepotEdgeOffset, Origin.Y, Origin.Z);
		FVector RedNormal = FVector::UpVector;
		FVector RedGround;
		if (GetGroundLocation(RedLoc, RedGround, RedNormal, false))
		{
			RedLoc = RedGround;
		}
		else
		{
			RedLoc.Z = Origin.Z;
		}

		FVector BlueLoc(Origin.X + DepotEdgeOffset, Origin.Y, Origin.Z);
		FVector BlueNormal = FVector::UpVector;
		FVector BlueGround;
		if (GetGroundLocation(BlueLoc, BlueGround, BlueNormal, false))
		{
			BlueLoc = BlueGround;
		}
		else
		{
			BlueLoc.Z = Origin.Z;
		}

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

	for (int32 i = 0; i < NumIron; ++i)
	{
		FVector P;
		if (PickPoint(Rng, P))
		{
			SpawnVein(EOreType::Iron, true, 0, P);
		}
	}

	for (int32 i = 0; i < NumGold; ++i)
	{
		FVector P;
		if (PickPoint(Rng, P))
		{
			SpawnVein(EOreType::Gold, false, GoldUnits, P);
		}
	}

	for (int32 i = 0; i < NumDiamond; ++i)
	{
		FVector P;
		if (PickPoint(Rng, P))
		{
			SpawnVein(EOreType::Diamond, false, DiamondUnits, P);
		}
	}

	if (PowerUpClasses.Num() > 0)
	{
		for (int32 i = 0; i < NumPowerUp; ++i)
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
		for (int32 i = 0; i < NumDecor; ++i)
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
