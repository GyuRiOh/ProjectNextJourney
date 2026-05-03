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
