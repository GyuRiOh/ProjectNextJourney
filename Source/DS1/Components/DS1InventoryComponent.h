// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DS1Define.h"
#include "Data/DS1ItemInstance.h"
#include "Components/ActorComponent.h"
#include "DS1InventoryComponent.generated.h"

class UDS1ItemData;

DECLARE_MULTICAST_DELEGATE(FOnInventoryChanged);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSlotChanged, int32 /* SlotIndex */);

/**
 * 그리드 기반 인벤토리 컴포넌트
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DS1_API UDS1InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	FOnInventoryChanged OnInventoryChanged;
	FOnSlotChanged OnSlotChanged;

public:
	UDS1InventoryComponent();

protected:
	virtual void BeginPlay() override;

	// ── 인벤토리 데이터 ──
protected:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 MaxSlots = 20;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float MaxCarryWeight = 100.f;

	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	float CurrentWeight = 0.f;

	UPROPERTY()
	TArray<FDS1ItemInstance> InventorySlots;

	// ── 아이템 추가/제거 ──
public:
	/**
	 * 아이템을 인벤토리에 추가. 스택 가능하면 기존 슬롯에 합침.
	 * @return 실제로 추가된 수량
	 */
	int32 AddItem(UDS1ItemData* ItemData, int32 Count = 1);

	/**
	 * 특정 슬롯에서 아이템을 제거.
	 * @return 실제로 제거된 수량
	 */
	int32 RemoveItemFromSlot(int32 SlotIndex, int32 Count = 1);

	/** 슬롯 간 아이템 이동 */
	bool MoveItem(int32 FromSlot, int32 ToSlot);

	/** 두 슬롯의 아이템 교환 */
	bool SwapItems(int32 SlotA, int32 SlotB);

	/** 아이템 보유 여부 */
	bool HasItem(UDS1ItemData* ItemData) const;

	/** 특정 아이템의 총 수량 */
	int32 GetItemCount(UDS1ItemData* ItemData) const;

	// ── 장비 연동 ──
public:
	/** 인벤토리 슬롯에서 장비 장착 */
	bool EquipFromSlot(int32 SlotIndex);

	/** 장착 중인 장비를 인벤토리로 반환 */
	bool UnequipToInventory(EDS1EquipSlotType SlotType);

	// ── 소비/드롭 ──
public:
	/** 소비 아이템 사용 */
	bool UseConsumableFromSlot(int32 SlotIndex);

	/** 슬롯의 아이템을 월드에 드롭 */
	bool DropItemFromSlot(int32 SlotIndex, int32 Count = 1);

	// ── 접근자 ──
public:
	FORCEINLINE int32 GetMaxSlots() const { return MaxSlots; }
	FORCEINLINE float GetMaxCarryWeight() const { return MaxCarryWeight; }
	FORCEINLINE float GetCurrentWeight() const { return CurrentWeight; }
	FORCEINLINE const TArray<FDS1ItemInstance>& GetInventorySlots() const { return InventorySlots; }
	const FDS1ItemInstance& GetSlot(int32 Index) const;

private:
	/** 빈 슬롯 인덱스 찾기, 없으면 -1 */
	int32 FindEmptySlot() const;

	/** 같은 ItemData이고 스택 가능한 슬롯 찾기 */
	int32 FindStackableSlot(UDS1ItemData* ItemData) const;

	/** 무게 확인 */
	bool CanCarryWeight(float AdditionalWeight) const;

	/** 무게 재계산 */
	void RecalculateWeight();

	void BroadcastInventoryChanged();
	void BroadcastSlotChanged(int32 SlotIndex);
};
