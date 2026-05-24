# 배고픔 & 갈증 시스템 (2026-05-24)

## 개요

타르코프(Escape from Tarkov) 스타일 생존 요소. 시간이 지남에 따라 배고픔과 갈증 수치가 자동으로 감소하며, 낮아지면 패널티가 적용된다.

| 상태 | 조건 | 패널티 |
| --- | --- | --- |
| 배고픔 낮음 | `CurrentHunger / MaxHunger ≤ 0.3` | 이동속도 30% 감소 |
| 갈증 낮음 | `CurrentThirst / MaxThirst ≤ 0.3` | 초당 HP 2 감소 |
| 둘 다 낮음 | 위 두 조건 동시 | 위 두 가지 + 스태미나 자동 회복 정지 |

수치는 HUD에 항상 바(Bar) 형태로 표시된다.

---

## 수정된 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `Source/DS1/DS1Define.h` | `EDS1AttributeType`에 `Hunger`/`Thirst` 추가, `EDS1ConsumableEffectType` enum 신규 추가 |
| `Source/DS1/Data/DS1ItemData.h` | `ConsumableEffectType` 필드 추가 |
| `Source/DS1/Components/DS1AttributeComponent.h` | 배고픔/갈증 프로퍼티, 패널티 파라미터, 내부 상태 플래그, 메서드 선언 추가 |
| `Source/DS1/Components/DS1AttributeComponent.cpp` | 1초 감소 타이머, 패널티 적용 로직, `RestoreHunger`/`RestoreThirst` 구현 |
| `Source/DS1/Components/DS1InventoryComponent.cpp` | `UseConsumableFromSlot()`에 EffectType 분기 추가 |
| `Source/DS1/UI/DS1PlayerHUDWidget.h` | `HungerBarWidget`, `ThirstBarWidget` BindWidgetOptional 추가 |
| `Source/DS1/UI/DS1PlayerHUDWidget.cpp` | NativeConstruct 초기 Broadcast, OnAttributeChanged switch 확장 |

---

## 신규 Enum

### DS1Define.h

```cpp
// EDS1AttributeType 확장
UENUM(BlueprintType)
enum class EDS1AttributeType : uint8
{
    Stamina,
    Health,
    Hunger,   // 추가
    Thirst,   // 추가
};

// 신규: 소비 아이템 효과 유형
UENUM(BlueprintType)
enum class EDS1ConsumableEffectType : uint8
{
    RestoreHP,
    RestoreHunger,
    RestoreThirst,
};
```

---

## DS1AttributeComponent — 추가 프로퍼티

```cpp
// 배고픔
float MaxHunger = 100.f;
float CurrentHunger = 100.f;
float HungerDecayRate = 0.5f;   // 초당 감소량

// 갈증
float MaxThirst = 100.f;
float CurrentThirst = 100.f;
float ThirstDecayRate = 0.8f;   // 초당 감소량

// 패널티 임계값
float LowThreshold = 0.3f;              // 30% 이하 = low 상태
float ThirstHPDrainPerSecond = 2.f;     // 갈증 low 시 초당 HP 감소
float HungerSpeedPenaltyMultiplier = 0.7f; // 배고픔 low 시 이동속도 배율

// 내부 상태
FTimerHandle HungerThirstDecayHandle;
float CachedBaseMaxWalkSpeed = 0.f;
bool bHungerLow = false;
bool bThirstLow = false;
bool bStaminaRegenSuppressed = false;
```

모든 패널티 파라미터는 `EditAnywhere`로 에디터에서 조정 가능하다.

---

## 동작 흐름

### 1. 감소 타이머 (BeginPlay 시작)

```cpp
// 1초마다 HungerThirstDecayTick() 호출
GetWorld()->GetTimerManager().SetTimer(
    HungerThirstDecayHandle, this,
    &ThisClass::HungerThirstDecayTick, 1.f, true);
```

### 2. HungerThirstDecayTick()

```cpp
CurrentHunger -= HungerDecayRate;   // 기본 0.5/초
CurrentThirst -= ThirstDecayRate;   // 기본 0.8/초
// BroadcastAttributeChanged → HUD 업데이트
// ApplyHungerThirstEffects() 호출
```

