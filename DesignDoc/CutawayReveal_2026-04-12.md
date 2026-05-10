# Cutaway Reveal System (2026-04-12)

## 개요

EFT(Escape from Tarkov) 스타일 컷어웨이 시스템.

- 화면에 원형 FOV 마스크가 존재하고 그 밖은 어둡게 처리된다.
- 플레이어가 벽/오브젝트 뒤로 들어가도 FOV 원 안에서는 해당 오브젝트가 지워진 것처럼 플레이어가 보인다.
- HUD(HP·스태미나 등)는 UMG 오버레이라 별도 처리 없이 항상 보인다.

## 구성 요소

| 요소 | 역할 |
| --- | --- |
| `DS1CutawayComponent` | 플레이어 메시 마킹·SceneCapture·PostProcess 관리 |
| `USceneCaptureComponent2D` | 플레이어 메시만 렌더링 → `PlayerCaptureRT` |
| `UTextureRenderTarget2D` (PlayerCaptureRT) | 플레이어 전용 렌더 버퍼 |
| `UPostProcessComponent` | `M_CutawayReveal` 머티리얼을 전체 씬에 적용 |
| `M_CutawayReveal` | 컷어웨이·다크닝 합성 Post Process 머티리얼 |
| CustomDepth Stencil = 2 | 플레이어 픽셀을 화면에서 식별 |

## 에디터 필수 작업 (3단계)

### 1. 프로젝트 설정 — Custom Depth 활성화

> Edit → Project Settings → Rendering → Postprocessing
>
> **Custom Depth-Stencil Pass** → `Enabled with Stencil`

이미 설정되어 있다면 스킵.

---

### 2. M_CutawayReveal 머티리얼 생성

`Content/_Game/Materials/` 에 **Post Process Material** 을 아래 사양으로 생성한다.

#### 머티리얼 설정

| 설정 | 값 |
| --- | --- |
| Material Domain | Post Process |
| Blendable Location | After Tonemapping |
| Output Node | Emissive Color |

#### 파라미터 목록

| 파라미터 이름 | 타입 | 기본값 | 설명 |
| --- | --- | --- | --- |
| `PlayerCaptureTex` | Texture2D | *(없음)* | 플레이어 전용 렌더타깃 (C++ 에서 자동 주입) |
| `FOVRadius` | Scalar | 0.35 | FOV 원 반지름 (화면 높이 대비 0~1) |
| `FOVEdgeSoftness` | Scalar | 0.06 | 원 경계 그라데이션 폭 |
| `DarkOutside` | Scalar | 0.65 | FOV 밖 다크닝 강도 (0=없음, 1=완전 검정) |
| `PlayerScreenPos` | Vector | (0.5, 0.5, 1.778, 0) | XY=플레이어 UV, Z=화면 종횡비 (C++ 에서 매 프레임 갱신) |

#### 노드 그래프 — 단계별 설명

#### Step 1 — 씬 색 가져오기

```text
[SceneTexture: PostProcessInput0] → SceneColor (RGB)
```

#### Step 2 — 플레이어 캡처 색 가져오기

```text
[TextureSampleParameter2D "PlayerCaptureTex"]
    UV 입력: [ScreenPosition (Normalized 0-1)]
→ PlayerColor (RGB)
```

#### Step 3 — 깊이·스텐실 가져오기

```text
[SceneTexture: SceneDepth]   → SceneDepth
[SceneTexture: CustomDepth]  → CustomDepth
[SceneTexture: CustomStencil] → CustomStencil
```

#### Step 4 — FOV 원형 마스크 계산

