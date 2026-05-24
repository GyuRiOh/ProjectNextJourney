// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DS1Define.h"
#include "Components/ActorComponent.h"
#include "DS1AttributeComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FDelegateOnAttributeChanged, EDS1AttributeType, float);
DECLARE_MULTICAST_DELEGATE(FOnDeath)

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DS1_API UDS1AttributeComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	/** 스탯 변경 Delegate */
	FDelegateOnAttributeChanged OnAttributeChanged;

	/** 죽음을 알리는 Delegate */
	FOnDeath OnDeath;

protected:

	UPROPERTY(EditAnywhere, Category="Stamina")
	float BaseStamina = 100.f;

	UPROPERTY(EditAnywhere, Category = "Stamina")
	float MaxStamina = 100.f;

	float StaminaRegenRate = 0.2f;

	/** 스태미나 회복 시작 전 대기 시간 */
	float StaminaRegenDelay = 2.f;

	UPROPERTY(EditAnywhere, Category = "Health")
	float BaseHealth = 100.f;

	UPROPERTY(EditAnywhere, Category = "Health")
	float MaxHealth = 100.f;

	UPROPERTY()
	float DefenseStat = 0.f;

	/** 스태미나 재충전 타이머 핸들 */
	FTimerHandle StaminaRegenTimerHandle;

	// ── 배고픔 ──

	UPROPERTY(EditAnywhere, Category = "Hunger")
	float MaxHunger = 100.f;

	UPROPERTY(VisibleAnywhere, Category = "Hunger")
	float CurrentHunger = 100.f;

	UPROPERTY(EditAnywhere, Category = "Hunger")
	float HungerDecayRate = 0.5f;

	// ── 갈증 ──

	UPROPERTY(EditAnywhere, Category = "Thirst")
	float MaxThirst = 100.f;

	UPROPERTY(VisibleAnywhere, Category = "Thirst")
	float CurrentThirst = 100.f;

	UPROPERTY(EditAnywhere, Category = "Thirst")
	float ThirstDecayRate = 0.8f;

	// ── 패널티 설정 ──

	/** 이 비율 이하면 low 상태로 간주 (기본 30%) */
	UPROPERTY(EditAnywhere, Category = "HungerThirst")
	float LowThreshold = 0.3f;

	/** 갈증 low 시 초당 HP 감소량 */
	UPROPERTY(EditAnywhere, Category = "HungerThirst")
	float ThirstHPDrainPerSecond = 2.f;

	/** 배고픔 low 시 이동속도 배율 */
	UPROPERTY(EditAnywhere, Category = "HungerThirst")
	float HungerSpeedPenaltyMultiplier = 0.7f;

	// ── 내부 상태 ──

	FTimerHandle HungerThirstDecayHandle;

	float CachedBaseMaxWalkSpeed = 0.f;

	bool bHungerLow = false;
	bool bThirstLow = false;
	bool bStaminaRegenSuppressed = false;

public:	
	UDS1AttributeComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	FORCEINLINE void SetStaminaRegenRate(float Rate) { StaminaRegenRate = Rate; }
	FORCEINLINE void SetStaminaRegenDelay(float Delay) { StaminaRegenDelay = Delay; }

	FORCEINLINE float GetBaseStamina() const { return BaseStamina; };
	FORCEINLINE float GetMaxStamina() const { return MaxStamina; };
	FORCEINLINE float GetBaseHealth() const { return BaseHealth; };
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; };

	FORCEINLINE void IncreaseDefense(const float DefenseAmount)
	{
		DefenseStat = DefenseStat + DefenseAmount;
	}

	FORCEINLINE void DecreaseDefense(const float DefenseAmount)
	{
		DefenseStat = DefenseStat - DefenseAmount;
	}

	/** 스테미나 비율 계산 */
	FORCEINLINE float GetStaminaRatio() const { return BaseStamina / MaxStamina; };
	FORCEINLINE float GetHealthRatio() const { return BaseHealth / MaxHealth; };

	/** 스테미너가 충분한지 체크 */
	bool CheckHasEnoughStamina(float StaminaCost) const;

	/** 스테미너 차감 */
	void DecreaseStamina(float StaminaCost);

	/** 스테미너 재충전/중지 토글. StartDelay < 0 이면 StaminaRegenDelay 사용 */
	void ToggleStaminaRegeneration(bool bEnabled, float StartDelay = -1.f);

	/** 스텟 변경을 통지하는 Broadcast Function */
	void BroadcastAttributeChanged(EDS1AttributeType InAttributeType) const;

	void TakeDamageAmount(float DamageAmount);

	/** 체력 회복 */
	void HealPlayer(float HealAmount);

	/** 배고픔 회복 */
	void RestoreHunger(float Amount);

	/** 갈증 회복 */
	void RestoreThirst(float Amount);

	FORCEINLINE float GetHungerRatio() const { return CurrentHunger / MaxHunger; }
	FORCEINLINE float GetThirstRatio() const { return CurrentThirst / MaxThirst; }

private:
	/** 스태미나 재충전 처리 핸들링 함수 */
	void RegenerateStaminaHandler();

	/** 1초마다 배고픔/갈증 감소 및 패널티 평가 */
	void HungerThirstDecayTick();

	/** 배고픔/갈증 수치에 따른 패널티 상태 적용 */
	void ApplyHungerThirstEffects();
		
};
