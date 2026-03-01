// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/DS1InventoryComponent.h"

#include "Data/DS1ItemData.h"
#include "Data/DS1ItemDataRegistry.h"
#include "Components/DS1CombatComponent.h"
#include "Components/DS1AttributeComponent.h"
#include "Equipments/DS1Equipment.h"
#include "Equipments/DS1Weapon.h"
#include "Equipments/DS1Shield.h"
#include "Equipments/DS1Armour.h"
#include "Items/DS1PickupItem.h"

UDS1InventoryComponent::UDS1InventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDS1InventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	InventorySlots.SetNum(MaxSlots);
}

// ── 아이템 추가 ──

int32 UDS1InventoryComponent::AddItem(UDS1ItemData* ItemData, int32 Count)
{
	if (!ItemData || Count <= 0)
	{
		return 0;
	}

	const float TotalWeight = ItemData->Weight * Count;
	if (!CanCarryWeight(TotalWeight))
	{
		return 0;
	}

	int32 Remaining = Count;

	// 스택 가능한 기존 슬롯에 먼저 합침
	if (ItemData->MaxStackCount > 1)
	{
		for (int32 i = 0; i < InventorySlots.Num() && Remaining > 0; ++i)
		{
			FDS1ItemInstance& Slot = InventorySlots[i];
			if (Slot.ItemData == ItemData && Slot.StackCount < ItemData->MaxStackCount)
			{
				const int32 Space = ItemData->MaxStackCount - Slot.StackCount;
				const int32 ToAdd = FMath::Min(Remaining, Space);
				Slot.StackCount += ToAdd;
				Remaining -= ToAdd;
				BroadcastSlotChanged(i);
			}
		}
	}

	// 남은 수량은 빈 슬롯에 배치
	while (Remaining > 0)
	{
		const int32 EmptySlot = FindEmptySlot();
		if (EmptySlot == -1)
		{
			break;
		}

		const int32 ToAdd = FMath::Min(Remaining, ItemData->MaxStackCount);
		InventorySlots[EmptySlot].ItemData = ItemData;
		InventorySlots[EmptySlot].StackCount = ToAdd;
		Remaining -= ToAdd;
		BroadcastSlotChanged(EmptySlot);
	}

	const int32 Added = Count - Remaining;
	if (Added > 0)
	{
		CurrentWeight += ItemData->Weight * Added;
		BroadcastInventoryChanged();
	}
	return Added;
}

int32 UDS1InventoryComponent::RemoveItemFromSlot(int32 SlotIndex, int32 Count)
{
	if (!InventorySlots.IsValidIndex(SlotIndex) || Count <= 0)
	{
		return 0;
	}

	FDS1ItemInstance& Slot = InventorySlots[SlotIndex];
	if (!Slot.IsValid())
	{
		return 0;
	}

	const int32 ToRemove = FMath::Min(Count, Slot.StackCount);
	const float WeightRemoved = Slot.ItemData->Weight * ToRemove;

	Slot.StackCount -= ToRemove;
	if (Slot.StackCount <= 0)
	{
		Slot.Clear();
	}

	CurrentWeight -= WeightRemoved;
	CurrentWeight = FMath::Max(0.f, CurrentWeight);

	BroadcastSlotChanged(SlotIndex);
	BroadcastInventoryChanged();
	return ToRemove;
}

bool UDS1InventoryComponent::MoveItem(int32 FromSlot, int32 ToSlot)
{
	if (FromSlot == ToSlot)
	{
		return false;
	}

	if (!InventorySlots.IsValidIndex(FromSlot) || !InventorySlots.IsValidIndex(ToSlot))
	{
		return false;
	}

	FDS1ItemInstance& From = InventorySlots[FromSlot];
	FDS1ItemInstance& To = InventorySlots[ToSlot];

	if (!From.IsValid())
	{
		return false;
	}

	// 대상 슬롯이 비어있으면 이동
	if (!To.IsValid())
	{
		To = From;
		From.Clear();
		BroadcastSlotChanged(FromSlot);
		BroadcastSlotChanged(ToSlot);
		BroadcastInventoryChanged();
		return true;
	}

	// 같은 아이템이면 스택 합치기 시도
	if (To.ItemData == From.ItemData && To.ItemData->MaxStackCount > 1)
	{
		const int32 Space = To.ItemData->MaxStackCount - To.StackCount;
		const int32 ToMove = FMath::Min(From.StackCount, Space);
		if (ToMove > 0)
		{
			To.StackCount += ToMove;
			From.StackCount -= ToMove;
			if (From.StackCount <= 0)
			{
				From.Clear();
			}
			BroadcastSlotChanged(FromSlot);
			BroadcastSlotChanged(ToSlot);
			BroadcastInventoryChanged();
			return true;
		}
	}

	// 교환
	return SwapItems(FromSlot, ToSlot);
}

