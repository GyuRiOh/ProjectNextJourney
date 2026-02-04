// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifyState_DS1RotateTo.h"

#include "Components/DS1RotationComponent.h"

void UAnimNotifyState_DS1RotateTo::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (UDS1RotationComponent* RotationComponent = MeshComp->GetOwner()->GetComponentByClass<UDS1RotationComponent>())
	{
		RotationComponent->ToggleShouldRotate(true);
	}
}

void UAnimNotifyState_DS1RotateTo::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (UDS1RotationComponent* RotationComponent = MeshComp->GetOwner()->GetComponentByClass<UDS1RotationComponent>())
	{
		RotationComponent->ToggleShouldRotate(false);
	}
}
