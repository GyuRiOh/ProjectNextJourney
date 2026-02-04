// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/DS1PlayerHUDWidget.h"

#include "DS1PotionWidget.h"
#include "DS1StatBarWidget.h"
#include "DS1WeaponWidget.h"
#include "Components/DS1AttributeComponent.h"
#include "Components/DS1CombatComponent.h"
#include "Components/DS1PotionInventoryComponent.h"
#include "Equipments/DS1Shield.h"
#include "Equipments/DS1Weapon.h"


UDS1PlayerHUDWidget::UDS1PlayerHUDWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UDS1PlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (APawn* OwningPawn = GetOwningPlayerPawn())
	{
		if (UDS1AttributeComponent* Attribute = OwningPawn->GetComponentByClass<UDS1AttributeComponent>())
		{
			Attribute->OnAttributeChanged.AddUObject(this, &ThisClass::OnAttributeChanged);
			Attribute->BroadcastAttributeChanged(EDS1AttributeType::Stamina);
			Attribute->BroadcastAttributeChanged(EDS1AttributeType::Health);
		}

		if (UDS1PotionInventoryComponent* PotionInventory = OwningPawn->GetComponentByClass<UDS1PotionInventoryComponent>())
		{
			// 포션인벤토리 델리게이트 바인딩
			PotionInventory->OnUpdatePotionAmount.AddUObject(this, &ThisClass::OnPotionQuantityChanged);
			PotionInventory->BroadcastPotionUpdate();
		}

		if (UDS1CombatComponent* CombatComponent = OwningPawn->GetComponentByClass<UDS1CombatComponent>())
		{
			CombatComponent->OnChangedWeapon.AddUObject(this, &ThisClass::OnWeaponChanged);
			CombatComponent->OnChangedWeapon.Broadcast();
		}
	}
}

void UDS1PlayerHUDWidget::OnAttributeChanged(EDS1AttributeType AttributeType, float InValue)
{
	switch (AttributeType)
	{
	case EDS1AttributeType::Stamina:
		StaminaBarWidget->SetRatio(InValue);
		break;
	case EDS1AttributeType::Health:
		HealthBarWidget->SetRatio(InValue);
		break;
	}
}

void UDS1PlayerHUDWidget::OnPotionQuantityChanged(int32 InAmount)
{
	if (PotionWidget)
	{
		PotionWidget->SetPotionQuantity(InAmount);
	}
}

void UDS1PlayerHUDWidget::OnWeaponChanged()
{
	if (const APawn* OwningPawn = GetOwningPlayerPawn())
	{
		if (const UDS1CombatComponent* CombatComponent = OwningPawn->GetComponentByClass<UDS1CombatComponent>())
		{
			UTexture2D* WeaponIconTexture = BlankIcon;
			UTexture2D* ShieldIconTexture = BlankIcon;

			// 무기의 아이콘을 골라준다.
			if (const ADS1Weapon* MainWeapon = CombatComponent->GetMainWeapon())
			{
				if (MainWeapon->GetCombatType() != ECombatType::MeleeFists)
				{
					// 무기 아이콘
					WeaponIconTexture = MainWeapon->GetEquipmentIcon();
				}
			}

			// 방패 아이콘을 골라준다.
			if (const ADS1Shield* Shield = CombatComponent->GetShield())
			{
				ShieldIconTexture = Shield->GetEquipmentIcon();
			}

			// 무기 아이콘 적용
			if (::IsValid(WeaponWidget))
			{
				WeaponWidget->SetWeaponImage(WeaponIconTexture);
			}

			// 방패 아이콘 적용
			if (::IsValid(ShieldWidget))
			{
				ShieldWidget->SetWeaponImage(ShieldIconTexture);
			}
		}
	}
}
