// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DS1ItemDataRegistry.generated.h"

class UDS1ItemData;
class ADS1Equipment;

/**
 * 모든 UDS1ItemData 에셋을 자동 수집하여 조회를 제공하는 서브시스템
 */
UCLASS()
class DS1_API UDS1ItemDataRegistry : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** ItemID로 ItemData 조회 */
	UDS1ItemData* FindItemDataByID(FName ItemID) const;

	/** EquipmentClass로 ItemData 역방향 조회 */
	UDS1ItemData* FindItemDataByEquipmentClass(TSubclassOf<ADS1Equipment> EquipmentClass) const;

private:
	void ScanAllItemData();

	UPROPERTY()
	TMap<FName, UDS1ItemData*> ItemDataMap;

	UPROPERTY()
	TMap<UClass*, UDS1ItemData*> EquipmentClassMap;
};
