# 치트 인포 (2026-05-31)

## 개요

개발/테스트용 콘솔 치트 명령어 목록. 게임 실행 중 **`~`(백틱)** 키로 콘솔을 열어 입력한다.

---

## 구현 파일

| 파일 | 역할 |
| --- | --- |
| `Source/DS1/Player/DS1CheatManager.h/.cpp` | 치트 명령어 구현체 |
| `Source/DS1/Player/DS1PlayerController.cpp` | `CheatClass = UDS1CheatManager::StaticClass()` 등록 |

---

## 명령어 목록

### GiveItem

```
GiveItem [ItemID] [Count]
```

아이템을 플레이어 인벤토리에 추가한다.

| 인자 | 타입 | 기본값 | 설명 |
| --- | --- | --- | --- |
| `ItemID` | FName | (필수) | `DS1ItemData` 에셋의 ItemID 필드 값 |
| `Count` | int32 | 1 | 추가할 수량 |

**예시**

```
GiveItem Food_Apple 3
GiveItem Drink_Water
```

**결과 메시지**

- 성공: 화면에 초록색으로 `GiveItem: Food_Apple x3 추가`
- ItemID 불일치: 빨간색으로 `GiveItem: 'Food_Apple' 아이템 없음`
- 인벤토리 꽉 참 / 무게 초과: `GiveItem: Food_Apple x0 추가`

---

## 주의사항

- `ItemID`는 `DS1ItemData` 에셋의 **ItemID 필드**와 대소문자까지 정확히 일치해야 한다.
- 인벤토리 슬롯(`MaxSlots = 20`)이 꽉 찼거나 무게 한도(`MaxCarryWeight`)를 초과하면 추가되지 않는다.
- 치트는 `ADS1PlayerController`에 등록되어 있으므로 플레이어 컨트롤러가 없는 상태에서는 동작하지 않는다.

---

## 관련 파일

- `Source/DS1/Data/DS1ItemDataRegistry.h` — ItemID → ItemData 조회
- `Source/DS1/Components/DS1InventoryComponent.h` — `AddItem()` 구현
- `Source/DS1/Data/DS1ItemData.h` — ItemID 필드 위치
