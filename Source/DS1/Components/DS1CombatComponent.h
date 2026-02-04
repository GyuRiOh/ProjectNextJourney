// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DS1Define.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "DS1CombatComponent.generated.h"


class ADS1Equipment;
class ADS1Shield;
class ADS1Armour;
class ADS1Weapon;

DECLARE_MULTICAST_DELEGATE_OneParam(FDelegateOnChangedCombat, bool);
DECLARE_MULTICAST_DELEGATE(FDelegateOnChangedWeapon);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DS1_API UDS1CombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Combat 상태 전환을 알리는 Delegate */
	FDelegateOnChangedCombat OnChangedCombat;

	/** 무기 변경을 알리는 Delegate */
	FDelegateOnChangedWeapon OnChangedWeapon;

protected:
	UPROPERTY()
	ADS1Weapon* MainWeapon;

	UPROPERTY()
	TMap<EDS1ArmourType, ADS1Armour*> ArmourMap;

	UPROPERTY()
	ADS1Shield* Shield;

	/* 전투 활성화 상태인지? */
	UPROPERTY(EditAnywhere)
	bool bCombatEnabled = false;

	/** 방어자세를 취하고 있는지? */
	UPROPERTY()
	bool bBlockingEnabled = false;

	/** 마지막 AttackType */
	UPROPERTY(VisibleAnywhere)
	FGameplayTag LastAttackType;

public:	
	UDS1CombatComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void SetWeapon(ADS1Weapon* NewWeapon);
	void SetArmour(ADS1Armour* NewArmour);
	void SetShield(ADS1Shield* NewShield);

public:
	FORCEINLINE bool IsCombatEnabled() const { return bCombatEnabled; }
	void SetCombatEnabled(const bool bEnabled);

	FORCEINLINE bool IsBlockingEnabled() const { return bBlockingEnabled; }
	FORCEINLINE void SetBlockingEnabled(const bool bEnabled) { bBlockingEnabled = bEnabled; }

	FORCEINLINE ADS1Weapon* GetMainWeapon() const { return MainWeapon; };
	FORCEINLINE ADS1Shield* GetShield() const { return Shield; }
	FORCEINLINE ADS1Armour* GetArmour(const EDS1ArmourType ArmourType)
	{
		if (ArmourMap.Contains(ArmourType))
		{
			return ArmourMap[ArmourType];
		}
		return nullptr;
	}

	FORCEINLINE FGameplayTag GetLastAttackType() const { return LastAttackType; };
	FORCEINLINE void SetLastAttackType(const FGameplayTag& NewAttackTypeTag) { LastAttackType = NewAttackTypeTag; };

private:
	void SpawnPickupItem(const AActor* OwnerActor, const TSubclassOf<ADS1Equipment>& NewEquipmentClass) const;
};
