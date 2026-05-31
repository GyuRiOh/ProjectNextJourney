// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DS1CharacterStatData.generated.h"

UCLASS(BlueprintType)
class DS1_API UDS1CharacterStatData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// ── 체력 ──

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.f;

	// ── 스태미나 ──

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (ClampMin = "1.0"))
	float MaxStamina = 100.f;

	/** 0.1초마다 회복되는 스태미나 양 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (ClampMin = "0.0"))
	float StaminaRegenRate = 0.2f;

	/** 스태미나 소모 후 회복 시작까지 대기 시간 (초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (ClampMin = "0.0", Units = "s"))
	float StaminaRegenDelay = 2.f;

	// ── 배고픔 ──

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hunger", meta = (ClampMin = "1.0"))
	float MaxHunger = 100.f;

	/** 배고픔 1이 감소하는 데 걸리는 시간 (초). 클수록 천천히 줄어든다 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hunger", meta = (ClampMin = "0.1", Units = "s"))
	float SecondsPerHungerDecay = 2.0f;

	// ── 갈증 ──

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Thirst", meta = (ClampMin = "1.0"))
	float MaxThirst = 100.f;

	/** 갈증 1이 감소하는 데 걸리는 시간 (초). 클수록 천천히 줄어든다 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Thirst", meta = (ClampMin = "0.1", Units = "s"))
	float SecondsPerThirstDecay = 1.25f;

	// ── 패널티 ──

	/** 이 비율 이하면 low 상태 (기본 30%) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Penalty", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LowThreshold = 0.3f;

	/** 갈증 low 시 초당 HP 감소량 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Penalty", meta = (ClampMin = "0.0"))
	float ThirstHPDrainPerSecond = 2.f;

	/** 배고픔 low 시 이동속도 배율 (0.7 = 30% 감소) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Penalty", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HungerSpeedPenaltyMultiplier = 0.7f;
};
