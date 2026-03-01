// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DS1QuickSlotComponent.generated.h"

class UDS1InventoryComponent;
class UDS1ItemData;

DECLARE_MULTICAST_DELEGATE(FOnQuickSlotChanged);

USTRUCT(BlueprintType)
struct FDS1QuickSlotEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UDS1ItemData* ItemData = nullptr;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DS1_API UDS1QuickSlotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	FOnQuickSlotChanged OnQuickSlotChanged;

public:
	UDS1QuickSlotComponent();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category = "QuickSlot")
	int32 MaxQuickSlots = 6;

	UPROPERTY(VisibleAnywhere, Category = "QuickSlot")
	TArray<FDS1QuickSlotEntry> QuickSlots;

	UPROPERTY()
	UDS1InventoryComponent* InventoryComponent = nullptr;

public:
	int32 GetMaxQuickSlots() const { return MaxQuickSlots; }
	const TArray<FDS1QuickSlotEntry>& GetQuickSlots() const { return QuickSlots; }

	bool RegisterSlot(int32 QuickSlotIndex, UDS1ItemData* ItemData);
	bool RegisterSlotFromInventorySlot(int32 QuickSlotIndex, int32 InventorySlotIndex);
	bool RegisterToFirstEmptySlot(int32 InventorySlotIndex);
	void ClearSlot(int32 QuickSlotIndex);
	bool UseQuickSlot(int32 QuickSlotIndex);

	UDS1ItemData* GetItemDataAt(int32 QuickSlotIndex) const;
	int32 GetItemCountAt(int32 QuickSlotIndex) const;

private:
	UFUNCTION()
	void HandleInventoryChanged();

	void BroadcastQuickSlotChanged() const;
	bool IsValidQuickSlotIndex(int32 QuickSlotIndex) const;
};

