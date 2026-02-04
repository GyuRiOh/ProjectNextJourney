// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "DS1MontageActionData.generated.h"



USTRUCT(BlueprintType)
struct FDS1MontageGroup
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UAnimMontage*> Animations;
};

/**
 * 
 */
UCLASS()
class DS1_API UDS1MontageActionData : public UPrimaryDataAsset
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Montage Groups")
	TMap<FGameplayTag, FDS1MontageGroup> MontageGroupMap;

public:
	UAnimMontage* GetMontageForTag(const FGameplayTag& GroupTag, const int32 Index) const;
	UAnimMontage* GetRandomMontageForTag(const FGameplayTag& GroupTag) const;

};
