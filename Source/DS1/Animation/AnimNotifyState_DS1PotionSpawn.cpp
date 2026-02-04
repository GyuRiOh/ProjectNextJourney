// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifyState_DS1PotionSpawn.h"

#include "Components/DS1PotionInventoryComponent.h"

void UAnimNotifyState_DS1PotionSpawn::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (const AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (UDS1PotionInventoryComponent* PotionInventory = OwnerActor->GetComponentByClass<UDS1PotionInventoryComponent>())
		{
			PotionInventory->SpawnPotion();
		}
	}
}

void UAnimNotifyState_DS1PotionSpawn::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (const AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (UDS1PotionInventoryComponent* PotionInventory = OwnerActor->GetComponentByClass<UDS1PotionInventoryComponent>())
		{
			PotionInventory->DespawnPotion();
		}
	}

	Super::NotifyEnd(MeshComp, Animation, EventReference);
}
