// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipments/DS1Armour.h"

#include "Components/DS1AttributeComponent.h"
#include "Components/DS1CombatComponent.h"
#include "GameFramework/Character.h"


ADS1Armour::ADS1Armour()
{
	if (USkeletalMeshComponent* SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>("ArmourMesh"))
	{
		Mesh->DestroyComponent();
		Mesh = SkeletalMesh;
		RootComponent = Mesh;
	}
}

void ADS1Armour::EquipItem()
{
	Super::EquipItem();

	if (UDS1CombatComponent* CombatComponent = GetOwner()->GetComponentByClass<UDS1CombatComponent>())
	{
		CombatComponent->SetArmour(this);

		AttachToOwner(NAME_None);

		SetBodyPartActive(false);
	}

	if (UDS1AttributeComponent* AttributeComponent = GetOwner()->GetComponentByClass<UDS1AttributeComponent>())
	{
		AttributeComponent->IncreaseDefense(DefenseAmount);
	}
}

void ADS1Armour::UnequipItem()
{
	Super::UnequipItem();

	if (UDS1AttributeComponent* AttributeComponent = GetOwner()->GetComponentByClass<UDS1AttributeComponent>())
	{
		AttributeComponent->DecreaseDefense(DefenseAmount);
	}

	if (USkeletalMeshComponent* SkeletalMesh = Cast<USkeletalMeshComponent>(Mesh))
	{
		SkeletalMesh->SetLeaderPoseComponent(nullptr);
	}

	SetBodyPartActive(true);

	// 현재 Transform 정보를 유지하면서 떼어낸다.
	DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);
}

void ADS1Armour::AttachToOwner(FName SocketName)
{
	Super::AttachToOwner(SocketName);

	if (USkeletalMeshComponent* SkeletalMesh = Cast<USkeletalMeshComponent>(Mesh))
	{
		if (const ACharacter* Character = Cast<ACharacter>(GetOwner()))
		{
			SkeletalMesh->SetLeaderPoseComponent(Character->GetMesh());
		}
	}
}

void ADS1Armour::SetBodyPartActive(const bool bActive) const
{
	if (const ADS1Character* Character = Cast<ADS1Character>(GetOwner()))
	{
		Character->SetBodyPartActive(ArmourType, bActive);
	}
}
