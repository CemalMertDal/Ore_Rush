// Ore Rush — third-person competitive miner character (implementation).

#include "Character/OreRushCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/WalletComponent.h"
#include "Ore/OreVein.h"
#include "CollisionQueryParams.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

AOreRushCharacter::AOreRushCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// Capsule.
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	// Kamera kontrol rotasyonunu mesh'e değil, hareket yönüne bağla (TPS).
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// CharacterMovement: hareket yönüne dön (ödev şartı: bOrientRotationToMovement = true).
	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->bOrientRotationToMovement = true;
	Move->RotationRate = FRotator(0.f, 500.f, 0.f);
	Move->JumpZVelocity = 500.f;
	Move->AirControl = 0.35f;
	Move->MaxWalkSpeed = 500.f;
	Move->MinAnalogWalkSpeed = 20.f;
	Move->BrakingDecelerationWalking = 2000.f;
	Move->BrakingDecelerationFalling = 1500.f;

	// SpringArm (kamera kolu) — pawn control rotation ile döner.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;

	// Takip kamerası — kol ucundaki sokete bağlı, kendi rotasyonunu kullanmaz.
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Cüzdan (taşınan cevher — replicated bileşen).
	Wallet = CreateDefaultSubobject<UWalletComponent>(TEXT("Wallet"));
}

void AOreRushCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Yerel olarak kontrol edilen oyuncuda Enhanced Input mapping context'i ekle.
	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
			if (MouseLookMappingContext)
			{
				// Mouse look ayrı context — template yapısıyla aynı.
				Subsystem->AddMappingContext(MouseLookMappingContext, 0);
			}
		}
	}

	// Cüzdan yüküne göre hız: değişimi dinle (RepNotify her makinede tetikler).
	BaseWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	if (Wallet)
	{
		Wallet->OnWalletChanged.AddDynamic(this, &AOreRushCharacter::UpdateCarrySpeed);
	}
}

void AOreRushCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AOreRushCharacter::Move);
		}
		if (LookAction)
		{
			// Gamepad bakış.
			EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AOreRushCharacter::Look);
		}
		if (MouseLookAction)
		{
			// Mouse bakış — template'le aynı: ayrı action, aynı Look handler.
			EIC->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AOreRushCharacter::Look);
		}
		if (JumpAction)
		{
			EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
			EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}
		if (DashAction)
		{
			EIC->BindAction(DashAction, ETriggerEvent::Started, this, &AOreRushCharacter::DashInput);
		}
		if (MineAction)
		{
			// Basılı tut: Started → kazmaya başla, Completed → bırak.
			EIC->BindAction(MineAction, ETriggerEvent::Started, this, &AOreRushCharacter::StartMine);
			EIC->BindAction(MineAction, ETriggerEvent::Completed, this, &AOreRushCharacter::StopMine);
		}
	}
}

void AOreRushCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();

	if (Controller == nullptr)
	{
		return;
	}

	// Kamera (control) yaw'ına göre ileri/sağ yönleri hesapla.
	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, Axis.Y);
	AddMovementInput(Right, Axis.X);

	// Dash hedeflemesi için son world-space hareket yönünü sakla.
	LastMoveWorldDir = (Forward * Axis.Y + Right * Axis.X).GetSafeNormal();
}

void AOreRushCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();

	if (Controller == nullptr)
	{
		return;
	}

	AddControllerYawInput(Axis.X * LookSensitivity);
	AddControllerPitchInput(Axis.Y * LookSensitivity);
}

void AOreRushCharacter::DashInput()
{
	// Yerel tahmini cooldown kapısı: sunucuyu boş yere spam'lemez, HUD'ı besler.
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastDashRequestTime < DashCooldown)
	{
		return;
	}
	LastDashRequestTime = Now;

	// Yön: önce mevcut hız, sonra son input yönü, en son karakterin baktığı yön.
	FVector Dir = GetVelocity().GetSafeNormal2D();
	if (Dir.IsNearlyZero())
	{
		Dir = LastMoveWorldDir.GetSafeNormal2D();
	}
	if (Dir.IsNearlyZero())
	{
		Dir = GetActorForwardVector().GetSafeNormal2D();
	}

	ServerDash(Dir);
}

