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
 * 洹몃━??湲곕컲 ?몃깽?좊━ 而댄룷?뚰듃
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

	// ?? ?몃깽?좊━ ?곗씠????
protected:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 MaxSlots = 20;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float MaxCarryWeight = 100.f;

	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	float CurrentWeight = 0.f;

	UPROPERTY()
	TArray<FDS1ItemInstance> InventorySlots;

	// ?? ?꾩씠??異붽?/?쒓굅 ??
public:
	/**
	 * ?꾩씠?쒖쓣 ?몃깽?좊━??異붽?. ?ㅽ깮 媛?ν븯硫?湲곗〈 ?щ’???⑹묠.
	 * @return ?ㅼ젣濡?異붽????섎웾
	 */
	int32 AddItem(UDS1ItemData* ItemData, int32 Count = 1);

	/**
	 * ?뱀젙 ?щ’?먯꽌 ?꾩씠?쒖쓣 ?쒓굅.
	 * @return ?ㅼ젣濡??쒓굅???섎웾
	 */
	int32 RemoveItemFromSlot(int32 SlotIndex, int32 Count = 1);

	/** ?щ’ 媛??꾩씠???대룞 */
	bool MoveItem(int32 FromSlot, int32 ToSlot);

	/** ???щ’???꾩씠??援먰솚 */
	bool SwapItems(int32 SlotA, int32 SlotB);

	/** ?꾩씠??蹂댁쑀 ?щ? */
	bool HasItem(UDS1ItemData* ItemData) const;

	/** ?뱀젙 ?꾩씠?쒖쓽 珥??섎웾 */
	int32 GetItemCount(UDS1ItemData* ItemData) const;

	/** ItemData와 일치하는 첫 슬롯 인덱스(-1이면 없음) */
	int32 FindFirstSlotByItemData(UDS1ItemData* ItemData) const;

	// ?? ?λ퉬 ?곕룞 ??
public:
	/** ?몃깽?좊━ ?щ’?먯꽌 ?λ퉬 ?μ갑 */
	bool EquipFromSlot(int32 SlotIndex);

	/** ?μ갑 以묒씤 ?λ퉬瑜??몃깽?좊━濡?諛섑솚 */
	bool UnequipToInventory(EDS1EquipSlotType SlotType);

	// ?? ?뚮퉬/?쒕∼ ??
public:
	/** ?뚮퉬 ?꾩씠???ъ슜 */
	bool UseConsumableFromSlot(int32 SlotIndex);

	/** ItemData 기반으로 소비 아이템 사용 */
	bool UseConsumableByItemData(UDS1ItemData* ItemData);

	/** ?щ’???꾩씠?쒖쓣 ?붾뱶???쒕∼ */
	bool DropItemFromSlot(int32 SlotIndex, int32 Count = 1);

	// ?? ?묎렐????
public:
	FORCEINLINE int32 GetMaxSlots() const { return MaxSlots; }
	FORCEINLINE float GetMaxCarryWeight() const { return MaxCarryWeight; }
	FORCEINLINE float GetCurrentWeight() const { return CurrentWeight; }
	FORCEINLINE const TArray<FDS1ItemInstance>& GetInventorySlots() const { return InventorySlots; }
	const FDS1ItemInstance& GetSlot(int32 Index) const;

private:
	/** 鍮??щ’ ?몃뜳??李얘린, ?놁쑝硫?-1 */
	int32 FindEmptySlot() const;

	/** 媛숈? ItemData?닿퀬 ?ㅽ깮 媛?ν븳 ?щ’ 李얘린 */
	int32 FindStackableSlot(UDS1ItemData* ItemData) const;

	/** 臾닿쾶 ?뺤씤 */
	bool CanCarryWeight(float AdditionalWeight) const;

	/** 臾닿쾶 ?ш퀎??*/
	void RecalculateWeight();

	void BroadcastInventoryChanged();
	void BroadcastSlotChanged(int32 SlotIndex);
};




