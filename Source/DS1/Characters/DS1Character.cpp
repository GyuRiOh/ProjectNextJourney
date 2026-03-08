// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/DS1Character.h"

#include "DS1GameplayTags.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputCoreTypes.h"
#include "Animation/DS1AnimInstance.h"
#include "Blueprint/UserWidget.h"
#include "Components/CapsuleComponent.h"
#include "Components/DS1CombatComponent.h"
#include "Components/DS1AttributeComponent.h"
#include "Components/DS1InventoryComponent.h"
#include "Components/DS1PotionInventoryComponent.h"
#include "Components/DS1QuickSlotComponent.h"
#include "Components/DS1StateComponent.h"
#include "Engine/DamageEvents.h"
#include "Equipments/DS1FistWeapon.h"
#include "Equipments/DS1Weapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/DS1Interact.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Player/DS1PlayerController.h"
#include "Sound/SoundCue.h"
#include "UI/DS1PlayerHUDWidget.h"
#include "UI/DS1VisionOverlayWidget.h"
#include "Components/DS1VisibilityComponent.h"

ADS1Character::ADS1Character()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);

	/** ?대룞, 媛먯냽 ?띾룄 */
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 1200.0f;
	CameraBoom->SetRelativeRotation(FRotator(-50.f, 0.f, 0.f));
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;
	CameraBoom->bDoCollisionTest = false;


	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false;

	TorsoMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Torso"));
	LegsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Legs"));
	FeetMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Feet"));
	TorsoMesh->SetupAttachment(GetMesh());
	LegsMesh->SetupAttachment(GetMesh());
	FeetMesh->SetupAttachment(GetMesh());

	AttributeComponent = CreateDefaultSubobject<UDS1AttributeComponent>(TEXT("Attribute"));
	StateComponent = CreateDefaultSubobject<UDS1StateComponent>(TEXT("State"));
	CombatComponent = CreateDefaultSubobject<UDS1CombatComponent>(TEXT("Combat"));
	// OnDeath Delegate ?⑥닔 諛붿씤??
	AttributeComponent->OnDeath.AddUObject(this, &ThisClass::OnDeath);

	// ?ъ뀡 ?몃깽?좊━
	PotionInventoryComponent = CreateDefaultSubobject<UDS1PotionInventoryComponent>(TEXT("PotionInventory"));

	// ?꾩씠???몃깽?좊━
	InventoryComponent = CreateDefaultSubobject<UDS1InventoryComponent>(TEXT("Inventory"));

	// 퀵 슬롯
	QuickSlotComponent = CreateDefaultSubobject<UDS1QuickSlotComponent>(TEXT("QuickSlot"));

	// NPC 가시성 관리
	VisibilityComponent = CreateDefaultSubobject<UDS1VisibilityComponent>(TEXT("Visibility"));
}

void ADS1Character::BeginPlay()
{
	Super::BeginPlay();

	AttributeComponent->SetStaminaRegenRate(StaminaRegenRate);
	AttributeComponent->SetStaminaRegenDelay(StaminaRegenDelay);

	// 시야 음영 오버레이 - HUD 아래 레이어
	if (VisionOverlayWidgetClass)
	{
		VisionOverlayWidget = CreateWidget<UDS1VisionOverlayWidget>(GetWorld(), VisionOverlayWidgetClass);
		if (VisionOverlayWidget)
		{
			VisionOverlayWidget->AddToViewport(0);
		}
	}

	// Player HUD瑜??앹꽦
	if (PlayerHUDWidgetClass)
	{
		PlayerHUDWidget = CreateWidget<UDS1PlayerHUDWidget>(GetWorld(), PlayerHUDWidgetClass);
		if (PlayerHUDWidget)
		{
			PlayerHUDWidget->AddToViewport(1);
		}
	}

	// 二쇰㉨ 臾닿린 ?μ갑
	if (FistWeaponClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		ADS1FistWeapon* FistWeapon = GetWorld()->SpawnActor<ADS1FistWeapon>(FistWeaponClass, GetActorTransform(), SpawnParams);
		FistWeapon->EquipItem();
	}
}

