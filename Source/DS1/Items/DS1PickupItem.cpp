// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/DS1PickupItem.h"

#include "DS1Define.h"
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

	// 설정된 장비(무기)아이템의 클래스 정보를 이용해서 PickupItem의 외형을 설정.
	if (EquipmentClass)
	{
		if (ADS1Equipment* CDO = EquipmentClass->GetDefaultObject<ADS1Equipment>())
		{
			Mesh->SetStaticMesh(CDO->MeshAsset);
			Mesh->SetSimulatePhysics(true);
		}
	}
}

void ADS1PickupItem::Interact(AActor* InteractionActor)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = InteractionActor;

	// 장비 아이템을 Spawn하고 캐릭터의 Socket에 장착.
	ADS1Equipment* SpawnItem = GetWorld()->SpawnActor<ADS1Equipment>(EquipmentClass, GetActorTransform(), SpawnParams);
	if (SpawnItem)
	{
		SpawnItem->EquipItem();
		Destroy();
	}
}

