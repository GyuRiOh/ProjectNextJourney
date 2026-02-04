// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/DS1PotionInventoryComponent.h"

#include "DS1AttributeComponent.h"
#include "GameFramework/Character.h"
#include "Items/DS1Potion.h"

UDS1PotionInventoryComponent::UDS1PotionInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDS1PotionInventoryComponent::DrinkPotion()
{
	if (PotionQuantity <= 0)
	{
		return;
	}

	if (UDS1AttributeComponent* Attribute = GetOwner()->GetComponentByClass<UDS1AttributeComponent>())
	{
		PotionQuantity--;
		Attribute->HealPlayer(PotionHealAmount);

		BroadcastPotionUpdate();
	}
}

void UDS1PotionInventoryComponent::SpawnPotion()
{
	PotionActor = GetWorld()->SpawnActor<ADS1Potion>(PotionClass, GetOwner()->GetActorTransform());
	if (PotionActor)
	{
		if (const ACharacter* Character = Cast<ACharacter>(GetOwner()))
		{
			PotionActor->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), PotionSocketName);
		}
	}
}

void UDS1PotionInventoryComponent::DespawnPotion()
{
	if (IsValid(PotionActor))
	{
		PotionActor->Destroy();
	}
}

void UDS1PotionInventoryComponent::SetPointQuantity(int32 InQuantity)
{
	PotionQuantity = InQuantity;
	BroadcastPotionUpdate();
}

void UDS1PotionInventoryComponent::BroadcastPotionUpdate() const
{
	if (OnUpdatePotionAmount.IsBound())
	{
		OnUpdatePotionAmount.Broadcast(PotionQuantity);
	}
}

