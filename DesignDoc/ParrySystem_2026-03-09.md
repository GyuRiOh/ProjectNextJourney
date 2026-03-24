# Parry System Implementation Log (2026-03-09, 최종 업데이트: 2026-03-22)

## Summary
- **일반 패리(Parry)** 시스템 구현 (단일 티어)
- 입력: RMB 탭(0.2초 미만) → 패리 / RMB 홀드(0.2초 이상) → 방어
- 패리 판정: `bInParryWindow` 플래그 + 타이머 기반
- 패리/방어 스태미나 소모 없음
- 패리 성공 시 적 리액션: `ADS1Enemy::Parried()`
- 패리 파티클: `ParryParticle` (BP_Character에서 에셋 할당 필요)

> ⚠️ 퍼펙트 패리(PerfectParry) 슬로우모션/전용 파티클/사운드 시스템은 현재 미구현.
> `AnimNotifyState_DS1PerfectParry` 클래스 파일은 존재하지만 실제 코드에서 사용하지 않음.

---

## 1) 입력 구조 — RMB 탭/홀드 통합

### IMC 설정 (IA_Blocking)
- `IA_Blocking` — RMB, 트리거: **길게 누르기(Hold)**, 한계치 `0.2초`
- `IA_Parry` — 미사용 (삭제해도 무방)

### C++ 바인딩 (DS1Character.cpp)
```cpp
EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Started,   this, &ThisClass::Parrying);    // RMB 누르는 순간
EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Canceled,  this, &ThisClass::ParryingEnd); // 0.2초 미만 탭 → 패리
EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Completed, this, &ThisClass::BlockingEnd); // 0.2초 이상 홀드 후 뗌 → 방어 종료
```

### 동작 흐름
| 입력 | 이벤트 흐름 | 결과 |
| --- | --- | --- |
| RMB 탭 (< 0.2초) | Started → Canceled | 패리 윈도우 오픈 → ParryingEnd() → 윈도우 만료 후 BlockingEnd() |
| RMB 홀드 (≥ 0.2초) | Started → Completed | 패리 윈도우 오픈 → 홀드 유지 → BlockingEnd() |

---

## 2) 패리 (Parry)

### 발동 조건 — `CanPerformParry()`
- 메인 무기가 존재할 것
- 전투 타입이 `ECombatType::SwordShield`일 것
- 차단 상태 목록에 없을 것:
  `Attacking, Rolling, GeneralAction, Hit, Blocking, Death, DrinkingPotion`
- **스태미나 조건 없음** (스태미나 소모 없음)

### 실행 흐름 — `Parrying()`
1. 이미 방어/패리 중이면 재진입 방지 (`IsBlockingEnabled()` 체크)
2. 이동 속도 → `BlockingSpeed`
3. `SetBlockingEnabled(true)` + AnimInstance 방어 자세 + State → `Character.State.Blocking`
4. `bInParryWindow = true` + 타이머(`ParryWindowDuration = 0.3f`) 시작
5. 타이머 만료 시 `bInParryWindow = false`

### 패리 판정 — `ParriedAttackSucceed()`
```cpp
return bInParryWindow && bFacingEnemy && CombatComponent->IsBlockingEnabled();
```

### 패리 성공 시 처리 (TakeDamage)
1. 적에게 `IDS1CombatInterface::Parried()` 호출
2. `ParryEffect(MainWeapon->GetActorLocation())` — 무기 위치에 파티클 스폰
3. 실제 데미지 0 (return)

### ParryEffect()
```cpp
void ADS1Character::ParryEffect(const FVector& Location) const
{
    if (ParryParticle)
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParryParticle, Location);
}
```
- `ParryParticle` — `UPROPERTY(EditAnywhere, Category="Effect")`, BP_Character에서 에셋 할당

### 종료 흐름 — `ParryingEnd()` / `BlockingEnd()`
- `ParryingEnd()`: 윈도우가 열려 있으면 남은 타이머 만료 후 `BlockingEnd()` 호출
- `BlockingEnd()`: 방어 해제 + 상태 클리어 + **`ToggleStaminaRegeneration(true)`** (재충전 시작)

---

## 3) 방어 (Blocking)

### 방어 가능 판정 — `CanPerformAttackBlocking()`
```cpp
return bFacingEnemy && CombatComponent->IsBlockingEnabled();
```
- **스태미나 조건 없음** (방어 피격 시 스태미나 소모 없음)

