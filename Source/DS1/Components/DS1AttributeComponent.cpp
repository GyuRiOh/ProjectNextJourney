// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/DS1AttributeComponent.h"

#include "DS1GameplayTags.h"
#include "DS1StateComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UDS1AttributeComponent::UDS1AttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UDS1AttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	if (StatData)
	{
		MaxHealth          = StatData->MaxHealth;
		BaseHealth         = StatData->MaxHealth;
		MaxStamina         = StatData->MaxStamina;
		BaseStamina        = StatData->MaxStamina;
		StaminaRegenRate   = StatData->StaminaRegenRate;
		StaminaRegenDelay  = StatData->StaminaRegenDelay;
		MaxHunger               = StatData->MaxHunger;
		SecondsPerHungerDecay   = StatData->SecondsPerHungerDecay;
		MaxThirst               = StatData->MaxThirst;
		SecondsPerThirstDecay   = StatData->SecondsPerThirstDecay;
		LowThreshold            = StatData->LowThreshold;
		ThirstHPDrainPerSecond  = StatData->ThirstHPDrainPerSecond;
		HungerSpeedPenaltyMultiplier = StatData->HungerSpeedPenaltyMultiplier;
	}

	CurrentHunger = MaxHunger;
	CurrentThirst = MaxThirst;

	GetWorld()->GetTimerManager().SetTimer(
		HungerThirstDecayHandle, this,
		&ThisClass::HungerThirstDecayTick, 1.f, true);
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
		if (bStaminaRegenSuppressed)
		{
			return;
		}
		const float Delay = StartDelay < 0.f ? StaminaRegenDelay : StartDelay;
		if (GetWorld()->GetTimerManager().IsTimerActive(StaminaRegenTimerHandle) == false)
		{
			GetWorld()->GetTimerManager().SetTimer(StaminaRegenTimerHandle, this, &ThisClass::RegenerateStaminaHandler, 0.1f, true, Delay);
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

		case EDS1AttributeType::Hunger:
			Ratio = GetHungerRatio();
			break;

		case EDS1AttributeType::Thirst:
			Ratio = GetThirstRatio();
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

void UDS1AttributeComponent::HungerThirstDecayTick()
{
	CurrentHunger = FMath::Clamp(CurrentHunger - (1.0f / SecondsPerHungerDecay), 0.f, MaxHunger);
	CurrentThirst = FMath::Clamp(CurrentThirst - (1.0f / SecondsPerThirstDecay), 0.f, MaxThirst);

	BroadcastAttributeChanged(EDS1AttributeType::Hunger);
	BroadcastAttributeChanged(EDS1AttributeType::Thirst);

	ApplyHungerThirstEffects();
}

void UDS1AttributeComponent::ApplyHungerThirstEffects()
{
	const bool bHungerNowLow = GetHungerRatio() <= LowThreshold;
	const bool bThirstNowLow  = GetThirstRatio()  <= LowThreshold;

	// ── 이동속도 패널티 (배고픔) ──
	if (bHungerNowLow != bHungerLow)
	{
		bHungerLow = bHungerNowLow;
		if (UCharacterMovementComponent* MoveComp = GetOwner()->FindComponentByClass<UCharacterMovementComponent>())
		{
			if (bHungerLow)
			{
				CachedBaseMaxWalkSpeed = MoveComp->MaxWalkSpeed;
				MoveComp->MaxWalkSpeed *= HungerSpeedPenaltyMultiplier;
			}
			else
			{
				MoveComp->MaxWalkSpeed = CachedBaseMaxWalkSpeed;
			}
		}
	}

	// ── HP 드레인 (갈증) ──
	if (bThirstNowLow)
	{
		BaseHealth = FMath::Clamp(BaseHealth - ThirstHPDrainPerSecond, 0.f, MaxHealth);
		BroadcastAttributeChanged(EDS1AttributeType::Health);

		if (BaseHealth <= 0.f)
		{
			if (OnDeath.IsBound())
			{
				OnDeath.Broadcast();
			}
			if (UDS1StateComponent* StateComp = GetOwner()->FindComponentByClass<UDS1StateComponent>())
			{
				StateComp->SetState(DS1GameplayTags::Character_State_Death);
			}
		}
	}
	bThirstLow = bThirstNowLow;

	// ── 스태미나 회복 정지 (둘 다 낮을 때) ──
	const bool bBothLow = bHungerNowLow && bThirstNowLow;
	if (bBothLow && !bStaminaRegenSuppressed)
	{
		bStaminaRegenSuppressed = true;
		ToggleStaminaRegeneration(false);
	}
	else if (!bBothLow && bStaminaRegenSuppressed)
	{
		bStaminaRegenSuppressed = false;
	}
}

void UDS1AttributeComponent::RestoreHunger(float Amount)
{
	CurrentHunger = FMath::Clamp(CurrentHunger + Amount, 0.f, MaxHunger);
	BroadcastAttributeChanged(EDS1AttributeType::Hunger);
	ApplyHungerThirstEffects();
}

void UDS1AttributeComponent::RestoreThirst(float Amount)
{
	CurrentThirst = FMath::Clamp(CurrentThirst + Amount, 0.f, MaxThirst);
	BroadcastAttributeChanged(EDS1AttributeType::Thirst);
	ApplyHungerThirstEffects();
}