void ADS1Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADS1Character::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ADS1Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);

		// 吏덉＜ (Shift)
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ThisClass::Sprinting);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ThisClass::StopSprint);
		// 援щⅤ湲?(Space)
		EnhancedInputComponent->BindAction(SprintRollingAction, ETriggerEvent::Started, this, &ThisClass::Rolling);
		// ?명꽣?됱뀡
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ThisClass::Interact);
		// ?꾪닾 ?쒖꽦/鍮꾪솢??
		EnhancedInputComponent->BindAction(ToggleCombatAction, ETriggerEvent::Started, this, &ThisClass::ToggleCombat);

		// Combat ?곹깭濡??먮룞 ?꾪솚.
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ThisClass::AutoToggleCombat);
		// ?쇰컲 怨듦꺽
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Canceled, this, &ThisClass::Attack);
		// ?뱀닔 怨듦꺽
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ThisClass::SpecialAttack);
		// HeavyAttack
		EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &ThisClass::HeavyAttack);

		// 패리 / 방어 (RMB)
		// 누르는 순간 패리 시도, 유지하면 블로킹
		EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Started, this, &ThisClass::Parrying);
		EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Triggered, this, &ThisClass::Blocking);
		EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Completed, this, &ThisClass::BlockingEnd);

		// ?ъ뀡 留덉떆湲?
		EnhancedInputComponent->BindAction(ConsumeAction, ETriggerEvent::Started, this, &ThisClass::Consume);

		// ?몃깽?좊━ ?좉?
		EnhancedInputComponent->BindAction(ToggleInventoryAction, ETriggerEvent::Started, this, &ThisClass::ToggleInventory);
	}

	PlayerInputComponent->BindKey(EKeys::One, EInputEvent::IE_Pressed, this, &ThisClass::UseQuickSlot1);
	PlayerInputComponent->BindKey(EKeys::Two, EInputEvent::IE_Pressed, this, &ThisClass::UseQuickSlot2);
	PlayerInputComponent->BindKey(EKeys::Three, EInputEvent::IE_Pressed, this, &ThisClass::UseQuickSlot3);
	PlayerInputComponent->BindKey(EKeys::Four, EInputEvent::IE_Pressed, this, &ThisClass::UseQuickSlot4);
	PlayerInputComponent->BindKey(EKeys::Five, EInputEvent::IE_Pressed, this, &ThisClass::UseQuickSlot5);
	PlayerInputComponent->BindKey(EKeys::Six, EInputEvent::IE_Pressed, this, &ThisClass::UseQuickSlot6);
}

bool ADS1Character::IsDeath() const
{
	check(StateComponent)

	FGameplayTagContainer CheckTags;
	CheckTags.AddTag(DS1GameplayTags::Character_State_Death);

	return StateComponent->IsCurrentStateEqualToAny(CheckTags);
}

void ADS1Character::SetBodyPartActive(const EDS1ArmourType ArmourType, const bool bActive) const
{
	switch (ArmourType) {
	case EDS1ArmourType::Chest:
		TorsoMesh->SetVisibility(bActive);
		TorsoMesh->SetActive(bActive);
		break;
	case EDS1ArmourType::Pants:
		LegsMesh->SetVisibility(bActive);
		LegsMesh->SetActive(bActive);
		break;
	case EDS1ArmourType::Boots:
		FeetMesh->SetVisibility(bActive);
		FeetMesh->SetActive(bActive);
		break;
	case EDS1ArmourType::Gloves:
		break;
	}
}

