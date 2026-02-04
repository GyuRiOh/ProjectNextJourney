// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifyState_DS1WeaponCollision.h"

#include "Interfaces/DS1CombatInterface.h"

UAnimNotifyState_DS1WeaponCollision::UAnimNotifyState_DS1WeaponCollision(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UAnimNotifyState_DS1WeaponCollision::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (IDS1CombatInterface* CombatInterface = Cast<IDS1CombatInterface>(OwnerActor))
		{
			CombatInterface->ActivateWeaponCollision(CollisionType);
		}
	}
}

void UAnimNotifyState_DS1WeaponCollision::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (IDS1CombatInterface* CombatInterface = Cast<IDS1CombatInterface>(OwnerActor))
		{
			CombatInterface->DeactivateWeaponCollision(CollisionType);
		}
	}
}
