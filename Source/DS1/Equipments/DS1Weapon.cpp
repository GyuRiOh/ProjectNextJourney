// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipments/DS1Weapon.h"

#include "DS1GameplayTags.h"
#include "DS1Shield.h"
#include "Animation/DS1AnimInstance.h"
#include "Characters/DS1Character.h"
#include "Components/DS1CombatComponent.h"
#include "Components/DS1WeaponCollisionComponent.h"
#include "Data/DS1MontageActionData.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ADS1Weapon::ADS1Weapon()
{
	WeaponCollision = CreateDefaultSubobject<UDS1WeaponCollisionComponent>("MainCollision");
	WeaponCollision->OnHitActor.AddUObject(this, &ThisClass::OnHitActor);

	SecondWeaponCollision = CreateDefaultSubobject<UDS1WeaponCollisionComponent>("SecondCollision");
	SecondWeaponCollision->OnHitActor.AddUObject(this, &ThisClass::OnHitActor);

	StaminaCostMap.Add(DS1GameplayTags::Character_Attack_Light, 7.f);
	StaminaCostMap.Add(DS1GameplayTags::Character_Attack_Running, 12.f);
	StaminaCostMap.Add(DS1GameplayTags::Character_Attack_Special, 15.f);
	StaminaCostMap.Add(DS1GameplayTags::Character_Attack_Heavy, 20.f);

	DamageMultiplierMap.Add(DS1GameplayTags::Character_Attack_Heavy, 1.8f);
	DamageMultiplierMap.Add(DS1GameplayTags::Character_Attack_Running, 1.8f);
	DamageMultiplierMap.Add(DS1GameplayTags::Character_Attack_Special, 2.1f);
}

void ADS1Weapon::EquipItem()
{
	Super::EquipItem();

	CombatComponent = GetOwner()->GetComponentByClass<UDS1CombatComponent>();

	if (CombatComponent)
	{
		CombatComponent->SetWeapon(this);

		const FName AttachSocket = CombatComponent->IsCombatEnabled() ? EquipSocketName : UnequipSocketName;

		AttachToOwner(AttachSocket);

		// 무기의 충돌 트레이스 컴포넌트에 무기 메쉬 컴포넌트를 설정합니다.
		WeaponCollision->SetWeaponMesh(Mesh);

		// 장착한 무기의 CombatType으로 업데이트.
		if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
		{
			if (UDS1AnimInstance* Anim = Cast<UDS1AnimInstance>(OwnerCharacter->GetMesh()->GetAnimInstance()))
			{
				Anim->UpdateCombatMode(CombatType);
			}
		}


		// 무기를 소유한 OwnerActor를 충돌에서 무시합니다.
		WeaponCollision->AddIgnoredActor(GetOwner());

		// 방패를 이미 가지고 있는지 체크해서 소켓의 위치를 잡아준다.
		if (ADS1Shield* Shield = CombatComponent->GetShield())
		{
			FName ShieldAttachSocket = Shield->GetUnequipSocketName();

			if (CombatType == ECombatType::SwordShield)
			{
				if (CombatComponent->IsCombatEnabled())
				{
					ShieldAttachSocket = Shield->GetEquipSocketName();
				}
			}

			Shield->AttachToOwner(ShieldAttachSocket);
		}
	}
}

void ADS1Weapon::Drop()
{
	// 맨주먹이 아니면
	if (CombatType != ECombatType::MeleeFists)
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetSimulatePhysics(true);
	}
}

UAnimMontage* ADS1Weapon::GetMontageForTag(const FGameplayTag& Tag, const int32 Index) const
{
	return MontageActionData->GetMontageForTag(Tag, Index);
}

UAnimMontage* ADS1Weapon::GetRandomMontageForTag(const FGameplayTag& Tag) const
{
	return MontageActionData->GetRandomMontageForTag(Tag);
}