### 방어 피격 처리 (TakeDamage)
- 데미지 0 처리
- State → `Character.State.Blocking`

### 방어 가능 조건 — `CanPlayerBlockStance()`
- 차단 상태: `Attacking, GeneralAction, Hit, Rolling, DrinkingPotion, Parrying`
- 무기 존재 + `ECombatType::SwordShield` + 스태미나 1 이상

---

## 4) 적 리액션 — ADS1Enemy

### `Parried()` — 패리 당했을 때
1. 현재 몽타주 즉시 중단
2. State → `Character.State.Parried`
3. `Character.Action.ParriedHit` 몽타주 재생
4. `(몽타주 길이 + 1.0초)` 후 State 클리어

### 타이머 핸들
```cpp
FTimerHandle ParriedDelayTimerHandle;
FTimerHandle StunnedDelayTimerHandle;
```
- `EndPlay()`에서 모두 ClearTimer 처리

---

## 5) 스태미나 정책

| 행동 | 스태미나 소모 |
| --- | --- |
| 패리 | **없음** |
| 방어 피격 | **없음** |
| 달리기 | 있음 (매 틱) |
| 구르기 | 있음 (1회) |

### 재충전
- `BlockingEnd()` 호출 시 `ToggleStaminaRegeneration(true)` 자동 호출
- 재충전 딜레이: `StaminaRegenDelay = 2.f` (DS1Character.cpp에서 AttributeComponent에 주입)

---

## 6) Gameplay Tags

| Tag | 용도 |
| --- | --- |
| `Character.State.Blocking` | 방어/패리 중 (통합 사용) |
| `Character.State.Parried` | 적이 패리 당한 무방비 상태 |
| `Character.Action.ParriedHit` | 적 패리 리액션 몽타주 키 |

---

## 7) DS1Character 관련 프로퍼티

```cpp
// Category = "Parry"
float ParryWindowDuration = 0.3f;   // 패리 판정 윈도우 지속 시간 (BP 조정 가능)

// Category = "Effect"
UParticleSystem* ParryParticle;     // BP_Character에서 에셋 할당

// Internal
bool          bInParryWindow = false;
FTimerHandle  ParryWindowTimerHandle;

// Category = "Stamina Cost" (현재 패리/방어에서 미사용)
float ParryingStaminaCost = 10.f;
float BlockingHitStaminaCost = 20.f;
```

---

## 8) 에디터에서 해야 할 남은 작업

### ✅ 완료
- IMC `IA_Blocking` Hold 트리거 설정 (0.2초)
- 패리/방어 스태미나 소모 제거
- 방어/패리 후 스태미나 재충전 수정

### ⚠️ 미완 (에디터 작업 필요)
| 항목 | 내용 |
| --- | --- |
| **BP_Character `ParryParticle` 에셋 할당** | Effect 카테고리 > Parry Particle 슬롯에 파티클 에셋 지정 |
| **패리 몽타주 AnimNotifyState 등록** | `AnimNotifyState_DS1Parry` — 패리 판정 구간에 배치 (현재 타이머로 대체 중) |
| **패리 후 State 클리어 Notify** | 패리 몽타주 끝에 State 클리어 AnimNotify 등록 필요 |
| **적 ParriedHit 몽타주 등록** | 무기 데이터 `Character.Action.ParriedHit` → 몽타주 매핑 |
| **`IA_Parry` 정리** | IMC에서 미사용 Action 삭제 |

### ❌ 미구현 (향후 필요 시)

- 퍼펙트 패리 (슬로우모션, 전용 파티클/사운드)
- 퍼펙트 패리 적 리액션 (`PerfectParried()`, 전용 몽타주)

---

## 9) 현재 상태 요약

| 항목 | 상태 |
| --- | --- |
| C++ 패리 로직 | ✅ 완료 |
| IMC Hold 트리거 설정 | ✅ 완료 |
| 스태미나 소모 제거 | ✅ 완료 |
| 방어/패리 후 스태미나 재충전 | ✅ 완료 |
| 적 Parried 리액션 | ✅ 완료 |
| BP_Character 파티클 에셋 할당 | ⚠️ 미완 |
| 패리 몽타주 Notify 등록 | ⚠️ 미완 |
| 퍼펙트 패리 시스템 | ❌ 미구현 |
