// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/DS1QuickSlotComponent.h"

#include "Components/DS1InventoryComponent.h"
#include "Data/DS1ItemData.h"

UDS1QuickSlotComponent::UDS1QuickSlotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDS1QuickSlotComponent::BeginPlay()
{
	Super::BeginPlay();

	QuickSlots.SetNum(MaxQuickSlots);

	InventoryComponent = GetOwner()->FindComponentByClass<UDS1InventoryComponent>();
	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryChanged.AddUObject(this, &ThisClass::HandleInventoryChanged);
	}
}

bool UDS1QuickSlotComponent::RegisterSlot(int32 QuickSlotIndex, UDS1ItemData* ItemData)
{
	if (!IsValidQuickSlotIndex(QuickSlotIndex) || !InventoryComponent || !ItemData)
	{
		return false;
	}

	if (InventoryComponent->GetItemCount(ItemData) <= 0)
	{
		return false;
	}

	QuickSlots[QuickSlotIndex].ItemData = ItemData;
	BroadcastQuickSlotChanged();
	return true;
}

bool UDS1QuickSlotComponent::RegisterSlotFromInventorySlot(int32 QuickSlotIndex, int32 InventorySlotIndex)
{
	if (!IsValidQuickSlotIndex(QuickSlotIndex) || !InventoryComponent)
	{
		return false;
	}

	if (!InventoryComponent->GetInventorySlots().IsValidIndex(InventorySlotIndex))
	{
		return false;
	}

	const FDS1ItemInstance& Slot = InventoryComponent->GetSlot(InventorySlotIndex);
	if (!Slot.IsValid())
	{
		return false;
	}

	return RegisterSlot(QuickSlotIndex, Slot.ItemData);
}

bool UDS1QuickSlotComponent::RegisterToFirstEmptySlot(int32 InventorySlotIndex)
{
	if (!InventoryComponent || !InventoryComponent->GetInventorySlots().IsValidIndex(InventorySlotIndex))
	{
		return false;
	}

	const FDS1ItemInstance& Slot = InventoryComponent->GetSlot(InventorySlotIndex);
	if (!Slot.IsValid())
	{
		return false;
	}

	for (int32 i = 0; i < QuickSlots.Num(); ++i)
	{
		if (!QuickSlots[i].ItemData)
		{
			return RegisterSlot(i, Slot.ItemData);
		}
	}

	return false;
}

void UDS1QuickSlotComponent::ClearSlot(int32 QuickSlotIndex)
{
	if (!IsValidQuickSlotIndex(QuickSlotIndex))
	{
		return;
	}

	QuickSlots[QuickSlotIndex].ItemData = nullptr;
	BroadcastQuickSlotChanged();
}

bool UDS1QuickSlotComponent::UseQuickSlot(int32 QuickSlotIndex)
{
	if (!IsValidQuickSlotIndex(QuickSlotIndex) || !InventoryComponent)
	{
		return false;
	}

	UDS1ItemData* ItemData = QuickSlots[QuickSlotIndex].ItemData;
	if (!ItemData)
	{
		return false;
	}

	bool bUsed = false;
	switch (ItemData->ItemType)
	{
	case EDS1ItemType::Consumable:
		bUsed = InventoryComponent->UseConsumableByItemData(ItemData);
		break;
	case EDS1ItemType::Equipment:
	{
		const int32 InventorySlotIndex = InventoryComponent->FindFirstSlotByItemData(ItemData);
		bUsed = InventorySlotIndex != -1 && InventoryComponent->EquipFromSlot(InventorySlotIndex);
		break;
	}
	default:
		break;
	}

	if (InventoryComponent->GetItemCount(ItemData) <= 0)
	{
		QuickSlots[QuickSlotIndex].ItemData = nullptr;
	}

	BroadcastQuickSlotChanged();
	return bUsed;
}

UDS1ItemData* UDS1QuickSlotComponent::GetItemDataAt(int32 QuickSlotIndex) const
{
	if (!IsValidQuickSlotIndex(QuickSlotIndex))
	{
		return nullptr;
	}

	return QuickSlots[QuickSlotIndex].ItemData;
}

int32 UDS1QuickSlotComponent::GetItemCountAt(int32 QuickSlotIndex) const
{
	if (!IsValidQuickSlotIndex(QuickSlotIndex) || !InventoryComponent)
	{
		return 0;
	}

	const UDS1ItemData* ItemData = QuickSlots[QuickSlotIndex].ItemData;
	if (!ItemData)
	{
		return 0;
	}

	return InventoryComponent->GetItemCount(const_cast<UDS1ItemData*>(ItemData));
}

void UDS1QuickSlotComponent::HandleInventoryChanged()
{
	if (!InventoryComponent)
	{
		return;
	}

	for (FDS1QuickSlotEntry& Entry : QuickSlots)
	{
		if (Entry.ItemData && InventoryComponent->GetItemCount(Entry.ItemData) <= 0)
		{
			Entry.ItemData = nullptr;
		}
	}

	BroadcastQuickSlotChanged();
}

void UDS1QuickSlotComponent::BroadcastQuickSlotChanged() const
{
	if (OnQuickSlotChanged.IsBound())
	{
		OnQuickSlotChanged.Broadcast();
	}
}

bool UDS1QuickSlotComponent::IsValidQuickSlotIndex(int32 QuickSlotIndex) const
{
	return QuickSlots.IsValidIndex(QuickSlotIndex);
}

