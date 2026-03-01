// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "Interfaces/DS1CombatInterface.h"
#include "DS1Character.generated.h"

class UDS1PotionInventoryComponent;
class UDS1InventoryComponent;
class UDS1VisionOverlayWidget;
class ADS1FistWeapon;
class UDS1CombatComponent;
class UDS1StateComponent;
class UDS1PlayerHUDWidget;
class UDS1VisibilityComponent;
struct FInputActionValue;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UDS1AttributeComponent;
class USoundCue;
class UParticleSystem;

UCLASS()
class DS1_API ADS1Character : public ACharacter, public IDS1CombatInterface
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

// Input Section
private:
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* SprintRollingAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* InteractAction;

	/** 전투 활성화/비활성화 토글 */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ToggleCombatAction;

	/** Attack */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* AttackAction;

	/** Heavy Attack */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* HeavyAttackAction;

	/** 방어 자세 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* BlockAction;

	/** 패링 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ParryAction;

	/** 포션마시기 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ConsumeAction;

	/** 인벤토리 토글 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ToggleInventoryAction;

private:
	/** 캐릭터의 각종 스탯 관리 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UDS1AttributeComponent* AttributeComponent;

	/** 캐릭터의 상태 관리 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UDS1StateComponent* StateComponent;

	/** 무기, 전투 관리 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UDS1CombatComponent* CombatComponent;

	/** 포션 인벤토리 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UDS1PotionInventoryComponent* PotionInventoryComponent;

	/** 아이템 인벤토리 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UDS1InventoryComponent* InventoryComponent;

	/** 주변 NPC 가시성 관리 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UDS1VisibilityComponent* VisibilityComponent;

// Body parts Mesh
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* TorsoMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* LegsMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* FeetMesh;


// UI Section
protected:
	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UUserWidget> PlayerHUDWidgetClass;

	UPROPERTY()
	UDS1PlayerHUDWidget* PlayerHUDWidget;

	/** 시야 음영 오버레이 위젯 클래스 (블루프린트에서 지정) */
	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UDS1VisionOverlayWidget> VisionOverlayWidgetClass;

	UPROPERTY()
	UDS1VisionOverlayWidget* VisionOverlayWidget;

// 주먹 무기
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ADS1FistWeapon> FistWeaponClass;

// Effect
protected:
	UPROPERTY(EditAnywhere, Category="Effect")
	USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere, Category = "Effect")
	USoundCue* BlockingSound;

	UPROPERTY(EditAnywhere, Category = "Effect")
	UParticleSystem* ImpactParticle;

	UPROPERTY(EditAnywhere, Category = "Effect")
	UParticleSystem* BlockingParticle;

protected:
	/** 질주 속도 */
	UPROPERTY(EditAnywhere, Category="Movement Speed")
	float SprintingSpeed = 750.f;

	/** 일반 속도 */
	UPROPERTY(EditAnywhere, Category = "Movement Speed")
	float NormalSpeed = 500.f;

	/** 방어자세 속도 */
	UPROPERTY(EditAnywhere, Category = "Movement Speed")
	float BlockingSpeed = 250.f;

	UPROPERTY(VisibleAnywhere, Category = "Movement Speed")
	bool bSprinting = false;

// Stamina Cost Section
protected:
	/** 질주 중 틱당 소모 스태미나 */
	UPROPERTY(EditAnywhere, Category = "Stamina Cost")
	float SprintStaminaCostPerTick = 0.1f;

	/** 질주 시작에 필요한 최소 스태미나 */
	UPROPERTY(EditAnywhere, Category = "Stamina Cost")
	float SprintMinStamina = 5.f;

	/** 구르기 스태미나 소모량 */
	UPROPERTY(EditAnywhere, Category = "Stamina Cost")
	float RollingStaminaCost = 15.f;

	/** 방어 피격 시 스태미나 소모량 */
	UPROPERTY(EditAnywhere, Category = "Stamina Cost")
	float BlockingHitStaminaCost = 20.f;

	/** 패링 스태미나 소모량 */
	UPROPERTY(EditAnywhere, Category = "Stamina Cost")
	float ParryingStaminaCost = 10.f;

	/** 스태미나 틱당 회복량 */
	UPROPERTY(EditAnywhere, Category = "Stamina Cost")
	float StaminaRegenRate = 0.2f;

	/** 스태미나 회복 시작 전 대기 시간 */
	UPROPERTY(EditAnywhere, Category = "Stamina Cost")
	float StaminaRegenDelay = 2.f;