float ADS1Character::TakeDamage(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float  ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (!CanReceiveDamage())
	{
		UE_LOG(LogTemp, Warning, TEXT("Rolling IFrames"));
		return ActualDamage;
	}

	check(AttributeComponent);
	check(StateComponent);

	// ?ъ뀡??留덉떆怨??덉쑝硫?以묐떒.
	InterruptWhileDrinkingPotion();

	// ?곴낵 ?移섏쨷??諛⑺뼢?몄??
	bFacingEnemy = UKismetMathLibrary::InRange_FloatFloat(GetDotProductTo(EventInstigator->GetPawn()), -0.1f, 1.f);

	// 퍼펙트 패리 체크 (짧은 윈도우, 우선 체크)
	if (PerfectParriedAttackSucceed())
	{
		if (IDS1CombatInterface* CombatInterface = Cast<IDS1CombatInterface>(EventInstigator->GetPawn()))
		{
			// 3. 적 무방비 리액션
			CombatInterface->PerfectParried();

			ADS1Weapon* MainWeapon = CombatComponent->GetMainWeapon();
			if (IsValid(MainWeapon))
			{
				const FVector Location = MainWeapon->GetActorLocation();

				// 1. 이펙트 피드백 + 2. 슬로모
				PerfectParryEffect(Location);
			}
		}

		return ActualDamage;
	}

	// 일반 패리 체크
	if (ParriedAttackSucceed())
	{
		if (IDS1CombatInterface* CombatInterface = Cast<IDS1CombatInterface>(EventInstigator->GetPawn()))
		{
			CombatInterface->Parried();

			ADS1Weapon* MainWeapon = CombatComponent->GetMainWeapon();
			if (IsValid(MainWeapon))
			{
				FVector Location = MainWeapon->GetActorLocation();
				ShieldBlockingEffect(Location);
			}
		}

		return ActualDamage;
	}


	// 諛⑺뙣 諛⑹뼱媛 媛?ν븳吏?
	if (CanPerformAttackBlocking())
	{
		AttributeComponent->TakeDamageAmount(0.f);
		// ?ㅽ뀒誘몃굹 李④컧
		AttributeComponent->DecreaseStamina(BlockingHitStaminaCost);
		StateComponent->SetState(DS1GameplayTags::Character_State_Blocking);
	}
	else
	{
		AttributeComponent->TakeDamageAmount(ActualDamage);
		StateComponent->SetState(DS1GameplayTags::Character_State_Hit);
	}

	// ?吏곸씠吏 紐삵븯寃??쒕떎.
	StateComponent->ToggleMovementInput(false);

	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent* PointDamageEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);

		// ?곕?吏 諛⑺뼢
		FVector ShotDirection = PointDamageEvent->ShotDirection;
		// ?덊듃 ?꾩튂 (?쒕㈃ ?묒큺 愿??
		FVector ImpactPoint = PointDamageEvent->HitInfo.ImpactPoint;
		// ?덊듃 諛⑺뼢
		FVector ImpactDirection = PointDamageEvent->HitInfo.ImpactNormal;
		// ?덊듃??媛앹껜??Location (媛앹껜 以묒떖 愿??
		FVector HitLocation = PointDamageEvent->HitInfo.Location;

		ImpactEffect(ImpactPoint);

		HitReaction(EventInstigator->GetPawn(), EDS1DamageType::HitBack);
	}
	else if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
	{
		const FRadialDamageEvent* RadialDamageEvent = static_cast<const FRadialDamageEvent*>(&DamageEvent);

		const FVector HitLocation = RadialDamageEvent->Origin;

		ImpactEffect(HitLocation);

		HitReaction(EventInstigator->GetPawn(), EDS1DamageType::KnockBack);
	}

	return ActualDamage;
}

void ADS1Character::ImpactEffect(const FVector& Location)
{
	if (CanPerformAttackBlocking())
	{
		ShieldBlockingEffect(Location);
	}
	else
	{
		if (ImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSound, Location);
		}

		if (ImpactParticle)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, Location);
		}
	}
}

void ADS1Character::ShieldBlockingEffect(const FVector& Location) const
{
	if (BlockingSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), BlockingSound, Location);
	}

	if (BlockingParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BlockingParticle, Location);
	}
}

void ADS1Character::PerfectParryEffect(const FVector& Location)
{
	// --- 1. Sound / Particle (assign in BP) ---
	if (PerfectParrySound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), PerfectParrySound, Location);
	}
	if (PerfectParryParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PerfectParryParticle, Location);
	}

	// --- 2. Slow-mo ---
	GetWorldTimerManager().ClearTimer(SlowMoTimerHandle);
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), PerfectParryTimeDilation);

	FTimerDelegate RestoreDelegate;
	RestoreDelegate.BindLambda([this]()
	{
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
	});
	// TimeDilation충 차사: 코드 타이머 시간은 dilated world time조 혜아대, SetTimerForNextTick조조 체크 안함
	GetWorldTimerManager().SetTimer(SlowMoTimerHandle, RestoreDelegate, PerfectParrySlowDuration, false);
}