bool AOreRushCharacter::ServerDash_Validate(FVector DashDirection)
{
	// NaN/sonsuz yön reddedilir; geri kalanı sunucu normalize eder.
	return !DashDirection.ContainsNaN();
}

void AOreRushCharacter::ServerDash_Implementation(FVector DashDirection)
{
	// Otorite cooldown kontrolü (gerçek karar burada).
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastDashTime < DashCooldown)
	{
		return;
	}
	LastDashTime = Now;

	FVector Dir = DashDirection.GetSafeNormal2D();
	if (Dir.IsNearlyZero())
	{
		Dir = GetActorForwardVector().GetSafeNormal2D();
	}

	const FVector LaunchVelocity = Dir * DashImpulse + FVector(0.f, 0.f, DashVerticalImpulse);

	// XY'yi dash hızıyla ez; dikey itme varsa Z'yi de ez.
	LaunchCharacter(LaunchVelocity, /*bXYOverride=*/true, /*bZOverride=*/DashVerticalImpulse > 0.f);

	// Kozmetik efekt herkese.
	MulticastDashFX();
}

void AOreRushCharacter::MulticastDashFX_Implementation()
{
	// Sunum: BP'de Niagara/SFX'e bağlanır (mantık değil).
	OnDashFX();
}

float AOreRushCharacter::GetDashCooldownRemaining() const
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return 0.f;
	}

	const float Elapsed = World->GetTimeSeconds() - LastDashRequestTime;
	return FMath::Max(0.f, DashCooldown - Elapsed);
}

//~ Kazma (mining) --------------------------------------------------------------

void AOreRushCharacter::StartMine()
{
	ServerStartMine();
}

void AOreRushCharacter::StopMine()
{
	ServerStopMine();
}

void AOreRushCharacter::ServerStartMine_Implementation()
{
	if (bIsMining)
	{
		return;
	}

	// Başlatmak için bir damara bakıyor olmalı (geri bildirim + MineTime'ı al).
	AOreVein* Vein = TraceForVein();
	if (Vein == nullptr || !Vein->CanBeMined())
	{
		return;
	}

	bIsMining = true;
	OnRep_IsMining(); // sunucuda da tetikle

	const float Interval = FMath::Max(0.05f, Vein->MineTime);
	GetWorldTimerManager().SetTimer(MineTimerHandle, this, &AOreRushCharacter::MineTick, Interval, true, Interval);
}

void AOreRushCharacter::ServerStopMine_Implementation()
{
	GetWorldTimerManager().ClearTimer(MineTimerHandle);
	if (bIsMining)
	{
		bIsMining = false;
		OnRep_IsMining();
	}
}

void AOreRushCharacter::MineTick()
{
	if (!HasAuthority())
	{
		return;
	}

	// Hâlâ kazılabilir bir damara bakıyor mu?
	AOreVein* Vein = TraceForVein();
	if (Vein == nullptr || !Vein->CanBeMined())
	{
		GetWorldTimerManager().ClearTimer(MineTimerHandle);
		bIsMining = false;
		OnRep_IsMining();
		return;
	}

	// Cüzdan doluysa birim harcamadan bekle (sınırlı cevheri ziyan etme).
	if (Wallet && Wallet->IsFull())
	{
		return;
	}

	const EOreType Extracted = Vein->ServerExtractOne();
	if (Extracted != EOreType::None && Wallet)
	{
		Wallet->ServerAddOre(Extracted);
	}
}

AOreVein* AOreRushCharacter::TraceForVein() const
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return nullptr;
	}

	// Kameraya (control rotation) göre göz hizasından ileri trace.
	const FVector Start = GetActorLocation() + FVector(0.f, 0.f, BaseEyeHeight);
	const FVector End = Start + GetControlRotation().Vector() * MineRange;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FHitResult Hit;
	if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		return Cast<AOreVein>(Hit.GetActor());
	}
	return nullptr;
}

void AOreRushCharacter::OnRep_IsMining()
{
	OnMiningStateChanged(bIsMining);
}

void AOreRushCharacter::UpdateCarrySpeed()
{
	if (Wallet)
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * Wallet->GetSpeedMultiplier();
	}
}

void AOreRushCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOreRushCharacter, bIsMining);
}
