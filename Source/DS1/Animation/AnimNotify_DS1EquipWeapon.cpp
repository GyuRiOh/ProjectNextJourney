// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify_DS1EquipWeapon.h"

#include "DS1GameplayTags.h"
#include "Components/DS1CombatComponent.h"
#include "Equipments/DS1Shield.h"
#include "Equipments/DS1Weapon.h"

UAnimNotify_DS1EquipWeapon::UAnimNotify_DS1EquipWeapon(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UAnimNotify_DS1EquipWeapon::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (const AActor* Owner = MeshComp->GetOwner())
	{
		if (UDS1CombatComponent* CombatComponent = Owner->GetComponentByClass<UDS1CombatComponent>())
		{
			if (ADS1Weapon* MainWeapon = CombatComponent->GetMainWeapon())
			{
				// 장착 해제 대상이 무기인 경우
				if (EquipmentType == EDS1EquipmentType::Weapon)
				{
					bool bCombatEnabled = CombatComponent->IsCombatEnabled();
					FName WeaponSocketName;

					if (MontageActionTag == DS1GameplayTags::Character_Action_Equip)
					{
						bCombatEnabled = true;
						WeaponSocketName = MainWeapon->GetEquipSocketName();
					}
					else if (MontageActionTag == DS1GameplayTags::Character_Action_Unequip)
					{
						bCombatEnabled = false;
						WeaponSocketName = MainWeapon->GetUnequipSocketName();
					}

					// AttachToPlayer 함수보다 먼저 호출해야한다.
					CombatComponent->SetCombatEnabled(bCombatEnabled);
					MainWeapon->AttachToOwner(WeaponSocketName);
				}

				// 장착 해제 대상이 방패인 경우
				if (EquipmentType == EDS1EquipmentType::Shield)
				{
					// 무기가 한손검이면 방패를 장착/해제
					if (MainWeapon->GetCombatType() == ECombatType::SwordShield)
					{
						if (ADS1Shield* Shield = CombatComponent->GetShield())
						{
							FName ShieldSocketName;
							if (MontageActionTag == DS1GameplayTags::Character_Action_Equip)
							{
								ShieldSocketName = Shield->GetEquipSocketName();
							}
							else if (MontageActionTag == DS1GameplayTags::Character_Action_Unequip)
							{
								ShieldSocketName = Shield->GetUnequipSocketName();
							}

							Shield->AttachToOwner(ShieldSocketName);
						}
						
					}
				}
			}
			
		}
	}
}
