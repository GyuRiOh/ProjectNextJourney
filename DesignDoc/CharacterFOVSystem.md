# Character FOV System

## 1. 시스템 개요
이 시스템은 플레이어 기준 시야(FOV)를 두 레이어로 처리합니다.

- 판정 레이어: 적이 플레이어에게 보이는지 계산 (`UDS1VisibilityComponent`)
- 표현 레이어: 화면 음영/시야 콘 오버레이 렌더링 (`UDS1VisionOverlayWidget`)

핵심 목적은 아래 두 가지입니다.

- 게임플레이 판정: 시야각/거리/장애물(LineTrace) 기반으로 적 가시성 제어
- 시각 피드백: 시야 밖 영역을 어둡게 표시하고, 시야 구조를 직관적으로 전달

---

## 2. 구성 요소와 연결

### 2.1 `UDS1VisibilityComponent` (판정)
- 파일:
  - `Source/DS1/Components/DS1VisibilityComponent.h`
  - `Source/DS1/Components/DS1VisibilityComponent.cpp`
- 생성 위치:
  - `Source/DS1/Characters/DS1Character.cpp` (`VisibilityComponent = CreateDefaultSubobject...`)

**동작 흐름:**
1. `BeginPlay`에서 `CheckInterval` 주기 타이머 등록 (`FTimerHandle`)
2. 타이머마다 `CheckEnemyVisibility()` 실행
3. `GetAllActorsOfClass`로 씬 내 모든 `ADS1Enemy` 수집
4. 각 적에 대해 아래 순서로 판정:
   - `NearDetectionRadius > 0` 이고 거리 ≤ NearDetectionRadius → **즉시 가시** (방향 무관)
   - 거리 ≤ `VisibilityRadius` 이고 콘 내부(dot product) → **LOS 체크**
   - 그 외 → 비가시
5. `Enemy->SetVisibleToPlayer(bVisible)` 호출
6. 이번 프레임 가시 목록을 `CurrentlyVisibleEnemies`(TSet)에 갱신

**LOS 체크 (`HasLineOfSight`):**
- 플레이어 위치와 적 위치 모두에 **눈높이 보정 Z+60** 적용 후 LineTrace
  - 발 위치 기준이면 낮은 벽에도 항상 차단되므로 허리~눈 높이(Z+60)를 기준으로 사용
- `ECC_Visibility` 채널로 `LineTraceSingleByChannel` 수행
- 히트 없음 또는 히트 대상이 Enemy 자신이면 → 시야 확보(`true`)
- `Params.AddIgnoredActor(GetOwner())` 로 자기 자신 무시

**수평 전방 벡터 (`GetVisionForward2D`):**
- `GetActorForwardVector()`(또는 Mesh) 기반으로 XY만 추출 후 정규화
- Z성분 제거로 경사 지형에서도 수평 판정 유지
- `ForwardYawOffsetDegrees`가 0이 아니면 추가 Yaw 회전 적용

### 2.2 `UDS1VisionOverlayWidget` (표현)
- 파일:
  - `Source/DS1/UI/DS1VisionOverlayWidget.h`
  - `Source/DS1/UI/DS1VisionOverlayWidget.cpp`
- 생성 위치:
  - `Source/DS1/Characters/DS1Character.cpp`
  - `VisionOverlayWidgetClass`로 생성 후 `AddToViewport(0)`

**Tick / Paint 분리 아키텍처:**
- `NativeTick` : 매 프레임 데이터 수집·캐싱 (월드→스크린 투영, 각도 계산)
  - 데이터가 유효하면 `bDataValid = true`로 설정
- `NativePaint` : `bDataValid` 가 true일 때만 렌더링 수행
  - 이 구조로 Tick 실패 시 이전 프레임 잔상 대신 즉시 렌더링 중단

**NativeTick 주요 처리:**
1. `PlayerController` → `Pawn` 에서 `UDS1VisibilityComponent` 참조 캐싱
2. `ProjectWorldLocationToWidgetPosition` 으로 캐릭터 Apex(허리 높이) 스크린 좌표 획득
3. **카메라 Right/Up 벡터 기반** 전방 각도 계산:
   - `PlayerCameraManager` 에서 카메라 회전 행렬 추출
   - `CamRight(EAxis::Y)`, `CamUp(EAxis::Z)` 단위 벡터 추출
   - `ScreenFwdX = dot(Fwd2D, CamRight)`, `ScreenFwdY = -dot(Fwd2D, CamUp)`
   - `Atan2(ScreenFwdY, ScreenFwdX)` → 스크린 공간 전방 각도(도)
   - 이 방식으로 카메라 회전과 무관하게 항상 올바른 스크린 방향 보장
