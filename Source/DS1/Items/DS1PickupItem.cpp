// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/DS1PickupItem.h"

#include "DS1Define.h"
#include "Components/DS1InventoryComponent.h"
#include "Data/DS1ItemData.h"
#include "Data/DS1ItemDataRegistry.h"
#include "Equipments/DS1Equipment.h"

ADS1PickupItem::ADS1PickupItem()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupItemMesh"));
	SetRootComponent(Mesh);

	// Collision 설정.
	Mesh->SetCollisionObjectType(COLLISION_OBJECT_INTERACTION);
	Mesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ADS1PickupItem::BeginPlay()
{
	Super::BeginPlay();

}

void ADS1PickupItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADS1PickupItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// 장비 아이템: EquipmentClass CDO에서 메시 가져오기
	if (EquipmentClass)
	{
		if (ADS1Equipment* CDO = EquipmentClass->GetDefaultObject<ADS1Equipment>())
		{
			Mesh->SetStaticMesh(CDO->MeshAsset);
			Mesh->SetSimulatePhysics(true);
		}
	}
	// 비장비 아이템: ItemData의 WorldMesh 사용
	else if (ItemData && ItemData->WorldMesh)
	{
		Mesh->SetStaticMesh(ItemData->WorldMesh);
		Mesh->SetSimulatePhysics(true);
	}
}

void ADS1PickupItem::Interact(AActor* InteractionActor)
{
	if (!InteractionActor)
	{
		return;
	}

	// 인벤토리 컴포넌트가 있으면 인벤토리에 추가 시도
	if (UDS1InventoryComponent* InvComp = InteractionActor->FindComponentByClass<UDS1InventoryComponent>())
	{
		// ItemData가 설정되어 있으면 그것을 사용
		UDS1ItemData* DataToAdd = ItemData;

		// ItemData가 없고 EquipmentClass가 있으면 Registry에서 조회
		if (!DataToAdd && EquipmentClass)
		{
			if (UDS1ItemDataRegistry* Registry = GetWorld()->GetGameInstance()->GetSubsystem<UDS1ItemDataRegistry>())
			{
				DataToAdd = Registry->FindItemDataByEquipmentClass(EquipmentClass);
			}
		}

		if (DataToAdd)
		{
			const int32 Added = InvComp->AddItem(DataToAdd, 1);
			if (Added > 0)
			{
				Destroy();
				return;
			}
			// 인벤토리 꽉 참 - 장비 아이템이면 기존 방식으로 즉시 장착 시도
		}
	}

	// 기존 동작: 장비 아이템이면 직접 스폰 후 장착
	if (EquipmentClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = InteractionActor;

		ADS1Equipment* SpawnItem = GetWorld()->SpawnActor<ADS1Equipment>(EquipmentClass, GetActorTransform(), SpawnParams);
		if (SpawnItem)
		{
			SpawnItem->EquipItem();
			Destroy();
		}
	}
}