void ADS1Character::HitReaction(const AActor* Attacker, const EDS1DamageType InDamageType)
{
	check(CombatComponent)

	if (CanPerformAttackBlocking())
	{
		if (UAnimMontage* BlockingMontage = CombatComponent->GetMainWeapon()->GetMontageForTag(DS1GameplayTags::Character_Action_BlockingHit))
		{
			PlayAnimMontage(BlockingMontage);
		}
	}
	else
	{
		if (InDamageType == EDS1DamageType::HitBack)
		{
			if (UAnimMontage* HitReactAnimMontage = CombatComponent->GetMainWeapon()->GetHitReactMontage(Attacker))
			{
				PlayAnimMontage(HitReactAnimMontage);
			}
		}
		else if (InDamageType == EDS1DamageType::KnockBack)
		{
			if (UAnimMontage* HitReactAnimMontage = CombatComponent->GetMainWeapon()->GetMontageForTag(DS1GameplayTags::Character_Action_KnockBackHit))
			{
				PlayAnimMontage(HitReactAnimMontage);
			}
		}
	}
}

void ADS1Character::OnDeath()
{
	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Ragdoll
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetCollisionProfileName("Ragdoll");
		MeshComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		MeshComp->SetSimulatePhysics(true);
	}
}

bool ADS1Character::IsMoving() const
{
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		return MovementComp->Velocity.Size2D() > 3.f && MovementComp->GetCurrentAcceleration() != FVector::Zero();
	}

	return false;
}

bool ADS1Character::CanToggleCombat() const
{
	check(StateComponent);

	if (IsValid(CombatComponent->GetMainWeapon()) == false)
	{
		return false;
	}

	if (CombatComponent->GetMainWeapon()->GetCombatType() == ECombatType::MeleeFists)
	{
		return false;
	}

	FGameplayTagContainer CheckTags;
	CheckTags.AddTag(DS1GameplayTags::Character_State_Attacking);
	CheckTags.AddTag(DS1GameplayTags::Character_State_Rolling);
	CheckTags.AddTag(DS1GameplayTags::Character_State_GeneralAction);

	return StateComponent->IsCurrentStateEqualToAny(CheckTags) == false;
}

bool ADS1Character::CanPerformAttack(const FGameplayTag& AttackTypeTag) const
{
	check(StateComponent)
	check(CombatComponent)
	check(AttributeComponent)

	if (IsValid(CombatComponent->GetMainWeapon()) == false)
	{
		return false;
	}

	FGameplayTagContainer CheckTags;
	CheckTags.AddTag(DS1GameplayTags::Character_State_Rolling);
	CheckTags.AddTag(DS1GameplayTags::Character_State_GeneralAction);
	CheckTags.AddTag(DS1GameplayTags::Character_State_Hit);
	CheckTags.AddTag(DS1GameplayTags::Character_State_Blocking);
	CheckTags.AddTag(DS1GameplayTags::Character_State_DrinkingPotion);
	CheckTags.AddTag(DS1GameplayTags::Character_State_Parrying);

	return StateComponent->IsCurrentStateEqualToAny(CheckTags) == false
		&& CombatComponent->IsCombatEnabled();
}


void ADS1Character::Move(const FInputActionValue& Values)
{
	check(StateComponent);

	if (!StateComponent->MovementInputEnabled())
	{
		return;
	}

	const FVector2D MovementVector = Values.Get<FVector2D>();

	// 荑쇳꽣酉? 移대찓??yaw 0 怨좎젙?대?濡??붾뱶 諛⑺뼢 吏곸젒 ?ъ슜
	AddMovementInput(FVector::ForwardVector, MovementVector.Y);
	AddMovementInput(FVector::RightVector, MovementVector.X);
}

