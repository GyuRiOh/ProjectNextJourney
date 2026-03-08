# Parry System Implementation Log (2026-03-09)

## Summary
오늘 작업으로 다음을 완료했습니다.
- **일반 패리(Parry)** + **퍼펙트 패리(Perfect Parry)** 2단계 패리 시스템 구현
- 입력: RMB 클릭 순간 패리 시도, 유지 시 블로킹으로 전환하는 구조
- 퍼펙트 패리 판정 AnimNotifyState(`AnimNotifyState_DS1PerfectParry`) 신규 추가
- 퍼펙트 패리 피드백: 슬로우모션(GlobalTimeDilation) + 사운드/파티클
- 적 측 패리/퍼펙트 패리 리액션 구현 (`ADS1Enemy::Parried`, `PerfectParried`)
- 새 Gameplay Tag 3개 추가: `Character.State.Parrying`, `Character.State.Parried`, `Character.Action.PerfectParriedHit`

---

## 1) 입력 구조 — RMB 패리 & 블로킹 통합

### 설계 원칙
- RMB **Started** → `Parrying()` 호출 (한 번만, 패리 모션 즉시 실행)
- RMB **Triggered** → `Blocking()` 호출 (홀드 중 매 프레임)
- RMB **Completed** → `BlockingEnd()` 호출

### 충돌 방지 처리
- `Parrying()`에서 즉시 State를 `Character.State.Parrying`으로 세팅
- `Blocking()`의 `CanPlayerBlockStance()`가 `Parrying` 상태를 막으므로,
  같은 프레임에 Triggered가 발동해도 블로킹이 덮어쓰지 않음
- `BlockingEnd()`에서 `Parrying` 상태 중에는 State 클리어를 하지 않음 (AnimNotifyState가 종료를 담당)

```cpp
// DS1Character.cpp — SetupPlayerInputComponent
EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Started,    this, &ThisClass::Parrying);
EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Triggered,  this, &ThisClass::Blocking);
EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Completed,  this, &ThisClass::BlockingEnd);
```

---

## 2) 일반 패리 (Parry)

### 발동 조건 — `CanPerformParry()`
- 메인 무기가 존재할 것
- 전투 타입이 `ECombatType::SwordShield`일 것
- 스태미나 1 이상
- 차단 상태 목록에 없을 것:
  `Attacking, Rolling, GeneralAction, Hit, Blocking, Death, Parrying, DrinkingPotion`

### 실행 흐름 — `Parrying()`
1. State → `Character.State.Parrying`
2. 이동 입력 차단 (`ToggleMovementInput(false)`)
3. 스태미나 소모 (`ParryingStaminaCost = 10.f`)
4. 무기의 `GetMontageForTag(Character.State.Parrying)` 몽타주 재생

### 패리 성공 판정 — `ParriedAttackSucceed()`
- 현재 State가 `Parrying`이고 `bFacingEnemy == true`
- `TakeDamage()` 호출 시 데미지 처리 전에 체크

### 성공 시 처리
- 적에게 `IDS1CombatInterface::Parried()` 호출
- 방어 파티클/사운드 이펙트 (`ShieldBlockingEffect`)
- 실제 데미지는 0 (return)

---

## 3) 퍼펙트 패리 (Perfect Parry)

### 개념
패리 모션 시작 직후 **짧은 윈도우 구간**에 공격을 받으면 퍼펙트 패리 발동.
일반 패리보다 적의 무방비 시간이 더 길고, 슬로우모션 연출 추가.

### AnimNotifyState — `UAnimNotifyState_DS1PerfectParry`

| 파일 | 경로 |
|---|---|
| Header | `Source/DS1/Animation/AnimNotifyState_DS1PerfectParry.h` |
| Source | `Source/DS1/Animation/AnimNotifyState_DS1PerfectParry.cpp` |

- **NotifyBegin**: `Character->SetPerfectParryWindow(true)`
- **NotifyEnd**: `Character->SetPerfectParryWindow(false)`
- 패리 몽타주의 **앞쪽 구간(짧게)** 에만 배치
- DisplayName: `"Perfect Parry Window"` (에디터 표시명)

