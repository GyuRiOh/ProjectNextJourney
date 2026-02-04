// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipments/DS1Equipment.h"

#include "GameFramework/Character.h"

// Sets default values
ADS1Equipment::ADS1Equipment()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EquipmentMesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void ADS1Equipment::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADS1Equipment::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADS1Equipment::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (UStaticMeshComponent* StaticMesh = Cast<UStaticMeshComponent>(Mesh))
	{
		StaticMesh->SetStaticMesh(MeshAsset);
	}
}

void ADS1Equipment::EquipItem()
{
}

void ADS1Equipment::UnequipItem()
{
}

void ADS1Equipment::AttachToOwner(FName SocketName)
{
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (USkeletalMeshComponent* CharacterMesh = OwnerCharacter->GetMesh())
		{
			AttachToComponent(CharacterMesh, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), SocketName);
		}
	}
}