```text
[ScreenPosition (Normalized)]           → PixelUV

[VectorParameter "PlayerScreenPos"]
    ComponentMask R → PlayerUV_X
    ComponentMask G → PlayerUV_Y
    ComponentMask B → AspectRatio

Subtract(PixelUV.R, PlayerUV_X)         → dX
Subtract(PixelUV.G, PlayerUV_Y)         → dY
Divide(dX, AspectRatio)                 → dX_corr   ← 가로 왜곡 보정

Multiply(dX_corr, dX_corr)              → dX2
Multiply(dY, dY)                        → dY2
Add(dX2, dY2)                           → dist2
Sqrt(dist2)                             → dist

[ScalarParameter "FOVRadius"]           → R
[ScalarParameter "FOVEdgeSoftness"]     → S
Subtract(R, S)                          → EdgeInner
Add(R, S)                               → EdgeOuter

[SmoothStep] Min=EdgeInner, Max=EdgeOuter, Value=dist → EdgeFade
  (0 = FOV 원 안, 1 = FOV 원 밖)

[OneMinus(EdgeFade)]                    → FOVMask
  (1 = 원 안, 0 = 원 밖)
```

#### Step 5 — FOV 밖 다크닝

```text
[ScalarParameter "DarkOutside"]         → D
OneMinus(FOVMask)                       → OutsideFactor
Multiply(OutsideFactor, D)              → DarkAmount
OneMinus(DarkAmount)                    → DarkMultiplier
  (FOV 안 = 1.0, FOV 밖 = 1-D)
```

#### Step 6 — 플레이어 가려짐 감지

```text
[If] A=CustomStencil, B=1.5
    A > B → 1.0
    A ≤ B → 0.0
→ isPlayer   (Stencil 2 이상 = 플레이어)

[If] A=CustomDepth, B=SceneDepth
    A > B → 1.0   ← CustomDepth > SceneDepth = 플레이어 뒤에 벽이 있음
    A ≤ B → 0.0
→ isOccluded

Multiply(isPlayer, isOccluded)          → tempReveal
Multiply(tempReveal, FOVMask)           → shouldReveal
  (FOV 원 안 + 플레이어가 가려진 픽셀에서만 1)
```

#### Step 7 — 최종 합성

```text
[Lerp] A=SceneColor, B=PlayerColor, Alpha=shouldReveal
→ RevealedColor
  (가려진 픽셀에서 플레이어 캡처 색으로 대체)

Multiply(RevealedColor, DarkMultiplier)
→ FinalColor

FinalColor → [Emissive Color 출력 핀]
```

---

### 3. BP_Player 에 머티리얼 지정

1. 언리얼 에디터에서 `BP_Player` 열기
2. 컴포넌트 패널 → `Cutaway` (`DS1CutawayComponent`) 선택
3. 디테일 패널 → **Cutaway → CutawayMaterial** 슬롯에 `M_CutawayReveal` 드래그

나머지 파라미터(`FOVRadius`, `DarkOutsideFOV` 등)도 이 패널에서 조정 가능.

---

## C++ 파라미터 튜닝 가이드

| 프로퍼티 | 추천 범위 | 효과 |
| --- | --- | --- |
| `FOVRadiusFraction` | 0.25 ~ 0.45 | 값이 클수록 밝은 원이 커짐 |
| `FOVEdgeSoftness` | 0.04 ~ 0.12 | 작을수록 경계가 선명 |
| `DarkOutsideFOV` | 0.5 ~ 0.8 | 클수록 밖이 더 어두움 |
| `CaptureResolutionFraction` | 0.5 | 성능과 품질의 균형점 |

## 동작 원리 요약

```text
메인 카메라 렌더 ─────────────────────────────┐
                                              │
SceneCaptureComponent2D                       │  PostProcess 머티리얼
  (플레이어 메시만, 동일 FOV·위치)              │  ┌─────────────────────────────────┐
  → PlayerCaptureRT (렌더타깃)                  │  │ FOV 원 계산 (플레이어 스크린 위치) │
                                              │  │ + CustomDepth 가려짐 감지         │
플레이어 메시 → CustomDepth Stencil=2 ─────────┤  │ → 가려진 픽셀: PlayerCaptureRT 색 │
                                              └─►│ → FOV 밖: 씬 색 다크닝            │
                                                 └─────────────────────────────────┘
```

## 주의 사항

- `DS1VisionOverlayWidget` 의 다크 오버레이와 이 시스템의 다크닝이 **중첩 적용**된다.
  VisionOverlayWidget 의 `DarkColor.A` 를 낮추거나 `DS1CutawayComponent.DarkOutsideFOV = 0` 으로 설정해 중복을 제거할 것.
