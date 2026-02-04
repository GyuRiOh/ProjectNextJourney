// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify_DS1PotionDrink.h"

#include "Components/DS1PotionInventoryComponent.h"

void UAnimNotify_DS1PotionDrink::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (const AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (UDS1PotionInventoryComponent* PotionInventory = OwnerActor->GetComponentByClass<UDS1PotionInventoryComponent>())
		{
			PotionInventory->DrinkPotion();
		}
	}
}
