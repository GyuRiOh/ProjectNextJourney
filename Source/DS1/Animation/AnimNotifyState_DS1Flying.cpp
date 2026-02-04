// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifyState_DS1Flying.h"

#include "GameFramework/CharacterMovementComponent.h"

void UAnimNotifyState_DS1Flying::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (const AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (UCharacterMovementComponent* MovementComp = OwnerActor->GetComponentByClass<UCharacterMovementComponent>())
		{
			MovementComp->SetMovementMode(MOVE_Flying);
		}
	}
}

void UAnimNotifyState_DS1Flying::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (const AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (UCharacterMovementComponent* MovementComp = OwnerActor->GetComponentByClass<UCharacterMovementComponent>())
		{
			MovementComp->SetMovementMode(MOVE_Walking);
		}
	}

	Super::NotifyEnd(MeshComp, Animation, EventReference);
}
