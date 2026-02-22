// Fill out your copyright notice in the Description page of Project Settings.

#include "Data/DS1ItemDataRegistry.h"

#include "Data/DS1ItemData.h"
#include "Engine/AssetManager.h"
#include "Equipments/DS1Equipment.h"

void UDS1ItemDataRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ScanAllItemData();
}

UDS1ItemData* UDS1ItemDataRegistry::FindItemDataByID(FName ItemID) const
{
	if (const auto* Found = ItemDataMap.Find(ItemID))
	{
		return *Found;
	}
	return nullptr;
}

UDS1ItemData* UDS1ItemDataRegistry::FindItemDataByEquipmentClass(TSubclassOf<ADS1Equipment> EquipmentClass) const
{
	if (!EquipmentClass)
	{
		return nullptr;
	}

	if (const auto* Found = EquipmentClassMap.Find(EquipmentClass.Get()))
	{
		return *Found;
	}
	return nullptr;
}

void UDS1ItemDataRegistry::ScanAllItemData()
{
	UAssetManager& AssetManager = UAssetManager::Get();

	TArray<FAssetData> AssetDataList;
	AssetManager.GetPrimaryAssetDataList(FPrimaryAssetType("DS1ItemData"), AssetDataList);

	for (const FAssetData& AssetData : AssetDataList)
	{
		if (UDS1ItemData* ItemData = Cast<UDS1ItemData>(AssetData.GetAsset()))
		{
			if (!ItemData->ItemID.IsNone())
			{
				ItemDataMap.Add(ItemData->ItemID, ItemData);
			}

			if (ItemData->ItemType == EDS1ItemType::Equipment && ItemData->EquipmentClass)
			{
				EquipmentClassMap.Add(ItemData->EquipmentClass.Get(), ItemData);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("DS1ItemDataRegistry: Scanned %d items"), ItemDataMap.Num());
}