- 적(Enemy) 은 Stencil 마킹이 없으므로 컷어웨이에 영향받지 않는다.
- 두 개의 렌더 패스(메인 + SceneCapture) 가 추가되므로 성능 민감 시 `CaptureResolutionFraction = 0.25` 로 낮출 것.

## 관련 파일

- `Source/DS1/Components/DS1CutawayComponent.h/.cpp` — 시스템 핵심 컴포넌트
- `Source/DS1/Characters/DS1Character.cpp` — `CutawayComponent` 생성 (생성자)
- `Content/_Game/Materials/M_CutawayReveal` — Post Process 머티리얼 (에디터 생성)

---

## 디버깅 기록 (2026-05-03)

### 최초 증상 (3가지)

1. **자글거림 (shimmer)** — 캐릭터 실루엣이 매 프레임 떨림
2. **특정 부위 어둡게** — 팔·다리·발이 검게 표시됨
3. **머리카락 날아감** — reveal 영역에서 헤어가 분리되어 표시

---

### 해결된 것: 자글거림

**원인**: SceneCapture에 `SetAntiAliasing(true)` 설정 시 TAA 히스토리 없이 매 프레임 픽셀 지터 발생.
추가로 머티리얼의 `CustomDepth > SceneDepth` 비교에 tolerance 없이 경계 픽셀이 매 프레임 occluded/non-occluded를 오가며 플리커.

**적용된 수정**:

- `DS1CutawayComponent.cpp` — `SetAntiAliasing(false)`, `SetMotionBlur(false)`
- `M_CutawayReveal` (머티리얼) — Step 6 occlusion 판정을 `CustomDepth > SceneDepth` 에서
  `Subtract(CustomDepth, SceneDepth) → Add B=5.0 → If A > B` 로 변경 (5cm 깊이 허용치 추가)

---

### 미해결: 팔·다리·발 검정 표시

**증상**: 캐릭터가 벽 뒤로 들어가 reveal될 때 상체(빨간 셔츠)는 정상이지만 하체(어두운 바지·발)가 검정으로 표시됨.

**원인 분석 (결론 미확정)**: `shouldReveal=1` 인데 `PlayerCaptureRT`가 해당 UV에서 검정을 반환하는 것으로 추정.
즉 SceneCapture가 하체 메시를 정상 렌더링하지 못하고 있을 가능성이 높음.

**시도했으나 효과 없는 것들**:

| 시도 | 결과 |
| --- | --- |
| `RTF_RGBA8` → `RTF_RGBA16f` | 오히려 악화. `SCS_FinalColorLDR`와 float16 조합 시 gamma mismatch로 어두운 색상이 블랙으로 수렴. **RGBA8으로 복원** |
| `PostProcessComponent(bUnbound)` → `Camera->AddOrUpdateBlendable` | 효과 없음. 복원 |
| `bAlwaysPersistRenderingState = true` 추가 | 효과 없음. 제거 |
| `StaticMeshComponent` 스텐실 마킹 추가 | 효과 없음. 제거 |

**다음 세션에서 시도할 것**:

- PIE 중 Content Browser에서 `PlayerCutawayRT` 렌더타깃 미리보기로 SceneCapture 출력 직접 확인
  → RT에서도 검정이면 SceneCapture 렌더 문제, 정상이면 머티리얼 depth/stencil 판정 문제
- `DS1Character`의 `LegsMesh` / `FeetMesh` 가 `SetMasterPoseComponent` 사용 여부 확인
  → SceneCapture에서 MasterPose 컴포넌트(`GetMesh()`)가 올바르게 포함되는지 점검
- SceneCapture `ShowFlags`에 추가로 비활성화해야 할 플래그 확인 (Lumen GI, Ray Tracing 등)
  → `SCS_FinalColorLDR`가 Lumen GI를 SceneCapture에서 올바르게 처리하지 못할 경우 하체 암부 발생 가능
- 머티리얼에서 `shouldReveal` 마스크를 상시 1로 고정 후 `PlayerCaptureTex`를 전체 화면에 출력,
  SceneCapture 내용 확인 (디버그용 임시 수정)

