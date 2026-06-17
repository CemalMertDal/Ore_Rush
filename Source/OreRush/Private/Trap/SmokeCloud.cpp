#include "Trap/SmokeCloud.h"
#include "Components/SceneComponent.h"

ASmokeCloud::ASmokeCloud()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void ASmokeCloud::BeginPlay()
{
	Super::BeginPlay();
	OnSmokeFX();
}

void ASmokeCloud::ServerInit(float Duration)
{
	if (HasAuthority() && Duration > 0.f)
	{
		SetLifeSpan(Duration);
	}
}