// Combo Section
protected:
	/** 콤보 시퀀스 진행중 */
	bool bComboSequenceRunning = false;

	/** 콤보 입력 가능? */
	bool bCanComboInput = false;

	/** 콤보 카운터 */
	int32 ComboCounter = 0;

	/** 콤보 입력 여부 */
	bool bSavedComboInput = false;

	/** 콤보 리셋 타이머 핸들 */
	FTimerHandle ComboResetTimerHandle;

protected:
	/** 적과 대치중인 방향인지? */
	bool bFacingEnemy = false;

	/** 무적프레임 활성화 여부 */
	bool bEnabledIFrames = false;

// Montage Section
protected:
	UPROPERTY(EditAnywhere, Category="Montage")
	UAnimMontage* RollingMontage;

	UPROPERTY(EditAnywhere, Category = "Montage")
	UAnimMontage* DrinkingMontage;

public:
	ADS1Character();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	FORCEINLINE UDS1StateComponent* GetStateComponent() const { return StateComponent; };
	bool IsDeath() const;

	void SetBodyPartActive(const EDS1ArmourType ArmourType, const bool bActive) const;

	virtual float TakeDamage(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	void ImpactEffect(const FVector& Location);
	void ShieldBlockingEffect(const FVector& Location) const;
	void HitReaction(const AActor* Attacker, const EDS1DamageType InDamageType);
	void OnDeath();

protected:
	/** 캐릭터가 이동중인지 체크 */
	bool IsMoving() const;
	bool CanToggleCombat() const;
	FORCEINLINE bool IsSprinting() const { return bSprinting; }
	FORCEINLINE bool CanReceiveDamage() const { return !bEnabledIFrames; }

	/** 이동 */
	void Move(const FInputActionValue& Values);
	/** 질주 */
	void Sprinting();
	/** 질주 중단 */
	void StopSprint();
	/** 구르기 */
	void Rolling();
	/** 인터렉션 */
	void Interact();
	/** 전투상태 전환 */
	void ToggleCombat();
	void AutoToggleCombat();
	/** Attack */
	void Attack();
	void SpecialAttack();
	void HeavyAttack();
	/** 방어 자세 */
	void Blocking();
	void BlockingEnd();
	/** 패링 */
	void Parrying();
	/** 포션 마시기 */
	void Consume();
	/** 인벤토리 토글 */
	void ToggleInventory();

protected:
	/** 현재 상태에서 수행 가능한 일반공격 */
	FGameplayTag GetAttackPerform() const;

	/** 공격 가능 조건 체크 */
	bool CanPerformAttack(const FGameplayTag& AttackTypeTag) const;
	/** 공격 실행 */
	void DoAttack(const FGameplayTag& AttackTypeTag);
	/** 콤보 실행 */
	void ExecuteComboAttack(const FGameplayTag& AttackTypeTag);
	/** 콤보 초기화 */
	void ResetCombo();

	/** 방어 자세 가능 여부 */
	bool CanPlayerBlockStance() const;

	/** 방패 막기 방어가 가능한지? */
	bool CanPerformAttackBlocking() const;

	/** 패링이 가능한지? */
	bool CanPerformParry() const;

	/** 패링 성공 여부 */
	bool ParriedAttackSucceed() const;

	/** 포션을 마실수 있는지?*/
	bool CanDrinkPotion()const;

	/** 포션 마시기 중단 */
	void InterruptWhileDrinkingPotion() const;

// Combo AnimNotify Section
public:
	void EnableComboWindow();
	void DisableComboWindow();
	void AttackFinished(const float ComboResetDelay);

public:
	virtual void ActivateWeaponCollision(EWeaponCollisionType WeaponCollisionType) override;
	virtual void DeactivateWeaponCollision(EWeaponCollisionType WeaponCollisionType) override;
	virtual void ToggleIFrames(const bool bEnabled) override;
};