UAnimMontage* ADS1Weapon::GetHitReactMontage(const AActor* Attacker) const
{
	// LookAt 회전값을 구합니다. (현재 Actor가 공격자를 바라보는 회전값)
	const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetOwner()->GetActorLocation(), Attacker->GetActorLocation());
	// 현재 Actor의 회전값과 LookAt 회전값의 차이를 구합니다.
	const FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(GetOwner()->GetActorRotation(), LookAtRotation);
	// Z축 기준의 회전값 차이만을 취합니다.
	const float DeltaZ = DeltaRotation.Yaw;

	EHitDirection HitDirection = EHitDirection::Front;

	if (UKismetMathLibrary::InRange_FloatFloat(DeltaZ, -45.f, 45.f))
	{
		HitDirection = EHitDirection::Front;
		UE_LOG(LogTemp, Log, TEXT("Front"));
	}
	else if (UKismetMathLibrary::InRange_FloatFloat(DeltaZ, 45.f, 135.f))
	{
		HitDirection = EHitDirection::Left;
		UE_LOG(LogTemp, Log, TEXT("Left"));
	}
	else if (UKismetMathLibrary::InRange_FloatFloat(DeltaZ, 135.f, 180.f)
		|| UKismetMathLibrary::InRange_FloatFloat(DeltaZ, -180.f, -135.f))
	{
		HitDirection = EHitDirection::Back;
		UE_LOG(LogTemp, Log, TEXT("Back"));
	}
	else if (UKismetMathLibrary::InRange_FloatFloat(DeltaZ, -135.f, -45.f))
	{
		HitDirection = EHitDirection::Right;
		UE_LOG(LogTemp, Log, TEXT("Right"));
	}

	UAnimMontage* SelectedMontage = nullptr;
	switch (HitDirection)
	{
	case EHitDirection::Front:
		SelectedMontage = GetMontageForTag(DS1GameplayTags::Character_Action_HitReaction, 0);
		break;
	case EHitDirection::Back:
		SelectedMontage = GetMontageForTag(DS1GameplayTags::Character_Action_HitReaction, 1);
		break;
	case EHitDirection::Left:
		SelectedMontage = GetMontageForTag(DS1GameplayTags::Character_Action_HitReaction, 2);
		break;
	case EHitDirection::Right:
		SelectedMontage = GetMontageForTag(DS1GameplayTags::Character_Action_HitReaction, 3);
		break;
	}

	return SelectedMontage;
}

float ADS1Weapon::GetStaminaCost(const FGameplayTag& InTag) const
{
	if (StaminaCostMap.Contains(InTag))
	{
		return StaminaCostMap[InTag];
	}
	return 0.f;
}

float ADS1Weapon::GetAttackDamage() const
{
	if (const AActor* OwnerActor = GetOwner())
	{
		const FGameplayTag LastAttackType = CombatComponent->GetLastAttackType();

		if (DamageMultiplierMap.Contains(LastAttackType))
		{
			const float Multiplier = DamageMultiplierMap[LastAttackType];
			return BaseDamage * Multiplier;
		}
	}

	return BaseDamage;
}

void ADS1Weapon::OnHitActor(const FHitResult& Hit)
{
	AActor* TargetActor = Hit.GetActor();
	
	// 데미지 방향
	FVector DamageDirection = GetOwner()->GetActorForwardVector();

	// 데미지
	float AttackDamage = GetAttackDamage();

	UGameplayStatics::ApplyPointDamage(
		TargetActor,
		AttackDamage,
		DamageDirection,
		Hit,
		GetOwner()->GetInstigatorController(),
		this,
		nullptr);
}

void ADS1Weapon::ActivateCollision(EWeaponCollisionType InCollisionType)
{
	switch (InCollisionType)
	{
	case EWeaponCollisionType::MainCollision:
		WeaponCollision->TurnOnCollision();
		break;
	case EWeaponCollisionType::SecondCollision:
		SecondWeaponCollision->TurnOnCollision();
		break;
	}
}

void ADS1Weapon::DeactivateCollision(EWeaponCollisionType InCollisionType)
{
	switch (InCollisionType)
	{
	case EWeaponCollisionType::MainCollision:
		WeaponCollision->TurnOffCollision();
		break;
	case EWeaponCollisionType::SecondCollision:
		SecondWeaponCollision->TurnOffCollision();
		break;
	}
}
