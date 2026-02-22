// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/DS1Interact.h"
#include "DS1PickupItem.generated.h"

class ADS1Equipment;
class UDS1ItemData;

UCLASS()
class DS1_API ADS1PickupItem : public AActor, public IDS1Interact
{
	GENERATED_BODY()

public:
	ADS1PickupItem();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void Interact(AActor* InteractionActor) override;

public:
	FORCEINLINE void SetEquipmentClass(const TSubclassOf<ADS1Equipment>& NewEquipmentClass) { EquipmentClass = NewEquipmentClass; };
	FORCEINLINE void SetItemData(UDS1ItemData* NewItemData) { ItemData = NewItemData; }
	FORCEINLINE UDS1ItemData* GetItemData() const { return ItemData; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item")
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TSubclassOf<ADS1Equipment> EquipmentClass;

	/** 아이템 데이터 (비장비 아이템 지원) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	UDS1ItemData* ItemData = nullptr;
};
