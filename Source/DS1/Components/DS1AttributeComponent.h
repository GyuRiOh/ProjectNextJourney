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

	UPROPERTY(EditAnywhere, Category = "Stamina")
	float StaminaRegenRate = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Health")
	float BaseHealth = 100.f;

	UPROPERTY(EditAnywhere, Category = "Health")
	float MaxHealth = 100.f;

	UPROPERTY()
	float DefenseStat = 0.f;

	/** 스태미나 재충전 타이머 핸들 */
	FTimerHandle StaminaRegenTimerHandle;

public:	
	UDS1AttributeComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
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

	/** 스테미너 재충전/중지 토글 */
	void ToggleStaminaRegeneration(bool bEnabled, float StartDelay = 2.f);

	/** 스텟 변경을 통지하는 Broadcast Function */
	void BroadcastAttributeChanged(EDS1AttributeType InAttributeType) const;

	void TakeDamageAmount(float DamageAmount);

	/** 체력 회복 */
	void HealPlayer(float HealAmount);

private:
	/** 스태미나 재충전 처리 핸들링 함수 */
	void RegenerateStaminaHandler();
		
};
