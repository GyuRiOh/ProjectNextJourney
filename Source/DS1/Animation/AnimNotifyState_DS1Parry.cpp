// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifyState_DS1Parry.h"

#include "Components/DS1StateComponent.h"

void UAnimNotifyState_DS1Parry::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (const AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (UDS1StateComponent* StateComponent = OwnerActor->GetComponentByClass<UDS1StateComponent>())
		{
			StateComponent->SetState(DS1GameplayTags::Character_State_Parrying);
		}
	}
}

void UAnimNotifyState_DS1Parry::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (const AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (UDS1StateComponent* StateComponent = OwnerActor->GetComponentByClass<UDS1StateComponent>())
		{
			StateComponent->ClearState();
		}
	}
}
