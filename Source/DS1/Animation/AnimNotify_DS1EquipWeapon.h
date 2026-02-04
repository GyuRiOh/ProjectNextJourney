// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DS1Define.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_DS1EquipWeapon.generated.h"

/**
 * 
 */
UCLASS(meta=( DisplayName = "Equip Weapon"))
class DS1_API UAnimNotify_DS1EquipWeapon : public UAnimNotify
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag MontageActionTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDS1EquipmentType EquipmentType = EDS1EquipmentType::Weapon;


public:
	UAnimNotify_DS1EquipWeapon(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