4. **NearRadius 월드→스크린 px 변환:**
   - Apex 위치에서 월드 X축 방향으로 `NearWorldRadius` 만큼 떨어진 점을 투영
   - 두 스크린 좌표 사이 거리 = 스크린상 Near 반지름(px)
   - 투시 왜곡이 반영된 정확한 픽셀 반지름 사용

**NativePaint 렌더링 요소:**
1. **어두운 도넛 섹터** (`DrawDarkAnnularSector`) — 항상 그림
2. **콘 경계선** (`DrawConeBoundary`) — `bDrawConeBoundary = true`일 때만
3. **근접 원 아웃라인** (`DrawNearCircle`) — `bDrawNearCircleBoundary = true`이고 NearR > 1px일 때만

---

## 3. 렌더링 상세 — 어두운 영역

### 3.1 도넛 섹터 방식 (`DrawDarkAnnularSector`)
어두운 영역을 "콘 밖 + 근접 원 바깥"으로 표현하기 위해 **도넛(annular) 섹터** 메시를 생성:

```
Inner edge = Apex + Dir * NearRadiusAbs  (근접 원 경계)
Outer edge = 화면 끝 교점 (RayToScreenEdge)
```

- 지정 각도 범위를 `ArcStepDeg` 간격으로 샘플링
- 화면 모서리가 범위 내에 포함되면 추가 샘플로 삽입 (코너 누락 방지)
- 샘플 정렬 후 `[inner, outer]` 쌍으로 버텍스 생성
- `FSlateDrawElement::MakeCustomVerts` 로 삼각형 스트립 렌더링

NearDetectionRadius = 0 이면 Inner edge = Apex (근접 원 없음, 순수 삼각 팬)

### 3.2 화면 교점 계산 (`RayToScreenEdge`)
주어진 각도에서 Apex 기준 레이를 쏘아 화면 4변 중 가장 가까운 교점 반환.
모든 교점을 검사한 뒤 양수 t 중 최솟값 선택.

---

## 4. 좌표/방향 기준

### 4.1 방향 기준
- 판정/오버레이 모두 `GetVisionForward2D()`를 사용해 동일 축 공유
- 기본은 Actor Forward (XY평면 수평)
- `bUseMeshForwardAsVisionBasis = true`면 Mesh Forward 사용

### 4.2 화면 좌표 기준
- `ProjectWorldLocationToWidgetPosition(..., bUsePositionInViewport=true)` 사용
- 위젯 `Local` 좌표계로 통일하여 X/Y 오프셋 문제 방지
- 카메라 Right/Up 기반 각도 변환으로 카메라 피치/롤에도 대응

---

## 5. 주요 파라미터 (어디서 조정?)

### 5.1 판정 파라미터 (`UDS1VisibilityComponent`)
조정 위치:
- BP 기준: `BP_Player`의 `Visibility` 컴포넌트 디테일
- C++ 기본값: `Source/DS1/Components/DS1VisibilityComponent.h`

파라미터:
| 이름 | 기본값 | 설명 |
|---|---|---|
| `CheckInterval` | `0.15` | 가시성 체크 주기(초) |
| `VisibilityRadius` | `2000` | 콘 판정 최대 거리 (UU) |
| `ConeAngleDegrees` | `90` | 시야 콘 전체 각도 (0~360) |
| `NearDetectionRadius` | `400` | 근접 전방위 감지 반경. `0`이면 비활성 |
| `ForwardYawOffsetDegrees` | `0` | 시야 방향 미세 Yaw 보정 |
| `bUseMeshForwardAsVisionBasis` | `false` | `true`면 메시 축 기준 전방 사용 |

### 5.2 오버레이 파라미터 (`UDS1VisionOverlayWidget`)
조정 위치:
- BP 기준: `WBP_VisionOverlay` 디테일(해당 위젯 클래스)
- C++ 기본값: `Source/DS1/UI/DS1VisionOverlayWidget.h`