bool UDS1InventoryComponent::SwapItems(int32 SlotA, int32 SlotB)
{
	if (SlotA == SlotB)
	{
		return false;
	}

	if (!InventorySlots.IsValidIndex(SlotA) || !InventorySlots.IsValidIndex(SlotB))
	{
		return false;
	}

	Swap(InventorySlots[SlotA], InventorySlots[SlotB]);
	BroadcastSlotChanged(SlotA);
	BroadcastSlotChanged(SlotB);
	BroadcastInventoryChanged();
	return true;
}

bool UDS1InventoryComponent::HasItem(UDS1ItemData* ItemData) const
{
	return GetItemCount(ItemData) > 0;
}

int32 UDS1InventoryComponent::GetItemCount(UDS1ItemData* ItemData) const
{
	int32 Total = 0;
	for (const FDS1ItemInstance& Slot : InventorySlots)
	{
		if (Slot.ItemData == ItemData)
		{
			Total += Slot.StackCount;
		}
	}
	return Total;
}

// ── 장비 연동 ──

bool UDS1InventoryComponent::EquipFromSlot(int32 SlotIndex)
{
	if (!InventorySlots.IsValidIndex(SlotIndex))
	{
		return false;
	}

	const FDS1ItemInstance& Slot = InventorySlots[SlotIndex];
	if (!Slot.IsValid() || Slot.ItemData->ItemType != EDS1ItemType::Equipment)
	{
		return false;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return false;
	}

	TSubclassOf<ADS1Equipment> EquipClass = Slot.ItemData->EquipmentClass;
	if (!EquipClass)
	{
		return false;
	}

	// 장비 스폰 & 장착
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerActor;
	ADS1Equipment* SpawnedEquip = GetWorld()->SpawnActor<ADS1Equipment>(EquipClass, OwnerActor->GetActorTransform(), SpawnParams);
	if (!SpawnedEquip)
	{
		return false;
	}

	SpawnedEquip->EquipItem();

	// 장착이 성공한 뒤에만 인벤토리에서 제거한다.
	if (RemoveItemFromSlot(SlotIndex, 1) <= 0)
	{
		SpawnedEquip->Destroy();
		return false;
	}

	return true;
}

bool UDS1InventoryComponent::UnequipToInventory(EDS1EquipSlotType SlotType)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return false;
	}

	UDS1CombatComponent* CombatComp = OwnerActor->FindComponentByClass<UDS1CombatComponent>();
	if (!CombatComp)
	{
		return false;
	}

	// 현재 장착 중인 장비를 가져옴
	ADS1Equipment* EquippedItem = nullptr;
	TSubclassOf<ADS1Equipment> EquipClass = nullptr;

	switch (SlotType)
	{
	case EDS1EquipSlotType::Weapon:
		if (ADS1Weapon* Weapon = CombatComp->GetMainWeapon())
		{
			EquippedItem = Weapon;
			EquipClass = Weapon->GetClass();
		}
		break;
	case EDS1EquipSlotType::Shield:
		if (ADS1Shield* Shield = CombatComp->GetShield())
		{
			EquippedItem = Shield;
			EquipClass = Shield->GetClass();
		}
		break;
	case EDS1EquipSlotType::Chest:
		if (ADS1Armour* Armour = CombatComp->GetArmour(EDS1ArmourType::Chest))
		{
			EquippedItem = Armour;
			EquipClass = Armour->GetClass();
		}
		break;
	case EDS1EquipSlotType::Pants:
		if (ADS1Armour* Armour = CombatComp->GetArmour(EDS1ArmourType::Pants))
		{
			EquippedItem = Armour;
			EquipClass = Armour->GetClass();
		}
		break;
	case EDS1EquipSlotType::Boots:
		if (ADS1Armour* Armour = CombatComp->GetArmour(EDS1ArmourType::Boots))
		{
			EquippedItem = Armour;
			EquipClass = Armour->GetClass();
		}
		break;
	case EDS1EquipSlotType::Gloves:
		if (ADS1Armour* Armour = CombatComp->GetArmour(EDS1ArmourType::Gloves))
		{
			EquippedItem = Armour;
			EquipClass = Armour->GetClass();
		}
		break;
	}

	if (!EquipClass || !EquippedItem)
	{
		return false;
	}

	// ItemDataRegistry에서 해당 장비의 ItemData 조회
	UDS1ItemDataRegistry* Registry = GetWorld()->GetGameInstance()->GetSubsystem<UDS1ItemDataRegistry>();
	if (!Registry)
	{
		return false;
	}

	UDS1ItemData* ItemData = Registry->FindItemDataByEquipmentClass(EquipClass);
	if (!ItemData)
	{
		return false;
	}

	// 인벤토리에 공간 확인
	if (FindEmptySlot() == -1 || !CanCarryWeight(ItemData->Weight))
	{
		return false;
	}

	if (AddItem(ItemData, 1) <= 0)
	{
		return false;
	}

	// 전투 컴포넌트 레퍼런스 정리 + 실제 장착 해제
	switch (SlotType)
	{
	case EDS1EquipSlotType::Weapon:
		CombatComp->ClearWeapon();
		break;
	case EDS1EquipSlotType::Shield:
		CombatComp->ClearShield();
		break;
	case EDS1EquipSlotType::Chest:
		CombatComp->ClearArmour(EDS1ArmourType::Chest);
		break;
	case EDS1EquipSlotType::Pants:
		CombatComp->ClearArmour(EDS1ArmourType::Pants);
		break;
	case EDS1EquipSlotType::Boots:
		CombatComp->ClearArmour(EDS1ArmourType::Boots);
		break;
	case EDS1EquipSlotType::Gloves:
		CombatComp->ClearArmour(EDS1ArmourType::Gloves);
		break;
	}

	EquippedItem->UnequipItem();
	EquippedItem->Destroy();

	return true;
}

