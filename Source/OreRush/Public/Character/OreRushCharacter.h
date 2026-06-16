// Ore Rush — third-person competitive miner character.
// Çekirdek mantık C++'ta: hareket, Enhanced Input, server-authoritative dash (G8).

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "OreRushCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UWalletComponent;
class AOreVein;
struct FInputActionValue;

/**
 * AOreRushCharacter
 * Third-person (TPS) madenci karakteri. SpringArm + Camera, Enhanced Input ile
 * WASD hareket / mouse look / jump / dash. Dash server-authoritative:
 * client niyeti ServerDash ile yollar, sunucu cooldown'u doğrular ve LaunchCharacter uygular.
 */
UCLASS()
class ORERUSH_API AOreRushCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AOreRushCharacter();

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/** Yerel oyuncu için kalan dash cooldown'u (HUD/feedback). */
	UFUNCTION(BlueprintPure, Category = "Ore Rush|Dash")
	float GetDashCooldownRemaining() const;

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Components")
	UWalletComponent* GetWallet() const { return Wallet; }

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//~ Kamera (TPS) ------------------------------------------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	//~ Bileşenler --------------------------------------------------------------
	/** Taşınan cevher (cüzdan). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Components")
	TObjectPtr<UWalletComponent> Wallet;

	//~ Enhanced Input (BP'de atanır) ------------------------------------------
	/** Gamepad/klavye context'i (IMC_Default). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** Mouse bakış context'i (IMC_MouseLook) — keskin mouse hissi buradan gelir. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Input")
	TObjectPtr<UInputMappingContext> MouseLookMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Input")
	TObjectPtr<UInputAction> MoveAction;

	/** Gamepad sağ-çubuk bakış. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Input")
	TObjectPtr<UInputAction> LookAction;

	/** Mouse bakış (ham delta — yumuşak/hassas). IA_MouseLook atanır. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Input")
	TObjectPtr<UInputAction> MouseLookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Input")
	TObjectPtr<UInputAction> DashAction;

	/** Kazma (basılı tut). IA_Mine atanır. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Input")
	TObjectPtr<UInputAction> MineAction;

	/** Bakış hassasiyeti çarpanı. Kamera yavaş dönüyorsa büyüt (örn. 2-3). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Input", meta = (ClampMin = "0.0"))
	float LookSensitivity = 1.0f;

	//~ Dash ayarları (G8) — BP'de tune edilebilir ------------------------------
	/** Dash'in yatay hız büyüklüğü (cm/s). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Dash", meta = (ClampMin = "0.0"))
	float DashImpulse = 1500.f;

	/** Dash sırasında dikey itme (0 = düz yatay dash). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Dash", meta = (ClampMin = "0.0"))
	float DashVerticalImpulse = 0.f;

	/** İki dash arası bekleme (saniye). Sunucuda otorite olarak uygulanır. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Dash", meta = (ClampMin = "0.0"))
	float DashCooldown = 1.5f;

	//~ Sunum hook'u (yalnız BP: Niagara/SFX — mantık değil) --------------------
	/** Dash görselleştirme. Multicast tarafından tüm makinelerde çağrılır. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Dash")
	void OnDashFX();

	//~ Kazma (mining) ----------------------------------------------------------
	/** Damar arama menzili (kameradan ileri trace). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Mining", meta = (ClampMin = "0.0"))
	float MineRange = 400.f;

	/** Şu an kazıyor mu (replicated — anim/FX için). */
	UPROPERTY(ReplicatedUsing = OnRep_IsMining, BlueprintReadOnly, Category = "Ore Rush|Mining")
	bool bIsMining = false;

	UFUNCTION()
	void OnRep_IsMining();

	/** Kazma durumu değişti (BP: pickaxe sesi/anim tetikleyici). */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Mining")
	void OnMiningStateChanged(bool bMining);

private:
	//~ Input handler'ları ------------------------------------------------------
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	/** Yerel: dash yönünü hesaplar ve niyeti sunucuya yollar. */
	void DashInput();

	//~ Dash networking ---------------------------------------------------------
	/** Client → Server: dash niyeti (yön world-space, normalize edilmiş). */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDash(FVector DashDirection);

	/** Server → herkes: kozmetik dash efekti tetikleyicisi. */
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDashFX();

	//~ Kazma networking --------------------------------------------------------
	/** Yerel input: kazmayı başlat/bitir → niyeti sunucuya yolla. */
	void StartMine();
	void StopMine();

	UFUNCTION(Server, Reliable)
	void ServerStartMine();

	UFUNCTION(Server, Reliable)
	void ServerStopMine();

	/** Server: MineTime aralığıyla çağrılır — bakılan damardan 1 birim çıkar. */
	void MineTick();

	/** Kameradan ileri trace ile bakılan damarı bul (sunucu). */
	AOreVein* TraceForVein() const;

	/** Cüzdan yüküne göre yürüme hızını güncelle (RepNotify ile her makinede). */
	UFUNCTION()
	void UpdateCarrySpeed();

	FTimerHandle MineTimerHandle;
	float BaseWalkSpeed = 500.f;

	/** En son world-space hareket yönü (dash hedefleme için). */
	FVector LastMoveWorldDir = FVector::ZeroVector;

	/** Yerel tahmini cooldown kapısı + HUD (yalnız sahip client / host). */
	float LastDashRequestTime = -1000.f;

	/** Sunucu-otorite cooldown zaman damgası. */
	float LastDashTime = -1000.f;
};