### 판정 — `PerfectParriedAttackSucceed()`
```cpp
bool ADS1Character::PerfectParriedAttackSucceed() const
{
    return ParriedAttackSucceed() && bInPerfectParryWindow;
}
```

### TakeDamage() 체크 순서
```
1. PerfectParriedAttackSucceed() → 퍼펙트 패리 처리 (우선)
2. ParriedAttackSucceed()        → 일반 패리 처리
3. CanPerformAttackBlocking()    → 블로킹 처리
4. 그 외                         → 피격 처리
```

### 퍼펙트 패리 피드백 — `PerfectParryEffect()`
1. `PerfectParrySound` 재생 (BP에서 에셋 할당)
2. `PerfectParryParticle` 스폰 (BP에서 에셋 할당)
3. **슬로우모션**: `UGameplayStatics::SetGlobalTimeDilation(World, 0.15f)`
4. `PerfectParrySlowDuration(0.3초)` 후 TimeDilation 1.0 복원

#### 슬로우모 타이머 주의사항
> `SetGlobalTimeDilation` 적용 상태에서 `SetTimer`는 dilated time 기준으로 작동.
> 즉, `0.3f` 설정 시 실제 체감 시간은 `0.3 / 0.15 = 2.0초`가 아니라
> **dilated time 기준 0.3초** (실제 월드에서 약 2초 느낌).
> 의도대로 동작하지 않으면 `SetTimerForNextTick` 또는 Tick 기반으로 교체 고려.

### DS1Character 추가 프로퍼티

```cpp
// Category = "Perfect Parry"
float PerfectParrySlowDuration  = 0.3f;   // 슬로우모 지속 시간 (dilated time)
float PerfectParryTimeDilation  = 0.15f;  // 시간 배율 (0.01~1.0)
UParticleSystem* PerfectParryParticle;    // BP에서 할당
USoundCue*       PerfectParrySound;       // BP에서 할당
FTimerHandle     SlowMoTimerHandle;       // 복원 타이머
bool             bInPerfectParryWindow;   // AnimNotifyState에서 관리
```

---

## 4) 적 리액션 — ADS1Enemy

### `Parried()` — 일반 패리 당했을 때
1. 현재 몽타주 즉시 중단 (`StopAnimMontage`)
2. State → `Character.State.Parried`
3. `Character.Action.ParriedHit` 몽타주 재생
4. `(몽타주 길이 + 1.0초)` 후 State 클리어 (사망 상태면 유지)

### `PerfectParried()` — 퍼펙트 패리 당했을 때
1. 현재 몽타주 즉시 중단
2. State → `Character.State.Parried`
3. `Character.Action.PerfectParriedHit` 몽타주 재생
   → 없으면 `Character.Action.ParriedHit` 폴백
4. `(몽타주 길이 + 2.0초)` 후 State 클리어 (일반 패리보다 무방비 시간 1초 더 길음)

### 추가된 타이머 핸들 (DS1Enemy.h)
```cpp
FTimerHandle ParriedDelayTimerHandle;
FTimerHandle PerfectParriedDelayTimerHandle;
FTimerHandle StunnedDelayTimerHandle; // 기존 스턴과 분리 관리
```
- `EndPlay()`에서 3개 모두 ClearTimer 처리

---

## 5) Gameplay Tags 추가

| Tag | 용도 |
|---|---|
| `Character.State.Parrying` | 플레이어가 패리 모션 중 |
| `Character.State.Parried` | 적이 패리 당한 무방비 상태 |
| `Character.Action.ParriedHit` | 적 일반 패리 리액션 몽타주 키 |
| `Character.Action.PerfectParriedHit` | 적 퍼펙트 패리 리액션 몽타주 키 |

> `Character.State.Parrying`과 `Character.Action.ParriedHit`은
> 이전 코드에도 일부 존재했을 수 있으나, 이번 작업에서 정식으로 정의/활용됨.

---

## 6) DS1CombatInterface 변경

