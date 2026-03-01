# Inventory & Quick Slot Implementation Log (2026-03-01)

## Summary
오늘 작업으로 다음을 완료했습니다.
- 포션 단일 슬롯 UI를 **6칸 퀵슬롯 시스템**으로 확장
- 인벤토리 슬롯/장비 슬롯/퀵슬롯 간 상호작용(우클릭, 드래그앤드롭) 연결
- `WBP_QuickSlot` 에셋 생성 및 HUD 연동
- UE 5.6 기준 프로젝트 파일/빌드 오류 해결 및 빌드 성공 확인

---

## 1) Quick Slot 시스템 추가

### Added
- `Source/DS1/Components/DS1QuickSlotComponent.h`
- `Source/DS1/Components/DS1QuickSlotComponent.cpp`

### 주요 기능
- 슬롯 수: 6 (`MaxQuickSlots = 6`)
- 슬롯 등록:
  - 특정 슬롯 등록
  - 인벤토리 슬롯 기반 등록
  - 첫 빈 슬롯 자동 등록
- 슬롯 사용:
  - 소비 아이템(Consumable) 사용
  - 장비 아이템(Equipment) 장착
- 인벤토리 변경 시 슬롯 유효성 자동 갱신

---

## 2) Inventory Component 확장

### Modified
- `Source/DS1/Components/DS1InventoryComponent.h`
- `Source/DS1/Components/DS1InventoryComponent.cpp`

### 추가 메서드
- `FindFirstSlotByItemData(UDS1ItemData*)`
- `UseConsumableByItemData(UDS1ItemData*)`

목적:
- 퀵슬롯에서 ItemData 기반으로 실제 인벤토리 슬롯을 찾아 소비/사용 가능하게 함.

---

## 3) Player Character 입력/컴포넌트 연결

### Modified
- `Source/DS1/Characters/DS1Character.h`
- `Source/DS1/Characters/DS1Character.cpp`

### 변경 내용
- `UDS1QuickSlotComponent` 컴포넌트 추가
- 숫자키 1~6 바인딩
  - `UseQuickSlot1()` ~ `UseQuickSlot6()`
- 각 키 입력 시 `QuickSlotComponent->UseQuickSlot(index)` 호출

---

## 4) HUD / QuickSlot Widget 구조

### Added
- `Source/DS1/UI/DS1QuickSlotWidget.h`

### Modified
- `Source/DS1/UI/DS1PotionWidget.h`
- `Source/DS1/UI/DS1PotionWidget.cpp`
- `Source/DS1/UI/DS1PlayerHUDWidget.h`
- `Source/DS1/UI/DS1PlayerHUDWidget.cpp`

### 변경 포인트
- 기존 `DS1PotionWidget`에 6칸 아이콘/수량 표시 기능 추가
  - `QuickSlotIcon1~6`
  - `QuickSlotCount1~6`
- 인벤토리 슬롯에서 드래그한 아이템을 퀵슬롯 위젯에 드롭하면
  - 드롭 X 위치 기준으로 1~6 슬롯 인덱스 계산 후 등록
- HUD는 `QuickSlotWidget` 우선 바인딩
- 기존 `PotionWidget`도 fallback 지원 (기존 BP 호환)

---

## 5) Inventory Slot / Equip Slot 연동

### Modified
- `Source/DS1/UI/DS1InventorySlotWidget.h`
- `Source/DS1/UI/DS1InventorySlotWidget.cpp`
- `Source/DS1/UI/DS1InventoryWidget.h`
- `Source/DS1/UI/DS1InventoryWidget.cpp`

### 구현 내용
- 장비 슬롯 초기화 API 추가
  - `InitEquipSlot(...)`
- 장비 슬롯 표시 로직
  - 현재 장착된 무기/방패/방어구를 아이콘으로 표시
- 장비 슬롯 상호작용
  - 우클릭: 장착 해제 (`UnequipToInventory`)
  - 인벤토리 -> 장비 슬롯 드롭: 슬롯 타입 일치 시 장착
  - 장비 슬롯 -> 인벤토리 드롭: 장착 해제
- 인벤토리 하단의 `QuickSlotWidget`(선택) 자동 바인딩 지원

---

## 6) UMG Asset 작업

### Created
- `Content/_Game/UI/WBP_QuickSlot.uasset`
  - `WBP_Potion` 복제 기반 생성
  - 퀵슬롯 UI용으로 사용

### BP 바인딩 이름 규칙
#### WBP_QuickSlot 내부
- 아이콘: `QuickSlotIcon1` ~ `QuickSlotIcon6`
- 수량: `QuickSlotCount1` ~ `QuickSlotCount6`

#### WBP_Inventory 내부
- 그리드: `InventoryGrid`
- 무게 텍스트: `WeightText`
- 장비 슬롯:
  - `WeaponSlot`, `ShieldSlot`, `ChestSlot`, `PantsSlot`, `BootsSlot`, `GlovesSlot`
- (옵션) 하단 퀵바 위젯: `QuickSlotWidget`

#### WBP_PlayerHUD 내부
- 퀵슬롯 위젯 변수: `QuickSlotWidget`

---

## 7) Build / Engine 이슈 해결

### 문제
- 초기 `.sln`에서 잘못된 엔진 경로 참조로 대량 `MSB3202` 발생
- Live Coding 활성 상태로 빌드 잠금 발생

### 조치
- UE 5.6 경로 기준 프로젝트 파일 재생성
- Live Coding 종료 후 재빌드
- 컴파일 오류(`FDragDropEvent` 전방 선언 타입 불일치) 수정
- 최종: `DS1Editor Win64 Development` 빌드 성공 확인

---

## 8) Current Status
- 퀵슬롯 6칸 기능: 동작
- 인벤토리 슬롯 드래그/우클릭: 동작
- 장비 슬롯 표시/장착/해제: 코드 반영 완료
- HUD 상시 퀵슬롯: 코드 반영 완료
- UE 5.6 빌드: 성공

