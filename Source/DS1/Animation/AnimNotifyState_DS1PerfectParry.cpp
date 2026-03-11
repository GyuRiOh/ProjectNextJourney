// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/AnimNotifyState_DS1PerfectParry.h"
#include "Characters/DS1Character.h"

void UAnimNotifyState_DS1PerfectParry::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (ADS1Character* Character = Cast<ADS1Character>(MeshComp->GetOwner()))
	{
		Character->SetPerfectParryWindow(true);
	}
}

void UAnimNotifyState_DS1PerfectParry::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (ADS1Character* Character = Cast<ADS1Character>(MeshComp->GetOwner()))
	{
		Character->SetPerfectParryWindow(false);
	}
}
