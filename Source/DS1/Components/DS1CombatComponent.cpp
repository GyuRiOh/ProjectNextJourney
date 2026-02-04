// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/DS1CombatComponent.h"

#include "Characters/DS1Character.h"
#include "Equipments/DS1Armour.h"
#include "Equipments/DS1Shield.h"
#include "Equipments/DS1Weapon.h"
#include "Items/DS1PickupItem.h"

UDS1CombatComponent::UDS1CombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UDS1CombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


void UDS1CombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UDS1CombatComponent::SetWeapon(ADS1Weapon* NewWeapon)
{
    // 이미 무기를 가지고 있으면 PickupItem으로 만들어서 떨군다.
    if (::IsValid(MainWeapon))
    {
        if (const AActor* OwnerActor = GetOwner())
        {
            SpawnPickupItem(OwnerActor, MainWeapon->GetClass());

            MainWeapon->Destroy();
        }
    }

	MainWeapon = NewWeapon;

    if (OnChangedWeapon.IsBound())
    {
        OnChangedWeapon.Broadcast();
    }
}

void UDS1CombatComponent::SetArmour(ADS1Armour* NewArmour)
{
    const EDS1ArmourType ArmourType = NewArmour->GetArmourType();

    // 이미 같은 부위에 장착된 방어구가 있으면 PickupItem으로 떨군다.
    if (ADS1Armour* EquippedArmourPart = GetArmour(ArmourType))
    {
	    if (IsValid(EquippedArmourPart))
	    {
		    // PickupItem으로 만든다.
            if (const AActor* OwnerActor = GetOwner())
            {
                SpawnPickupItem(OwnerActor, EquippedArmourPart->GetClass());
            }

            // 장착 해제
            EquippedArmourPart->UnequipItem();
            // 삭제
            EquippedArmourPart->Destroy();
	    }

        ArmourMap[ArmourType] = NewArmour;
    }
    else
    {
        ArmourMap.Add(ArmourType, NewArmour);
    }
}

void UDS1CombatComponent::SetShield(ADS1Shield* NewShield)
{
    if (IsValid(Shield))
    {
	    if (const AActor* OwnerActor = GetOwner())
	    {
            SpawnPickupItem(OwnerActor, Shield->GetClass());

            Shield->Destroy();
	    }
    }

    Shield = NewShield;

    if (OnChangedWeapon.IsBound())
    {
        OnChangedWeapon.Broadcast();
    }
}

void UDS1CombatComponent::SetCombatEnabled(const bool bEnabled)
{
    bCombatEnabled = bEnabled;
    if (OnChangedCombat.IsBound())
    {
        OnChangedCombat.Broadcast(bCombatEnabled);
    }
}

void UDS1CombatComponent::SpawnPickupItem(const AActor* OwnerActor, const TSubclassOf<ADS1Equipment>& NewEquipmentClass) const
{
    ADS1PickupItem* PickupItem = GetWorld()->SpawnActorDeferred<ADS1PickupItem>(ADS1PickupItem::StaticClass(), OwnerActor->GetActorTransform(), nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
    PickupItem->SetEquipmentClass(NewEquipmentClass);
    PickupItem->FinishSpawning(OwnerActor->GetActorTransform());
}

