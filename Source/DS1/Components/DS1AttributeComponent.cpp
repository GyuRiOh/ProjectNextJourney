// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/DS1AttributeComponent.h"

#include "DS1GameplayTags.h"
#include "DS1StateComponent.h"

UDS1AttributeComponent::UDS1AttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UDS1AttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	
}


void UDS1AttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

bool UDS1AttributeComponent::CheckHasEnoughStamina(float StaminaCost) const
{
	return BaseStamina >= StaminaCost;
}

void UDS1AttributeComponent::DecreaseStamina(float StaminaCost)
{
	BaseStamina = FMath::Clamp(BaseStamina - StaminaCost, 0.f, MaxStamina);

	BroadcastAttributeChanged(EDS1AttributeType::Stamina);
}

void UDS1AttributeComponent::ToggleStaminaRegeneration(bool bEnabled, float StartDelay)
{
	if (bEnabled)
	{
		if (GetWorld()->GetTimerManager().IsTimerActive(StaminaRegenTimerHandle) == false)
		{
			GetWorld()->GetTimerManager().SetTimer(StaminaRegenTimerHandle, this, &ThisClass::RegenerateStaminaHandler, 0.1f, true, StartDelay);
		}
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(StaminaRegenTimerHandle);
	}
}

void UDS1AttributeComponent::BroadcastAttributeChanged(EDS1AttributeType InAttributeType) const
{
	if (OnAttributeChanged.IsBound())
	{
		// Type에 따라서 값을 골라준다.
		float Ratio = 0.f;
		switch (InAttributeType)
		{
		case EDS1AttributeType::Stamina:
			Ratio = GetStaminaRatio();
			break;

		case EDS1AttributeType::Health:
			Ratio = GetHealthRatio();
			break;
		}

		OnAttributeChanged.Broadcast(InAttributeType, Ratio);
	}
}

void UDS1AttributeComponent::TakeDamageAmount(float DamageAmount)
{
	// 방어력 공식 적용.
	const float MaxDamage = DamageAmount * (DamageAmount / (DamageAmount + DefenseStat));
	const float TotalDamage = FMath::Clamp(DamageAmount, 0, MaxDamage);

	GEngine->AddOnScreenDebugMessage(6, 1.f, FColor::Red, FString::Printf(TEXT("DamageAmount:%f, TotalDamage: %f"), DamageAmount, TotalDamage));

	// 체력 차감.
	BaseHealth = FMath::Clamp(BaseHealth - TotalDamage, 0.f, MaxHealth);

	BroadcastAttributeChanged(EDS1AttributeType::Health);

	if (BaseHealth <= 0.f)
	{
		// Call Death Delegate
		if (OnDeath.IsBound())
		{
			OnDeath.Broadcast();
		}

		// Set Death State
		if (UDS1StateComponent* StateComp = GetOwner()->FindComponentByClass<UDS1StateComponent>())
		{
			StateComp->SetState(DS1GameplayTags::Character_State_Death);
		}
	}
}

void UDS1AttributeComponent::HealPlayer(float HealAmount)
{
	BaseHealth = FMath::Clamp(BaseHealth + HealAmount, 0.f, MaxHealth);
	BroadcastAttributeChanged(EDS1AttributeType::Health);
}

void UDS1AttributeComponent::RegenerateStaminaHandler()
{
	BaseStamina = FMath::Clamp(BaseStamina + StaminaRegenRate, 0.f, MaxStamina);

	BroadcastAttributeChanged(EDS1AttributeType::Stamina);

	if (BaseStamina >= MaxStamina)
	{
		ToggleStaminaRegeneration(false);
	}
}

