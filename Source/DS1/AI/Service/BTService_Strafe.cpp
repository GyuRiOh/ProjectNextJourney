// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Service/BTService_Strafe.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTService_Strafe::UBTService_Strafe()
{
	INIT_SERVICE_NODE_NOTIFY_FLAGS();
}

void UBTService_Strafe::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

    AAIController* OwnerController = OwnerComp.GetAIOwner();
    if (!OwnerController)
    {
        return;
    }

    APawn* ControlledPawn = OwnerController->GetPawn();
    if (!ControlledPawn)
    {
        return;
    }

    const UBlackboardComponent* BlackBoardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackBoardComp)
    {
        return;
    }

    if (AActor* TargetActor = Cast<AActor>(BlackBoardComp->GetValueAsObject(TargetKey.SelectedKeyName)))
    {
        // AI의 시선을 플레이어에게 고정.
        OwnerController->SetFocus(TargetActor, EAIFocusPriority::Gameplay);

        if (const ACharacter* Character = Cast<ACharacter>(ControlledPawn))
        {
            if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
            {
                MovementComp->bUseControllerDesiredRotation = true;
                MovementComp->bOrientRotationToMovement = false;
            }
        }
    }
}

void UBTService_Strafe::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* OwnerController = OwnerComp.GetAIOwner();
    if (!OwnerController)
    {
        return;
    }

    APawn* ControlledPawn = OwnerController->GetPawn();
    if (!ControlledPawn)
    {
        return;
    }

    // 시선 고정해제.
    OwnerController->ClearFocus(EAIFocusPriority::Gameplay);

    if (const ACharacter* Character = Cast<ACharacter>(ControlledPawn))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            MovementComp->bUseControllerDesiredRotation = false;
            MovementComp->bOrientRotationToMovement = true;
        }
    }

	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBTService_Strafe::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* OwnerController = OwnerComp.GetAIOwner();
    if (!OwnerController)
    {
        return;
    }

    APawn* ControlledPawn = OwnerController->GetPawn();
    if (!ControlledPawn)
    {
        return;
    }

    const FVector Origin = ControlledPawn->GetActorLocation();
    FNavLocation OutRandomPoint(Origin);

    if (const UNavigationSystemV1* NavSys = UNavigationSystemV1::GetNavigationSystem(ControlledPawn))
    {
        if (NavSys->GetRandomPointInNavigableRadius(Origin, SearchRadius, OutRandomPoint))
        {
            UAIBlueprintHelperLibrary::SimpleMoveToLocation(OwnerController, OutRandomPoint.Location);
        }
    }
}
