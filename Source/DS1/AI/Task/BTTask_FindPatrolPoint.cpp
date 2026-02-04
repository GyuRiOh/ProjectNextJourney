// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Task/BTTask_FindPatrolPoint.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/DS1Enemy.h"
#include "Engine/TargetPoint.h"

EBTNodeResult::Type UBTTask_FindPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    APawn* ControlledPawn = OwnerComp.GetAIOwner()->GetPawn();
    if (!ControlledPawn)
    {
        return EBTNodeResult::Failed;
    }

    if (ADS1Enemy* Character = Cast<ADS1Enemy>(ControlledPawn))
    {
        // TargetPoint의 Location을 Blackboard에 저장.
        OwnerComp.GetBlackboardComponent()->SetValueAsVector(BlackboardLocation.SelectedKeyName, Character->GetPatrolPoint()->GetActorLocation());
        Character->IncrementPatrolIndex();
        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}