스타일/표시:
| 이름 | 기본값 | 설명 |
|---|---|---|
| `DarkColor` | `(0,0,0, α=0.5)` | 시야 밖 어두운 영역 색/알파 |
| `bDrawConeBoundary` | `false` | 시야 콘 경계선(직선+호) 표시 여부 |
| `bDrawNearCircleBoundary` | `false` | 근접 원 경계선 표시 여부 |
| `ConeBoundaryColor` | 연한 노랑 α0.75 | 콘 경계선 색 |
| `NearCircleColor` | 연한 파랑 α0.55 | 근접 원 경계선 색 |
| `LineThickness` | `2.0` | 경계선 두께 |

보정:
| 이름 | 기본값 | 설명 |
|---|---|---|
| `ConeApexOffsetZ` | `0` | 시야 시작점 월드 Z 오프셋. 0이면 발 기준 → 콘이 치우쳐 보임. 80~100 권장 |
| `ForwardAngleCorrectionDeg` | `0` | 화면상 전방 각도 미세 보정. 양수=시계 방향 |
| `ArcStepDeg` | `4` | 호/원 분할 간격. 작을수록 부드럽고 버텍스 증가 |
| `ConeArcRadiusPct` | `0.25` | 콘 외곽 호 반경(뷰포트 짧은 변 비율, 0.05~0.8) |

---

## 6. 자주 쓰는 튜닝 가이드

### 6.1 "적이 너무 빨리 보인다/늦게 보인다"
- `VisibilityRadius` 조정 (콘 감지 거리)
- `NearDetectionRadius` 조정 (뒤쪽 인기척 범위)
- `ConeAngleDegrees` 조정 (시야 폭)

### 6.2 "시야 방향이 캐릭터 정면과 미세하게 다르다"
- `ForwardYawOffsetDegrees` 소폭 조정 (예: `-5 ~ +5`)
- 효과 없으면 `bUseMeshForwardAsVisionBasis` 전환 시도

### 6.3 "오버레이 콘이 캐릭터 위치와 시각적으로 어긋난다"
- `ConeApexOffsetZ`를 80~100 사이로 설정 (허리~가슴 높이 기준)
- 카메라 앵글에 따라 달라지므로 플레이 중 실시간 조정 권장
- 여전히 어긋나면 `ForwardAngleCorrectionDeg` 추가 조정

### 6.4 "경계선 자체를 숨기고 싶다"
- `bDrawConeBoundary = false`
- `bDrawNearCircleBoundary = false`

### 6.5 "오버레이가 끊겨 보인다/각져 보인다"
- `ArcStepDeg`를 줄임 (예: `4` → `2`)
- 단, 버텍스 수가 늘어 렌더링 비용 증가

---

## 7. 디버깅 체크리스트
- BP에서 C++ 기본값을 오버라이드했는지 확인
- Live Coding 사용 시 `Ctrl+Alt+F11`로 반영
- 위젯 클래스가 실제로 `VisionOverlayWidgetClass`에 연결되어 있는지 확인
- 판정은 `ECC_Visibility` 충돌 채널 설정 영향을 받음 (LOS LineTrace)
- `bDataValid = false` 상태면 오버레이가 그려지지 않음
  - PC, Pawn, VisibilityComponent, PlayerCameraManager 중 하나라도 null이면 발생
  - 뷰포트 크기가 0이면 발생

---

## 8. 알려진 제약 및 한계

- **멀티플레이어 미지원**: `GetAllActorsOfClass`로 전체 순회하므로 서버/멀티 환경에서는 별도 구조 필요
- **적 외 오브젝트 미지원**: 현재 `ADS1Enemy`만 판정. 다른 타입에 적용하려면 인터페이스화 필요
- **카메라 방향 고정 가정**: 오버레이 전방 각도는 카메라 Right/Up 기반 계산이므로 카메라가 크게 기울면(예: 360° 회전 카메라) 보정 필요
- **LOS EyeOffset 고정**: 눈높이 보정값 Z+60이 하드코딩. 키가 다른 캐릭터에서는 조정 필요
- **NearRadius 스크린 크기**: 원근 투영으로 변환하므로 카메라 줌/FOV가 바뀌면 시각적 크기도 달라짐 (판정 반경은 변하지 않음)

---

## 9. 관련 파일 목록
- `Source/DS1/Characters/DS1Character.h`
- `Source/DS1/Characters/DS1Character.cpp`
- `Source/DS1/Components/DS1VisibilityComponent.h`
- `Source/DS1/Components/DS1VisibilityComponent.cpp`
- `Source/DS1/UI/DS1VisionOverlayWidget.h`
- `Source/DS1/UI/DS1VisionOverlayWidget.cpp`
