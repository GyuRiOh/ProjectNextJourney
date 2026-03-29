# Stamina Ring Implementation Log (2026-03-29)

## Summary

- 캐릭터 주변에 원형 스태미나 링을 월드 공간 위젯으로 표시
- `UWidgetComponent` + 커스텀 `UDS1StaminaRingWidget`(NativePaint) 조합
- 캐릭터가 어느 방향을 보더라도 항상 **카메라 좌측**에 고정 표시

## 관련 클래스

| 클래스 | 파일 | 역할 |
|---|---|---|
| `UDS1StaminaRingWidget` | `Source/DS1/UI/DS1StaminaRingWidget.h/.cpp` | NativePaint로 링 직접 드로우 |
| `ADS1Character` | `Source/DS1/Characters/DS1Character.h/.cpp` | WidgetComponent 생성·Tick 갱신 |

## 구현 방식

### 위젯 드로우 (DS1StaminaRingWidget)
- `NativePaint` 오버라이드로 Slate `MakeLines`를 사용해 직접 원호 드로우
- 배경 링(360°) + 스태미나 아크(StaminaRatio * 360°) 두 레이어로 구성
- 아크 시작각: -90° (12시 방향), 시계방향
- 세그먼트 수: 64 (부드러운 곡선)

```cpp
// 스태미나 비율 갱신
void UDS1StaminaRingWidget::SetStaminaRatio(float InRatio);
// AttributeComponent->OnAttributeChanged 델리게이트에서 호출
```

### WidgetComponent 설정 (DS1Character 생성자)
```cpp
StaminaRingComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("StaminaRing"));
StaminaRingComponent->SetupAttachment(RootComponent);
StaminaRingComponent->SetWidgetSpace(EWidgetSpace::World);
StaminaRingComponent->SetRelativeLocation(FVector::ZeroVector); // 위치는 Tick에서 계산
StaminaRingComponent->SetDrawSize(FVector2D(150.f, 150.f));
```

### 카메라 기준 위치 고정 (Tick)
```cpp
// 카메라 좌측 방향(월드) = -RightVector
const FVector CameraLeft = -FollowCamera->GetRightVector();
const FVector Origin     = GetActorLocation();
StaminaRingComponent->SetWorldLocation(Origin + CameraLeft * 130.f + FVector(0.f, 0.f, 60.f));

// 링 면이 카메라를 향하도록
const FRotator CamRot = FollowCamera->GetComponentRotation();
StaminaRingComponent->SetWorldRotation(FRotator(-CamRot.Pitch, CamRot.Yaw + 180.f, 0.f));
```

- `RelativeLocation` 고정 대신 매 Tick `SetWorldLocation` 계산
- CameraBoom이 `bInheritYaw = false`라 카메라 Yaw는 사실상 고정이지만, `FollowCamera->GetRightVector()` 사용으로 카메라 회전 변경 시에도 대응 가능

## 크기 조정 파라미터

| 파라미터 | 위치 | 기본값 | 효과 |
|---|---|---|---|
| `SetDrawSize` | DS1Character 생성자 또는 BP_Player | 150×150 | 링 전체 크기 |
| `RingThickness` | DS1StaminaRingWidget UPROPERTY | 10.f | 링 선 두께 |
| Z 오프셋 | Tick SetWorldLocation | 60.f | 링 높이 위치 |
| 좌측 오프셋 | Tick SetWorldLocation | 130.f | 캐릭터에서 좌측 거리 |

## 항상 표시 (지형 관통)

기본 WidgetComponent는 지형에 가려짐. 항상 표시하려면:
1. 에디터에서 머티리얼 생성 (`M_StaminaRing_NoDepth`)
   - Blend Mode: Translucent / Shading Model: Unlit / **Disable Depth Test** 체크
2. BP_Player → StaminaRing 컴포넌트 → Material[0]에 할당

## 스태미나 연동

```cpp
// BeginPlay에서 델리게이트 바인딩
AttributeComponent->OnAttributeChanged.AddLambda([RingWidget](EDS1AttributeType Type, float Ratio)
{
    if (Type == EDS1AttributeType::Stamina)
        RingWidget->SetStaminaRatio(Ratio);
});
```