void ADS1Character::Sprinting()
{
	check(AttributeComponent);
	check(CombatComponent);

	if (CombatComponent->IsBlockingEnabled())
	{
		return;
	}

	if (AttributeComponent->CheckHasEnoughStamina(SprintMinStamina) && IsMoving())
	{
		AttributeComponent->ToggleStaminaRegeneration(false);

		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;

		AttributeComponent->DecreaseStamina(SprintStaminaCostPerTick);

		bSprinting = true;
	}
	else
	{
		StopSprint();
	}
}

void ADS1Character::StopSprint()
{
	check(AttributeComponent);
	check(CombatComponent);

	if (CombatComponent->IsBlockingEnabled())
	{
		return;
	}

	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
	AttributeComponent->ToggleStaminaRegeneration(true);
	bSprinting = false;
}

void ADS1Character::Rolling()
{
	check(AttributeComponent);
	check(StateComponent);

	if (AttributeComponent->CheckHasEnoughStamina(RollingStaminaCost))
	{
		// ?ㅽ깭誘몃굹 ?ъ땐??硫덉땄
		AttributeComponent->ToggleStaminaRegeneration(false);

		// ?대룞?낅젰 泥섎━ 臾댁떆.
		StateComponent->ToggleMovementInput(false);

		// ?ㅽ깭誘몃굹 李④컧.
		AttributeComponent->DecreaseStamina(RollingStaminaCost);

		// 援щⅤ湲??좊땲硫붿씠???ъ깮
		PlayAnimMontage(RollingMontage);

		StateComponent->SetState(DS1GameplayTags::Character_State_Rolling);

		// ?ㅽ깭誘몃굹 ?ъ땐???쒖옉
		AttributeComponent->ToggleStaminaRegeneration(true);
	}
}

void ADS1Character::Interact()
{
	FHitResult OutHit;
	const FVector Start = GetActorLocation();
	const FVector End = Start;
	constexpr float Radius = 100.f;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_OBJECT_INTERACTION));

	TArray<AActor*> ActorsToIgnore;

	bool bHit = UKismetSystemLibrary::SphereTraceSingleForObjects(
		this,
		Start,
		End,
		Radius,
		ObjectTypes,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		OutHit,
		true);

	if (bHit)
	{
		// 媛먯???Actor? Interaction
		if (AActor* HitActor = OutHit.GetActor())
		{
			if (IDS1Interact* Interaction = Cast<IDS1Interact>(HitActor))
			{
				Interaction->Interact(this);
			}
		}
	}
}

void ADS1Character::ToggleCombat()
{
	check(CombatComponent)
	check(StateComponent)

	if (CombatComponent)
	{
		if (const ADS1Weapon* Weapon = CombatComponent->GetMainWeapon())
		{
			if (CanToggleCombat())
			{
				StateComponent->SetState(DS1GameplayTags::Character_State_GeneralAction);

				if (CombatComponent->IsCombatEnabled())
				{
					PlayAnimMontage(Weapon->GetMontageForTag(DS1GameplayTags::Character_Action_Unequip));
				}
				else
				{
					PlayAnimMontage(Weapon->GetMontageForTag(DS1GameplayTags::Character_Action_Equip));
				}
			}
		}
	}
}

void ADS1Character::AutoToggleCombat()
{
	if (CombatComponent)
	{
		if (!CombatComponent->IsCombatEnabled())
		{
			ToggleCombat();
		}
	}
}

void ADS1Character::Attack()
{
	const FGameplayTag AttackTypeTag = GetAttackPerform();

	if (CanPerformAttack(AttackTypeTag))
	{
		ExecuteComboAttack(AttackTypeTag);
	}
}

void ADS1Character::SpecialAttack()
{
	const FGameplayTag AttackTypeTag = DS1GameplayTags::Character_Attack_Special;

	if (CanPerformAttack(AttackTypeTag))
	{
		ExecuteComboAttack(AttackTypeTag);
	}
}

void ADS1Character::HeavyAttack()
{
	AutoToggleCombat();

	const FGameplayTag AttackTypeTag = DS1GameplayTags::Character_Attack_Heavy;

	if (CanPerformAttack(AttackTypeTag))
	{
		ExecuteComboAttack(AttackTypeTag);
	}
}

