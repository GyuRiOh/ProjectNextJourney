// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifyState_DS1IFrames.h"

#include "Interfaces/DS1CombatInterface.h"

void UAnimNotifyState_DS1IFrames::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	if (IDS1CombatInterface* CombatInterface = Cast<IDS1CombatInterface>(OwnerActor))
	{
		CombatInterface->ToggleIFrames(true);
	}
}

void UAnimNotifyState_DS1IFrames::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	if (IDS1CombatInterface* CombatInterface = Cast<IDS1CombatInterface>(OwnerActor))
	{
		CombatInterface->ToggleIFrames(false);
	}

	Super::NotifyEnd(MeshComp, Animation, EventReference);
}