### 3. ApplyHungerThirstEffects() — 패널티 평가

```text
배고픔 low 상태 변화 감지 (bHungerNowLow != bHungerLow)
  → low 진입: CachedBaseMaxWalkSpeed 저장 후 MaxWalkSpeed *= 0.7
  → low 해제: MaxWalkSpeed = CachedBaseMaxWalkSpeed 복원

갈증 low 상태
  → BaseHealth -= ThirstHPDrainPerSecond (매 tick)
  → HP 0 도달 시 기존 사망 로직(OnDeath Delegate + StateComp::Death) 실행

둘 다 low
  → bStaminaRegenSuppressed = true
  → ToggleStaminaRegeneration(false) 호출
둘 중 하나라도 회복
  → bStaminaRegenSuppressed = false
  → 스태미나 재생은 다음 스태미나 소모 시 자연스럽게 재개
```

### 4. 스태미나 회복 차단

`ToggleStaminaRegeneration(true)` 진입 시 `bStaminaRegenSuppressed` 체크:

```cpp
if (bEnabled && bStaminaRegenSuppressed)
{
    return; // 배고픔+갈증 패널티 중 차단
}
```

기존 스태미나 회복 흐름을 수정 없이 그대로 재활용한다.

---

## 소비 아이템 연동

### DS1ItemData.h — 신규 필드

```cpp
// 기존 EffectValue 아래에 추가
UPROPERTY(EditDefaultsOnly, Category = "Item | Consumable",
    meta = (EditCondition = "ItemType == EDS1ItemType::Consumable"))
EDS1ConsumableEffectType ConsumableEffectType = EDS1ConsumableEffectType::RestoreHP;
```

기본값이 `RestoreHP`이므로 기존 포션 데이터 에셋은 수정 없이 자동 호환된다.

### DS1InventoryComponent.cpp — UseConsumableFromSlot() 분기

```cpp
switch (Slot.ItemData->ConsumableEffectType)
{
case EDS1ConsumableEffectType::RestoreHP:
    AttribComp->HealPlayer(Slot.ItemData->EffectValue);    break;
case EDS1ConsumableEffectType::RestoreHunger:
    AttribComp->RestoreHunger(Slot.ItemData->EffectValue); break;
case EDS1ConsumableEffectType::RestoreThirst:
    AttribComp->RestoreThirst(Slot.ItemData->EffectValue); break;
}
```

`RestoreHunger()` / `RestoreThirst()` 호출 후 `ApplyHungerThirstEffects()`를 즉시 재실행해 패널티 상태가 바로 해제된다.

---

## HUD 연동

### DS1PlayerHUDWidget.h

```cpp
UPROPERTY(meta = (BindWidgetOptional), BlueprintReadWrite)
UDS1StatBarWidget* HungerBarWidget;

UPROPERTY(meta = (BindWidgetOptional), BlueprintReadWrite)
UDS1StatBarWidget* ThirstBarWidget;
```

`BindWidgetOptional`로 선언되어 WBP에 위젯이 없어도 빌드 에러가 발생하지 않는다.

### DS1PlayerHUDWidget.cpp — OnAttributeChanged

```cpp
case EDS1AttributeType::Hunger:
    if (HungerBarWidget) HungerBarWidget->SetRatio(InValue); break;
case EDS1AttributeType::Thirst:
    if (ThirstBarWidget) ThirstBarWidget->SetRatio(InValue); break;
```

`NativeConstruct`에서 초기 Broadcast도 추가되어 있어 HUD가 열릴 때 바가 즉시 현재 값으로 초기화된다.

---

## 에디터 작업 (빌드 후 필수)

### 1. WBP_PlayerHUD 수정

- `DS1StatBarWidget` 타입의 `HungerBarWidget`, `ThirstBarWidget` 추가 및 레이아웃 배치
- 기존 `DS1StatBarWidget`과 동일하게 `FillColorAndOpacity`로 색상 구분 권장 (배고픔: 주황, 갈증: 하늘)