void ADS1Character::Blocking()
{
	check(CombatComponent);
	check(StateComponent);

	if (CombatComponent->GetMainWeapon())
	{
		if (CanPlayerBlockStance())
		{
			GetCharacterMovement()->MaxWalkSpeed = BlockingSpeed;
			CombatComponent->SetBlockingEnabled(true);
			if (UDS1AnimInstance* AnimInstance = Cast<UDS1AnimInstance>(GetMesh()->GetAnimInstance()))
			{
				AnimInstance->UpdateBlocking(true);
				StateComponent->SetState(DS1GameplayTags::Character_State_Blocking);
			}
		}
	}
}

void ADS1Character::BlockingEnd()
{
	check(CombatComponent);
	check(StateComponent);

	CombatComponent->SetBlockingEnabled(false);
	if (UDS1AnimInstance* AnimInstance = Cast<UDS1AnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInstance->UpdateBlocking(false);

		// 패리 중에는 state 밀지 않음 (AnimNotifyStateꬌ 정리시킴)
		FGameplayTagContainer ParryingCheck;
		ParryingCheck.AddTag(DS1GameplayTags::Character_State_Parrying);
		if (!StateComponent->IsCurrentStateEqualToAny(ParryingCheck))
		{
			StateComponent->ClearState();
		}
	}
	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
}

void ADS1Character::Parrying()
{
	check(CombatComponent);
	check(StateComponent);
	check(AttributeComponent);

	if (CanPerformParry())
	{
		// Blocking() 충돌 방지: 같은 프레임에 Triggered가 발생해도 블로킹 차단
		StateComponent->SetState(DS1GameplayTags::Character_State_Parrying);

		if (const ADS1Weapon* MainWeapon = CombatComponent->GetMainWeapon())
		{
			UAnimMontage* ParryingMontage = MainWeapon->GetMontageForTag(DS1GameplayTags::Character_State_Parrying);

			StateComponent->ToggleMovementInput(false);
			AttributeComponent->ToggleStaminaRegeneration(false);
			AttributeComponent->DecreaseStamina(ParryingStaminaCost);

			PlayAnimMontage(ParryingMontage);

			AttributeComponent->ToggleStaminaRegeneration(true);
		}
	}
}

void ADS1Character::Consume()
{
	if (!StateComponent)
	{
		return;
	}

	if (CanDrinkPotion())
	{
		StateComponent->SetState(DS1GameplayTags::Character_State_DrinkingPotion);
		PlayAnimMontage(DrinkingMontage);
	}
}

FGameplayTag ADS1Character::GetAttackPerform() const
{
	if (IsSprinting())
	{
		return DS1GameplayTags::Character_Attack_Running;
	}

	return DS1GameplayTags::Character_Attack_Light;
}

void ADS1Character::DoAttack(const FGameplayTag& AttackTypeTag)
{
	check(StateComponent)
	check(AttributeComponent)
	check(CombatComponent)

	if (const ADS1Weapon* Weapon = CombatComponent->GetMainWeapon())
	{
		StateComponent->SetState(DS1GameplayTags::Character_State_Attacking);
		StateComponent->ToggleMovementInput(false);
		CombatComponent->SetLastAttackType(AttackTypeTag);

		UAnimMontage* Montage = Weapon->GetMontageForTag(AttackTypeTag, ComboCounter);
		if (!Montage)
		{
			// 肄ㅻ낫 ?쒓퀎 ?꾨떖.
			ComboCounter = 0;
			Montage = Weapon->GetMontageForTag(AttackTypeTag, ComboCounter);
		}

		PlayAnimMontage(Montage);
	}
}

void ADS1Character::ExecuteComboAttack(const FGameplayTag& AttackTypeTag)
{
	if (StateComponent->GetCurrentState() != DS1GameplayTags::Character_State_Attacking)
	{
		if (bComboSequenceRunning && bCanComboInput == false)
		{
			// ?좊땲硫붿씠?섏? ?앸궗吏留??꾩쭅 肄ㅻ낫 ?쒗?ㅺ? ?좏슚????- 異붽? ?낅젰 湲고쉶
			ComboCounter++;
			UE_LOG(LogTemp, Warning, TEXT("Additional input : Combo Counter = %d"), ComboCounter);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT(">>> ComboSequence Started <<<"));
			ResetCombo();
			bComboSequenceRunning = true;
		}

		DoAttack(AttackTypeTag);
		GetWorld()->GetTimerManager().ClearTimer(ComboResetTimerHandle);
	}
	else if (bCanComboInput)
	{
		// 肄ㅻ낫 ?덈룄?곌? ?대젮 ?덉쓣 ??- 理쒖쟻????대컢
		bSavedComboInput = true;
	}
}

