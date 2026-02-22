// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DS1Define.h"
#include "Engine/DataAsset.h"
#include "Equipments/DS1Equipment.h"
#include "DS1ItemData.generated.h"

/**
 * 아이템 데이터 에셋 — 모든 아이템(장비/소비/기타)의 공통 정보를 담는다.
 */
UCLASS(BlueprintType)
class DS1_API UDS1ItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 고유 아이템 ID */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FName ItemID;

	/** 표시 이름 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	/** 설명 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText Description;

	/** 인벤토리 아이콘 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	UTexture2D* Icon = nullptr;

	/** 아이템 유형 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	EDS1ItemType ItemType = EDS1ItemType::Misc;

	/** 무게 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	float Weight = 1.f;

	/** 최대 스택 수 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	int32 MaxStackCount = 1;

	// ── 장비 전용 ──

	/** 장비 클래스 (Equipment, Weapon, Shield, Armour) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item | Equipment", meta = (EditCondition = "ItemType == EDS1ItemType::Equipment"))
	TSubclassOf<ADS1Equipment> EquipmentClass;

	/** 장비 슬롯 유형 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item | Equipment", meta = (EditCondition = "ItemType == EDS1ItemType::Equipment"))
	EDS1EquipSlotType EquipSlotType = EDS1EquipSlotType::Weapon;

	// ── 소비 전용 ──

	/** 소비 아이템 효과값 (회복량 등) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item | Consumable", meta = (EditCondition = "ItemType == EDS1ItemType::Consumable"))
	float EffectValue = 0.f;

	// ── 월드 드롭 ──

	/** 바닥에 드롭 시 사용할 메시 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item | World")
	UStaticMesh* WorldMesh = nullptr;

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("DS1ItemData", ItemID);
	}
};