### 2. 음식 아이템 데이터 에셋 생성

| 필드 | 값 |
| --- | --- |
| `ItemType` | `Consumable` |
| `ConsumableEffectType` | `RestoreHunger` |
| `EffectValue` | `30` (회복량) |
| `MaxStackCount` | `5` 등 |

### 3. 음료 아이템 데이터 에셋 생성

| 필드 | 값 |
| --- | --- |
| `ItemType` | `Consumable` |
| `ConsumableEffectType` | `RestoreThirst` |
| `EffectValue` | `40` (회복량) |
| `MaxStackCount` | `5` 등 |

---

## 파라미터 튜닝 가이드

| 프로퍼티 | 기본값 | 효과 |
| --- | --- | --- |
| `HungerDecayRate` | 0.5/초 | 약 200초(3.3분)에 0으로 감소 |
| `ThirstDecayRate` | 0.8/초 | 약 125초(2분)에 0으로 감소 |
| `LowThreshold` | 0.3 | 30% 이하에서 패널티 발동 |
| `ThirstHPDrainPerSecond` | 2 | 갈증 low 시 50초면 HP 소진 |
| `HungerSpeedPenaltyMultiplier` | 0.7 | 이동속도 30% 감소 |

게임 플레이 테스트 후 `LowThreshold`, `DecayRate` 위주로 조정하면 된다.

---

## 관련 파일

- `Source/DS1/DS1Define.h` — enum 정의
- `Source/DS1/Data/DS1ItemData.h` — 소비 아이템 효과 타입 필드
- `Source/DS1/Components/DS1AttributeComponent.h/.cpp` — 핵심 로직
- `Source/DS1/Components/DS1InventoryComponent.cpp` — 소비 분기
- `Source/DS1/UI/DS1PlayerHUDWidget.h/.cpp` — HUD 연동

---

## 기능 설명 (플레이어 체감 기준)

### 배고픔 (Hunger)

게임 시작과 동시에 배고픔 수치가 초당 0.5씩 자동으로 줄어든다. 수치가 30% 이하로 떨어지면 이동속도가 30% 감소한다. 음식 아이템을 사용하면 수치가 회복되고 이동속도 패널티도 즉시 해제된다.

### 갈증 (Thirst)

배고픔보다 빠르게 초당 0.8씩 감소한다. 수치가 30% 이하가 되면 초당 HP 2씩 드레인이 시작된다. 아무 조치를 취하지 않으면 약 50초 만에 HP가 소진되어 사망한다. 음료 아이템으로 수치를 회복하면 드레인이 즉시 멈춘다.

### 스태미나 회복 정지 (둘 다 낮을 때)

배고픔과 갈증 **둘 다** 30% 이하일 때, 스태미나 자동 회복이 완전히 멈춘다. 스태미나를 소모하면 회복되지 않아 전투와 달리기가 모두 제한된다. 둘 중 하나라도 30% 이상으로 회복되면 다음 스태미나 소모 시점부터 자동 회복이 재개된다.

---

## 파라미터 조정 위치

`DS1AttributeComponent`를 가진 블루프린트(보통 BP_Player)를 에디터에서 열고, Details 패널에서 아래 항목을 조정한다.

| 파라미터 | 카테고리 | 기본값 | 설명 |
| --- | --- | --- | --- |
| `HungerDecayRate` | Hunger | 0.5/초 | 클수록 배고픔이 빨리 줄어든다 |
| `ThirstDecayRate` | Thirst | 0.8/초 | 클수록 갈증이 빨리 줄어든다 |
| `LowThreshold` | Penalty | 0.3 | 패널티 발동 임계값 (0.3 = 30%) |
| `HungerSpeedPenaltyMultiplier` | Penalty | 0.7 | 이동속도 배율 (0.7 = 30% 감소) |
| `ThirstHPDrainPerSecond` | Penalty | 2.0 | 갈증 low 시 초당 HP 감소량 |

빠른 테스트가 필요할 때는 `HungerDecayRate`와 `ThirstDecayRate`를 5.0 이상으로 올려서 확인하고, 완료 후 원래 값으로 되돌린다.
