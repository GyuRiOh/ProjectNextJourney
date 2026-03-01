// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "Interfaces/DS1CombatInterface.h"
#include "DS1Character.generated.h"

class UDS1PotionInventoryComponent;
class UDS1InventoryComponent;
class UDS1QuickSlotComponent;
class ADS1FistWeapon;
class UDS1CombatComponent;
class UDS1StateComponent;
class UDS1PlayerHUDWidget;
class UDS1VisionOverlayWidget;
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

	/** ?꾪닾 ?쒖꽦??鍮꾪솢?깊솕 ?좉? */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ToggleCombatAction;

	/** Attack */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* AttackAction;

	/** Heavy Attack */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* HeavyAttackAction;

	/** 諛⑹뼱 ?먯꽭 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* BlockAction;

	/** ?⑤쭅 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ParryAction;

	/** ?ъ뀡留덉떆湲?*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ConsumeAction;

	/** ?몃깽?좊━ ?좉? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ToggleInventoryAction;

private:
	/** 罹먮┃?곗쓽 媛곸쥌 ?ㅽ꺈 愿由?*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UDS1AttributeComponent* AttributeComponent;

	/** 罹먮┃?곗쓽 ?곹깭 愿由?*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UDS1StateComponent* StateComponent;

	/** 臾닿린, ?꾪닾 愿由?*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UDS1CombatComponent* CombatComponent;

	/** ?ъ뀡 ?몃깽?좊━ */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UDS1PotionInventoryComponent* PotionInventoryComponent;

	/** ?꾩씠???몃깽?좊━ */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UDS1InventoryComponent* InventoryComponent;

	/** 퀵 슬롯 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UDS1QuickSlotComponent* QuickSlotComponent;

	/** NPC 가시성(시야 판정) 관리 컴포넌트 */
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
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UDS1VisionOverlayWidget> VisionOverlayWidgetClass;

	UPROPERTY()
	UDS1VisionOverlayWidget* VisionOverlayWidget;

// 二쇰㉨ 臾닿린
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
	/** 吏덉＜ ?띾룄 */
	UPROPERTY(EditAnywhere, Category="Movement Speed")
	float SprintingSpeed = 750.f;

	/** ?쇰컲 ?띾룄 */
	UPROPERTY(EditAnywhere, Category = "Movement Speed")
	float NormalSpeed = 500.f;

	/** 諛⑹뼱?먯꽭 ?띾룄 */
	UPROPERTY(EditAnywhere, Category = "Movement Speed")
	float BlockingSpeed = 250.f;

	UPROPERTY(VisibleAnywhere, Category = "Movement Speed")
	bool bSprinting = false;

// Stamina Cost Section
protected:
	/** 吏덉＜ 以??깅떦 ?뚮え ?ㅽ깭誘몃굹 */
	UPROPERTY(EditAnywhere, Category = "Stamina Cost")
	float SprintStaminaCostPerTick = 0.1f;

	/** 吏덉＜ ?쒖옉???꾩슂??理쒖냼 ?ㅽ깭誘몃굹 */
	UPROPERTY(EditAnywhere, Category = "Stamina Cost")
	float SprintMinStamina = 5.f;

	/** 援щⅤ湲??ㅽ깭誘몃굹 ?뚮え??*/
	UPROPERTY(EditAnywhere, Category = "Stamina Cost")
	float RollingStaminaCost = 15.f;

	/** 諛⑹뼱 ?쇨꺽 ???ㅽ깭誘몃굹 ?뚮え??*/
	UPROPERTY(EditAnywhere, Category = "Stamina Cost")
	float BlockingHitStaminaCost = 20.f;

	/** ?⑤쭅 ?ㅽ깭誘몃굹 ?뚮え??*/
	UPROPERTY(EditAnywhere, Category = "Stamina Cost")
	float ParryingStaminaCost = 10.f;

	/** ?ㅽ깭誘몃굹 ?깅떦 ?뚮났??*/
	UPROPERTY(EditAnywhere, Category = "Stamina Cost")
	float StaminaRegenRate = 0.2f;

	/** ?ㅽ깭誘몃굹 ?뚮났 ?쒖옉 ???湲??쒓컙 */
	UPROPERTY(EditAnywhere, Category = "Stamina Cost")
	float StaminaRegenDelay = 2.f;

