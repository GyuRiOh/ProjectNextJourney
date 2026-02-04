// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/DS1RotationComponent.h"

#include "Kismet/KismetMathLibrary.h"

UDS1RotationComponent::UDS1RotationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UDS1RotationComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


void UDS1RotationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!TargetActor)
	{
		return;
	}

	if (!bShouldRotate)
	{
		return;
	}

	const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetOwner()->GetActorLocation(), TargetActor->GetActorLocation());
	GetOwner()->SetActorRotation(FRotator(0.f, LookAtRotation.Yaw, 0.f));
}

