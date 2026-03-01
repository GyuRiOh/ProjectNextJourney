# Character FOV System

## 1. 시스템 개요
이 시스템은 플레이어 기준 시야(FOV)를 두 레이어로 처리합니다.

- 판정 레이어: 적이 플레이어에게 보이는지 계산 (`UDS1VisibilityComponent`)
- 표현 레이어: 화면 음영/시야 콘 오버레이 렌더링 (`UDS1VisionOverlayWidget`)

핵심 목적은 아래 두 가지입니다.

- 게임플레이 판정: 시야각/거리/장애물(LineTrace) 기반으로 적 가시성 제어
- 시각 피드백: 시야 밖 영역을 어둡게 표시하고, 시야 구조를 직관적으로 전달

## 2. 구성 요소와 연결

### 2.1 `UDS1VisibilityComponent` (판정)
- 파일:
  - `Source/DS1/Components/DS1VisibilityComponent.h`
  - `Source/DS1/Components/DS1VisibilityComponent.cpp`
- 생성 위치:
  - `Source/DS1/Characters/DS1Character.cpp` (`VisibilityComponent = CreateDefaultSubobject...`)

동작:
- `CheckInterval` 주기로 타이머 실행
- 모든 `ADS1Enemy`를 순회
- `NearDetectionRadius` 우선 판정(방향 무시)
- 그 외는 `VisibilityRadius + ConeAngleDegrees`로 판정
- 콘 내부면 LOS(LineTrace, `ECC_Visibility`) 검사 후 표시/비표시 결정

### 2.2 `UDS1VisionOverlayWidget` (표현)
- 파일:
  - `Source/DS1/UI/DS1VisionOverlayWidget.h`
  - `Source/DS1/UI/DS1VisionOverlayWidget.cpp`
- 생성 위치:
  - `Source/DS1/Characters/DS1Character.cpp`
  - `VisionOverlayWidgetClass`로 생성 후 `AddToViewport(0)`

동작:
- 플레이어 월드 위치를 위젯 좌표로 투영
- 시야 각도 방향을 계산해 어두운 영역/콘 형태를 그림
- 판정 컴포넌트의 값(`ConeHalfAngle`, `NearDetectionRadius`)을 읽어 동기화

## 3. 좌표/방향 기준 (현재 구현)

### 3.1 방향 기준
- 판정/오버레이 모두 `GetVisionForward2D()`를 사용해 동일 축을 공유
- 기본은 Actor Forward
- 필요 시 옵션으로 Mesh Forward 사용 가능

### 3.2 화면 좌표 기준
- `ProjectWorldLocationToWidgetPosition(...)` 사용
- 위젯 `Local` 좌표계로 통일해서 X/Y 오프셋 문제 방지

## 4. 주요 파라미터 (어디서 조정?)

## 4.1 판정 파라미터 (`UDS1VisibilityComponent`)
조정 위치:
- BP 기준: `BP_Player`의 `Visibility` 컴포넌트 디테일
- C++ 기본값: `Source/DS1/Components/DS1VisibilityComponent.h`

파라미터:
- `CheckInterval` (기본 `0.15`)
  - 가시성 체크 주기(초)
- `VisibilityRadius` (기본 `2000`)
  - 콘 판정 최대 거리
- `ConeAngleDegrees` (기본 `90`)
  - 시야 콘 전체 각도
- `NearDetectionRadius` (기본 `400`)
  - 근접 원형 감지 반경(방향 무시)
  - `0`으로 두면 근접 원형 판정 비활성화
- `ForwardYawOffsetDegrees` (기본 `0`)
  - 시야 방향 미세 회전 보정
- `bUseMeshForwardAsVisionBasis` (기본 `false`)
  - `true`면 메시 축 기준으로 전방 사용

## 4.2 오버레이 파라미터 (`UDS1VisionOverlayWidget`)
조정 위치:
- BP 기준: `WBP_VisionOverlay` 디테일(해당 위젯 클래스)
- C++ 기본값: `Source/DS1/UI/DS1VisionOverlayWidget.h`

스타일/표시:
- `DarkColor`
  - 시야 밖 어두운 영역 색/알파
- `bDrawConeBoundary` (기본 `false`)
  - 시야 콘 경계선(직선+호) 표시 여부
- `bDrawNearCircleBoundary` (기본 `false`)
  - 근접 원 경계선 표시 여부
- `ConeBoundaryColor`, `NearCircleColor`, `LineThickness`
  - 경계선 색/두께

보정:
- `ConeApexOffsetZ` (기본 `0`)
  - 시야 시작점 높이 오프셋
- `ForwardAngleCorrectionDeg` (기본 `0`)
  - 화면상 전방 각도 미세 보정
- `ArcStepDeg` (기본 `4`)
  - 곡선 분할 간격(작을수록 부드럽고 비용 증가)
- `ConeArcRadiusPct` (기본 `0.25`)
  - 콘 외곽 호 반경(뷰포트 짧은 변 비율)

## 5. 자주 쓰는 튜닝 가이드

### 5.1 "적이 너무 빨리 보인다/늦게 보인다"
- `VisibilityRadius` 조정
- `NearDetectionRadius` 조정
- `ConeAngleDegrees` 조정

### 5.2 "시야 방향이 캐릭터 정면과 미세하게 다르다"
- `ForwardYawOffsetDegrees` 먼저 소폭 조정 (예: `-5 ~ +5`)
- 필요 시 `bUseMeshForwardAsVisionBasis` 사용 여부 변경

### 5.3 "경계선 자체를 숨기고 싶다"
- `bDrawConeBoundary = false`
- `bDrawNearCircleBoundary = false`

## 6. 디버깅 체크리스트
- BP에서 C++ 기본값을 오버라이드했는지 확인
- Live Coding 사용 시 `Ctrl+Alt+F11`로 반영
- 위젯 클래스가 실제로 `VisionOverlayWidgetClass`에 연결되어 있는지 확인
- 판정은 `ECC_Visibility` 충돌 채널 설정 영향을 받음 (LOS LineTrace)

## 7. 관련 파일 목록
- `Source/DS1/Characters/DS1Character.h`
- `Source/DS1/Characters/DS1Character.cpp`
- `Source/DS1/Components/DS1VisibilityComponent.h`
- `Source/DS1/Components/DS1VisibilityComponent.cpp`
- `Source/DS1/UI/DS1VisionOverlayWidget.h`
- `Source/DS1/UI/DS1VisionOverlayWidget.cpp`
