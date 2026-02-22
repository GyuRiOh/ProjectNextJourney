// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Data/DS1ItemInstance.h"
#include "DS1InventoryDragDropOp.generated.h"

/**
 * 인벤토리 드래그 앤 드롭 오퍼레이션
 */
UCLASS()
class DS1_API UDS1InventoryDragDropOp : public UDragDropOperation
{
	GENERATED_BODY()

public:
	/** 원본 슬롯 인덱스 (-1 = 장비 슬롯) */
	UPROPERTY()
	int32 SourceSlotIndex = -1;

	/** 장비 슬롯에서 드래그했는지 */
	UPROPERTY()
	bool bFromEquipSlot = false;

	/** 장비 슬롯 유형 (장비 슬롯에서 드래그한 경우) */
	UPROPERTY()
	EDS1EquipSlotType EquipSlotType = EDS1EquipSlotType::Weapon;

	/** 드래그 중인 아이템 정보 */
	UPROPERTY()
	FDS1ItemInstance DraggedItem;
};