```cpp
// Interfaces/DS1CombatInterface.h
virtual void Parried() {}          // 기존 — 일반 패리 리액션
virtual void PerfectParried() {}   // 신규 추가 — 퍼펙트 패리 리액션
```
- `ADS1Enemy`에서 두 메서드 모두 override 구현

---

## 7) 엔진에서 해야 할 남은 작업 (다음 세션)

### ⚠️ 몽타주 AnimNotify/NotifyState 미등록 항목
아래 몽타주들은 C++ 코드에서 참조하지만, **에디터에서 아직 Notify가 등록되지 않음**.
다음 엔진 작업 시 반드시 추가해야 함.

| 몽타주 | 등록해야 할 Notify/NotifyState | 위치 | 비고 |
|---|---|---|---|
| **패리 몽타주** (무기별 `Character.State.Parrying`) | `AnimNotifyState_DS1PerfectParry` | 앞쪽 짧은 구간 (예: 0.0~0.2초) | 퍼펙트 패리 윈도우 |
| **패리 몽타주** | State 종료 AnimNotify (State 클리어) | 몽타주 끝 부분 | 현재 State 클리어 로직 없음 — 패리 모션 후 상태가 안 풀릴 수 있음 |
| **PatriedHit 몽타주** (적용) | 없으면 무방비 해제가 타이머만으로 처리됨 | — | 있어도 무관, 확인만 |
| **PerfectParriedHit 몽타주** (적용) | 현재 없으면 ParriedHit 폴백 사용 | — | 전용 몽타주 원한다면 에셋 신규 제작 필요 |

### ✅ BP에서 에셋 할당 (DS1Character BP)
- `Perfect Parry` 카테고리 노출됨
  - `PerfectParrySound` → 사운드 에셋 지정
  - `PerfectParryParticle` → 파티클 에셋 지정
  - `PerfectParrySlowDuration` — 기본 0.3 (필요 시 조정)
  - `PerfectParryTimeDilation` — 기본 0.15 (필요 시 조정)

### ✅ 무기 데이터에 몽타주 등록 (WeaponData/Blueprint)
각 무기 데이터에 Tag → Montage 매핑이 있어야 패리 작동:
- `Character.State.Parrying` → 패리 모션 몽타주
- `Character.Action.ParriedHit` → 적 피패리 리액션 몽타주
- `Character.Action.PerfectParriedHit` → 적 퍼펙트 피패리 몽타주 (없으면 폴백)

### ⚠️ 패리 후 State 클리어 이슈
- 현재 패리 성공 시 `Character.State.Parrying` State 클리어 코드가 **없음**
- 패리 모션이 끝나도 State가 `Parrying`에 남아 다음 패리/공격이 막힐 수 있음
- 해결 방법 (둘 중 하나 선택):
  1. 패리 몽타주에 AnimNotify를 추가해 `StateComponent->ClearState()` 호출
  2. 몽타주 EndDelegate에서 State 클리어 처리

### ⚠️ 슬로우모 타이머 정확도 검증
- `PerfectParryEffect()`의 `SetTimer(SlowMoTimerHandle, ..., PerfectParrySlowDuration)`는
  dilated world time 기준으로 작동 — 실제 플레이에서 타이밍 확인 필요
- 의도와 다를 경우: Tick에서 RealTime 누적 방식으로 교체

---

## 8) 현재 상태 요약

| 항목 | 상태 |
|---|---|
| C++ 패리 로직 | 완료 |
| C++ 퍼펙트 패리 판정 | 완료 |
| AnimNotifyState_DS1PerfectParry | 완료 (코드) |
| 적 Parried / PerfectParried 리액션 | 완료 |
| 슬로우모션 피드백 | 완료 |
| Gameplay Tags 등록 | 완료 |
| 에디터 몽타주 Notify 등록 | **미완 (다음 세션)** |
| BP 사운드/파티클 에셋 할당 | **미완 (다음 세션)** |
| 패리 후 State 클리어 처리 | **미완 (다음 세션)** |
| 빌드 확인 | 미확인 (빌드 필요) |