// Combo Section
protected:
	/** 肄ㅻ낫 ?쒗??吏꾪뻾以?*/
	bool bComboSequenceRunning = false;

	/** 肄ㅻ낫 ?낅젰 媛?? */
	bool bCanComboInput = false;

	/** 肄ㅻ낫 移댁슫??*/
	int32 ComboCounter = 0;

	/** 肄ㅻ낫 ?낅젰 ?щ? */
	bool bSavedComboInput = false;

	/** 肄ㅻ낫 由ъ뀑 ??대㉧ ?몃뱾 */
	FTimerHandle ComboResetTimerHandle;

protected:
	/** ?곴낵 ?移섏쨷??諛⑺뼢?몄?? */
	bool bFacingEnemy = false;

	/** 臾댁쟻?꾨젅???쒖꽦???щ? */
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
	/** 罹먮┃?곌? ?대룞以묒씤吏 泥댄겕 */
	bool IsMoving() const;
	bool CanToggleCombat() const;
	FORCEINLINE bool IsSprinting() const { return bSprinting; }
	FORCEINLINE bool CanReceiveDamage() const { return !bEnabledIFrames; }

	/** ?대룞 */
	void Move(const FInputActionValue& Values);
	/** 吏덉＜ */
	void Sprinting();
	/** 吏덉＜ 以묐떒 */
	void StopSprint();
	/** 援щⅤ湲?*/
	void Rolling();
	/** ?명꽣?됱뀡 */
	void Interact();
	/** ?꾪닾?곹깭 ?꾪솚 */
	void ToggleCombat();
	void AutoToggleCombat();
	/** Attack */
	void Attack();
	void SpecialAttack();
	void HeavyAttack();
	/** 諛⑹뼱 ?먯꽭 */
	void Blocking();
	void BlockingEnd();
	/** ?⑤쭅 */
	void Parrying();
	/** ?ъ뀡 留덉떆湲?*/
	void Consume();
	/** ?몃깽?좊━ ?좉? */
	void ToggleInventory();
	void UseQuickSlot1();
	void UseQuickSlot2();
	void UseQuickSlot3();
	void UseQuickSlot4();
	void UseQuickSlot5();
	void UseQuickSlot6();

protected:
	/** ?꾩옱 ?곹깭?먯꽌 ?섑뻾 媛?ν븳 ?쇰컲怨듦꺽 */
	FGameplayTag GetAttackPerform() const;

	/** 怨듦꺽 媛??議곌굔 泥댄겕 */
	bool CanPerformAttack(const FGameplayTag& AttackTypeTag) const;
	/** 怨듦꺽 ?ㅽ뻾 */
	void DoAttack(const FGameplayTag& AttackTypeTag);
	/** 肄ㅻ낫 ?ㅽ뻾 */
	void ExecuteComboAttack(const FGameplayTag& AttackTypeTag);
	/** 肄ㅻ낫 珥덇린??*/
	void ResetCombo();

	/** 諛⑹뼱 ?먯꽭 媛???щ? */
	bool CanPlayerBlockStance() const;

	/** 諛⑺뙣 留됯린 諛⑹뼱媛 媛?ν븳吏? */
	bool CanPerformAttackBlocking() const;

	/** ?⑤쭅??媛?ν븳吏? */
	bool CanPerformParry() const;

	/** ?⑤쭅 ?깃났 ?щ? */
	bool ParriedAttackSucceed() const;

	/** ?ъ뀡??留덉떎???덈뒗吏?*/
	bool CanDrinkPotion()const;

	/** ?ъ뀡 留덉떆湲?以묐떒 */
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