**현재 코드 상태 (2026-05-03 기준)**:

```text
RTF_RGBA8 (원복)
SetAntiAliasing(false)     ← 추가됨 (자글거림 수정)
SetMotionBlur(false)       ← 추가됨
PostProcessComponent bUnbound Priority=5 (원복)
SkeletalMeshComponent 스텐실 마킹만 유지 (StaticMesh 마킹 제거)
M_CutawayReveal Step6: depth bias 5.0 적용 (자글거림 수정)
```

---

## 디버깅 기록 (2026-05-10)

### 상태 요약

오늘 테스트 기준으로 기존 Cutaway Reveal 방식은 아직 만족스럽게 해결되지 않았다.

현재 구조:

```text
SceneCaptureComponent2D
  -> PlayerCaptureRT
  -> M_CutawayReveal에서 PlayerCaptureTex로 샘플링
  -> CustomDepth/CustomStencil/FOVMask로 벽 뒤 플레이어 픽셀에 합성
```

남은 문제:

```text
플레이어가 벽 뒤에서 reveal될 때 팔, 다리, 머리 주변 외곽이 자글자글하거나 검게 먹는 느낌이 남는다.
밝기, RT 해상도, SceneCapture AA, 단순 depth SmoothStep 보정으로는 해결되지 않았다.
```

### 오늘 적용한 C++ 변경

파일:

```text
Source/DS1/Components/DS1CutawayComponent.h
Source/DS1/Components/DS1CutawayComponent.cpp
```

#### 1. 장비/부착 액터 메시 포함

`RefreshCutawayPrimitives()`를 추가했다.

동작:

```text
Owner의 모든 UMeshComponent 수집
Owner에 Attach된 Actor들의 모든 UMeshComponent 수집
각 MeshComponent에 CustomDepth Stencil = 2 적용
SceneCapture ShowOnlyComponent 목록에 추가
```

목적:

```text
ADS1Armour처럼 별도 액터로 붙는 방어구, 무기, 방패가 PlayerCaptureRT에 빠지는 문제를 막기 위함.
```

결과:

```text
장비 누락 가능성은 줄였지만, 자글자글한 외곽 문제 자체는 해결되지 않음.
```

#### 2. CaptureSource 변경 실험

기존:

```cpp
PlayerCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
```

실험:

```cpp
PlayerCapture->CaptureSource = ESceneCaptureSource::SCS_BaseColor;
```

결과:

```text
캐릭터는 밝아졌지만 팔/장갑 일부가 하얗게 튀는 문제가 생김.
BaseColor는 최종 컷어웨이용으로 부적합.
```

현재 유지 후보:

```cpp
PlayerCapture->CaptureSource = ESceneCaptureSource::SCS_FinalToneCurveHDR;
```

결과:

```text
SCS_FinalColorLDR보다 덜 어둡고, SCS_BaseColor의 하얀 튐도 줄어듦.
오늘 테스트 중 가장 나은 CaptureSource.
```

#### 3. SceneCapture AA 재실험

실험:

```cpp
PlayerCapture->ShowFlags.SetAntiAliasing(true);
PlayerCapture->bAlwaysPersistRenderingState = true;
```

결과:

```text
자글자글함 변화 없음.
이전 shimmer 위험도 있으므로 다시 SetAntiAliasing(false)로 되돌림.
```

### 오늘 적용/시도한 머티리얼 변경

파일:

```text
Content/_Game/Materials/M_CutawayReveal
```

#### 1. PlayerCaptureTex 직접 출력

목적:

```text
PlayerCaptureRT 원본이 찍히는지 확인.
```

확인:

```text
PlayerCaptureTex에는 캐릭터가 찍히고 있었음.
다만 검은 배경 + 플레이어 구조라 외곽에서 검은 배경이 섞일 가능성이 있음.
```

#### 2. PlayerCaptureTex 밝기 보정

구조:

```text
PlayerCaptureTex RGB
  -> Multiply 1.08 ~ 1.5
  -> Lerp B
```

결과:

```text
밝기만 변함.
자글자글한 외곽/검은 픽셀 문제는 해결되지 않음.
```

#### 3. isOccluded depth 판정 softening 시도

기존:

```text
CustomDepth > SceneDepth + 5 ? 1 : 0
```

시도:

```text
CustomDepth - SceneDepth
  -> SmoothStep(Min, Max)
  -> isOccludedSoft
```

테스트 값:

```text
Min=5 Max=20
Min=0 Max=5
Min=0 Max=10
```

결과:

```text
컷어웨이가 꺼진 것처럼 보임.
값을 바꿔도 reveal이 복구되지 않음.
현재 그래프의 depth 값 범위/방향/비선형성 때문에 단순 SmoothStep 치환은 실패.
원래 If 방식으로 복구 필요.
```

#### 4. 검은 RT 배경 완화용 Max 보정 시도

구조:

```text
SceneColor RGB -> Lerp A

PlayerCaptureTex RGB * 1.08 -> Max A
SceneColor RGB * 0.25       -> Max B
Max Output                  -> Lerp B

shouldReveal -> Lerp Alpha
```

목적:

```text
PlayerCaptureTex의 검은 배경이 캐릭터 외곽에 섞일 때 완전 검정으로 떨어지지 않게 함.
```

결론:

```text
근본 해결책은 아님.
검은 테두리 강도 완화는 가능할 수 있으나, 자글자글한 외곽 자체를 제거하지는 못함.
```

### 오늘 결론

현재 방식의 근본 한계:

```text
PlayerCaptureRT는 검은 배경 위에 플레이어만 렌더링한다.
M_CutawayReveal은 CustomDepth/Stencil 마스크로 그 RT를 벽 위에 합성한다.
캐릭터 외곽에서는 RT의 검은 배경과 캐릭터 색이 섞인다.
CustomDepth/Stencil 마스크와 PlayerCaptureTex 실루엣도 완벽히 일치하지 않는다.
그 결과 팔/다리/머리 주변에 검은 픽셀 또는 자글자글한 외곽이 남는다.
```

### 다음 재설계 후보

#### 후보 A: CustomStencil 기반 단색/림 reveal

```text
CustomDepth/Stencil로 가려진 플레이어 영역 감지
-> 부드러운 실루엣/림/반투명 단색으로 표시
```

장점:

```text
검은 RT 배경 문제 없음
SceneCapture 비용 없음
외곽 자글자글함을 스타일로 감출 수 있음
구현과 디버깅이 단순
```

단점:

```text
실제 캐릭터 색/장비 색이 보이지 않음
```

#### 후보 B: 벽/장애물 쪽을 투명화하는 cutaway

```text
카메라-플레이어 사이 장애물 검출
-> 해당 장애물 머티리얼을 dither/fade/cutout
-> 실제 메인 렌더의 플레이어를 그대로 보이게 함
```

장점:

```text
플레이어 색/장비/머리카락이 원래 렌더 그대로 보임
RT 합성 불일치 없음
```

단점:

```text
장애물 머티리얼 교체/관리 필요
레벨 오브젝트 처리 규칙 필요
```

#### 후보 C: SceneCapture + Alpha Mask를 별도 생성

```text
PlayerColorRT: 플레이어 색
PlayerMaskRT: 플레이어 실루엣 alpha
합성: Lerp(SceneColor, PlayerColor, PlayerMask * isOccluded * FOVMask)
```

장점:

```text
검은 배경 RGB가 외곽에 섞이는 문제를 줄일 수 있음
```

단점:

```text
RT/머티리얼/SceneCapture 구성이 더 복잡
UE PostProcess에서 안정적인 mask 생성 방식 검토 필요
```

### 현재 권장 임시 상태

```text
CaptureSource = SCS_FinalToneCurveHDR
ShowFlags AntiAliasing = false
MotionBlur = false
RenderTargetFormat = RTF_RGBA8
CustomDepth/Stencil 판정은 기존 If 방식 유지
PlayerCaptureTex 밝기 보정은 1.0 ~ 1.08 정도만 사용
SmoothStep depth softening은 사용하지 않음
```