void ADS1Character::ResetCombo()
{
	UE_LOG(LogTemp, Warning, TEXT("Combo Reset"));

	bComboSequenceRunning = false;
	bCanComboInput = false;
	bSavedComboInput = false;
	ComboCounter = 0;
}

bool ADS1Character::CanPlayerBlockStance() const
{
	check(StateComponent);
	check(CombatComponent);
	check(AttributeComponent);

	if (IsSprinting())
	{
		return false;
	}

	ADS1Weapon* Weapon = CombatComponent->GetMainWeapon();
	if (!IsValid(Weapon))
	{
		return false;
	}

	FGameplayTagContainer CheckTags;
	CheckTags.AddTag(DS1GameplayTags::Character_State_Attacking);
	CheckTags.AddTag(DS1GameplayTags::Character_State_GeneralAction);
	CheckTags.AddTag(DS1GameplayTags::Character_State_Hit);
	CheckTags.AddTag(DS1GameplayTags::Character_State_Rolling);
	CheckTags.AddTag(DS1GameplayTags::Character_State_DrinkingPotion);
	CheckTags.AddTag(DS1GameplayTags::Character_State_Parrying);

	return StateComponent->IsCurrentStateEqualToAny(CheckTags) == false &&
		Weapon->GetCombatType() == ECombatType::SwordShield &&
		AttributeComponent->CheckHasEnoughStamina(1.f);
}

bool ADS1Character::CanPerformAttackBlocking() const
{
	check(CombatComponent);
	check(AttributeComponent);

	return bFacingEnemy && CombatComponent->IsBlockingEnabled() && AttributeComponent->CheckHasEnoughStamina(BlockingHitStaminaCost);
}

bool ADS1Character::CanPerformParry() const
{
	check(StateComponent);
	check(CombatComponent);
	check(AttributeComponent);

	ADS1Weapon* MainWeapon = CombatComponent->GetMainWeapon();
	if (!IsValid(MainWeapon))
	{
		return false;
	}

	FGameplayTagContainer CheckTags;
	CheckTags.AddTag(DS1GameplayTags::Character_State_Attacking);
	CheckTags.AddTag(DS1GameplayTags::Character_State_Rolling);
	CheckTags.AddTag(DS1GameplayTags::Character_State_GeneralAction);
	CheckTags.AddTag(DS1GameplayTags::Character_State_Hit);
	CheckTags.AddTag(DS1GameplayTags::Character_State_Blocking);
	CheckTags.AddTag(DS1GameplayTags::Character_State_Death);
	CheckTags.AddTag(DS1GameplayTags::Character_State_Parrying);
	CheckTags.AddTag(DS1GameplayTags::Character_State_DrinkingPotion);

	return StateComponent->IsCurrentStateEqualToAny(CheckTags) == false &&
		MainWeapon->GetCombatType() == ECombatType::SwordShield &&
		AttributeComponent->CheckHasEnoughStamina(1.f);
}

bool ADS1Character::ParriedAttackSucceed() const
{
	check(StateComponent);

	FGameplayTagContainer CheckTags;
	CheckTags.AddTag(DS1GameplayTags::Character_State_Parrying);

	return StateComponent->IsCurrentStateEqualToAny(CheckTags) && bFacingEnemy;
}

bool ADS1Character::PerfectParriedAttackSucceed() const
{
	return ParriedAttackSucceed() && bInPerfectParryWindow;
}