// ── 소비/드롭 ──

bool UDS1InventoryComponent::UseConsumableFromSlot(int32 SlotIndex)
{
	if (!InventorySlots.IsValidIndex(SlotIndex))
	{
		return false;
	}

	const FDS1ItemInstance& Slot = InventorySlots[SlotIndex];
	if (!Slot.IsValid() || Slot.ItemData->ItemType != EDS1ItemType::Consumable)
	{
		return false;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return false;
	}

	// 체력 회복 적용
	if (UDS1AttributeComponent* AttribComp = OwnerActor->FindComponentByClass<UDS1AttributeComponent>())
	{
		AttribComp->HealPlayer(Slot.ItemData->EffectValue);
	}

	RemoveItemFromSlot(SlotIndex, 1);
	return true;
}

bool UDS1InventoryComponent::DropItemFromSlot(int32 SlotIndex, int32 Count)
{
	if (!InventorySlots.IsValidIndex(SlotIndex))
	{
		return false;
	}

	FDS1ItemInstance& Slot = InventorySlots[SlotIndex];
	if (!Slot.IsValid())
	{
		return false;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return false;
	}

	const int32 ToDrop = FMath::Min(Count, Slot.StackCount);
	UDS1ItemData* ItemData = Slot.ItemData;

	// 장비 아이템이면 EquipmentClass 기반 PickupItem 생성
	if (ItemData->ItemType == EDS1ItemType::Equipment && ItemData->EquipmentClass)
	{
		for (int32 i = 0; i < ToDrop; ++i)
		{
			ADS1PickupItem* Pickup = GetWorld()->SpawnActorDeferred<ADS1PickupItem>(
				ADS1PickupItem::StaticClass(),
				OwnerActor->GetActorTransform(),
				nullptr, nullptr,
				ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
			Pickup->SetEquipmentClass(ItemData->EquipmentClass);
			Pickup->SetItemData(ItemData);
			Pickup->FinishSpawning(OwnerActor->GetActorTransform());
		}
	}
	else
	{
		// 비장비 아이템 — ItemData 기반 PickupItem 생성
		for (int32 i = 0; i < ToDrop; ++i)
		{
			ADS1PickupItem* Pickup = GetWorld()->SpawnActorDeferred<ADS1PickupItem>(
				ADS1PickupItem::StaticClass(),
				OwnerActor->GetActorTransform(),
				nullptr, nullptr,
				ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
			Pickup->SetItemData(ItemData);
			Pickup->FinishSpawning(OwnerActor->GetActorTransform());
		}
	}

	RemoveItemFromSlot(SlotIndex, ToDrop);
	return true;
}

// ── 접근자 ──

const FDS1ItemInstance& UDS1InventoryComponent::GetSlot(int32 Index) const
{
	check(InventorySlots.IsValidIndex(Index));
	return InventorySlots[Index];
}

// ── 유틸리티 ──

int32 UDS1InventoryComponent::FindEmptySlot() const
{
	for (int32 i = 0; i < InventorySlots.Num(); ++i)
	{
		if (!InventorySlots[i].IsValid())
		{
			return i;
		}
	}
	return -1;
}

int32 UDS1InventoryComponent::FindStackableSlot(UDS1ItemData* ItemData) const
{
	if (!ItemData || ItemData->MaxStackCount <= 1)
	{
		return -1;
	}

	for (int32 i = 0; i < InventorySlots.Num(); ++i)
	{
		const FDS1ItemInstance& Slot = InventorySlots[i];
		if (Slot.ItemData == ItemData && Slot.StackCount < ItemData->MaxStackCount)
		{
			return i;
		}
	}
	return -1;
}

bool UDS1InventoryComponent::CanCarryWeight(float AdditionalWeight) const
{
	return (CurrentWeight + AdditionalWeight) <= MaxCarryWeight;
}

void UDS1InventoryComponent::RecalculateWeight()
{
	CurrentWeight = 0.f;
	for (const FDS1ItemInstance& Slot : InventorySlots)
	{
		if (Slot.IsValid())
		{
			CurrentWeight += Slot.ItemData->Weight * Slot.StackCount;
		}
	}
}

void UDS1InventoryComponent::BroadcastInventoryChanged()
{
	if (OnInventoryChanged.IsBound())
	{
		OnInventoryChanged.Broadcast();
	}
}

void UDS1InventoryComponent::BroadcastSlotChanged(int32 SlotIndex)
{
	if (OnSlotChanged.IsBound())
	{
		OnSlotChanged.Broadcast(SlotIndex);
	}
}
