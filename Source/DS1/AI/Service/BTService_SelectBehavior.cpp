// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Service/BTService_SelectBehavior.h"

#include "AIController.h"
#include "DS1GameplayTags.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/DS1Enemy.h"
#include "Components/DS1StateComponent.h"

UBTService_SelectBehavior::UBTService_SelectBehavior()
{
	INIT_SERVICE_NODE_NOTIFY_FLAGS();
}

void UBTService_SelectBehavior::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
}

void UBTService_SelectBehavior::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	if (APawn* ControlledPawn = OwnerComp.GetAIOwner()->GetPawn())
	{
		if (ADS1Enemy* ControlledEnemy = Cast<ADS1Enemy>(ControlledPawn))
		{
			UpdateBehavior(OwnerComp.GetBlackboardComponent(), ControlledEnemy);
		}
	}
}

void UBTService_SelectBehavior::SetBehaviorKey(UBlackboardComponent* BlackboardComp, EDS1AIBehavior Behavior) const
{
	BlackboardComp->SetValueAsEnum(BehaviorKey.SelectedKeyName, static_cast<uint8>(Behavior));
}

void UBTService_SelectBehavior::UpdateBehavior(UBlackboardComponent* BlackboardComp, ADS1Enemy* ControlledEnemy) const
{
	check(BlackboardComp);
	check(ControlledEnemy);

	const UDS1StateComponent* StateComponent = ControlledEnemy->GetComponentByClass<UDS1StateComponent>();
	check(StateComponent);

	FGameplayTagContainer CheckTags;
	CheckTags.AddTag(DS1GameplayTags::Character_State_Parried);
	CheckTags.AddTag(DS1GameplayTags::Character_State_Stunned);

	// 스턴
	if (StateComponent->IsCurrentStateEqualToAny(CheckTags))
	{
		SetBehaviorKey(BlackboardComp, EDS1AIBehavior::Stunned);
	}
	else
	{
		AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));

		if (IsValid(TargetActor))
		{
			const float Distance = TargetActor->GetDistanceTo(ControlledEnemy);

			// 공격범위 안쪽이면
			if (Distance <= AttackRangeDistance)
			{
				SetBehaviorKey(BlackboardComp, EDS1AIBehavior::MeleeAttack);
			}
			else
			{
				SetBehaviorKey(BlackboardComp, EDS1AIBehavior::Approach);
			}
		}
		else
		{
			// Patrol point 있으면
			if (ControlledEnemy->GetPatrolPoint() != nullptr)
			{
				SetBehaviorKey(BlackboardComp, EDS1AIBehavior::Patrol);
			}
			else
			{
				SetBehaviorKey(BlackboardComp, EDS1AIBehavior::Idle);
			}
		}
	}
	
}