bool ADS1Character::CanDrinkPotion() const
{
	check(PotionInventoryComponent);
	check(StateComponent);

	FGameplayTagContainer CheckTags;
	CheckTags.AddTag(DS1GameplayTags::Character_State_Attacking);
	CheckTags.AddTag(DS1GameplayTags::Character_State_Blocking);
	CheckTags.AddTag(DS1GameplayTags::Character_State_Death);
	CheckTags.AddTag(DS1GameplayTags::Character_State_GeneralAction);
	CheckTags.AddTag(DS1GameplayTags::Character_State_Hit);
	CheckTags.AddTag(DS1GameplayTags::Character_State_Parrying);
	CheckTags.AddTag(DS1GameplayTags::Character_State_Rolling);

	return PotionInventoryComponent->GetPotionQuantity() > 0 && StateComponent->IsCurrentStateEqualToAny(CheckTags) == false;
}

void ADS1Character::InterruptWhileDrinkingPotion() const
{
	check(StateComponent);

	FGameplayTagContainer CheckTags;
	CheckTags.AddTag(DS1GameplayTags::Character_State_DrinkingPotion);

	if (StateComponent->IsCurrentStateEqualToAny(CheckTags))
	{
		if (PotionInventoryComponent)
		{
			PotionInventoryComponent->DespawnPotion();
		}
	}
}

void ADS1Character::EnableComboWindow()
{
	bCanComboInput = true;
	UE_LOG(LogTemp, Warning, TEXT("Combo Window Opened: Combo Counter = %d"), ComboCounter);
}

void ADS1Character::DisableComboWindow()
{
	check(CombatComponent)

	bCanComboInput = false;

	if (bSavedComboInput)
	{
		bSavedComboInput = false;
		ComboCounter++;
		UE_LOG(LogTemp, Warning, TEXT("Combo Window Closed: Advancing to next combo = %d"), ComboCounter);
		DoAttack(CombatComponent->GetLastAttackType());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Combo Window Closed: No input received"));
	}
}

void ADS1Character::AttackFinished(const float ComboResetDelay)
{
	UE_LOG(LogTemp, Warning, TEXT("AttackFinished"));
	if (StateComponent)
	{
		StateComponent->ToggleMovementInput(true);
	}
	// ComboResetDelay ?꾩뿉 肄ㅻ낫 ?쒗??醫낅즺
	GetWorld()->GetTimerManager().SetTimer(ComboResetTimerHandle, this, &ThisClass::ResetCombo, ComboResetDelay, false);
}

void ADS1Character::ActivateWeaponCollision(EWeaponCollisionType WeaponCollisionType)
{
	if (CombatComponent)
	{
		CombatComponent->GetMainWeapon()->ActivateCollision(WeaponCollisionType);
	}
}

void ADS1Character::DeactivateWeaponCollision(EWeaponCollisionType WeaponCollisionType)
{
	if (CombatComponent)
	{
		CombatComponent->GetMainWeapon()->DeactivateCollision(WeaponCollisionType);
	}
}

void ADS1Character::ToggleIFrames(const bool bEnabled)
{
	bEnabledIFrames = bEnabled;
}

void ADS1Character::ToggleInventory()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ADS1PlayerController* DS1PC = Cast<ADS1PlayerController>(PC))
		{
			DS1PC->ToggleInventory();
		}
	}
}

void ADS1Character::UseQuickSlot1()
{
	if (QuickSlotComponent)
	{
		QuickSlotComponent->UseQuickSlot(0);
	}
}

void ADS1Character::UseQuickSlot2()
{
	if (QuickSlotComponent)
	{
		QuickSlotComponent->UseQuickSlot(1);
	}
}

void ADS1Character::UseQuickSlot3()
{
	if (QuickSlotComponent)
	{
		QuickSlotComponent->UseQuickSlot(2);
	}
}

void ADS1Character::UseQuickSlot4()
{
	if (QuickSlotComponent)
	{
		QuickSlotComponent->UseQuickSlot(3);
	}
}

void ADS1Character::UseQuickSlot5()
{
	if (QuickSlotComponent)
	{
		QuickSlotComponent->UseQuickSlot(4);
	}
}

void ADS1Character::UseQuickSlot6()
{
	if (QuickSlotComponent)
	{
		QuickSlotComponent->UseQuickSlot(5);
	}
}



