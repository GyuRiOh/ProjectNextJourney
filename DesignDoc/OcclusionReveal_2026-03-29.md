# Occlusion Reveal System (2026-03-29)

## Summary

- 플레이어 캐릭터가 벽/오브젝트 뒤에 있어도 항상 표시되는 시스템
- **Custom Depth Stencil** + **Post Process Material** 조합으로 구현
- NPC는 마킹하지 않으므로 기존대로 벽에 가려짐
- 스태미나 링 위젯도 동일 원리(`Disable Depth Test`)로 벽 뒤 표시

## 구성 요소

| 요소 | 역할 |
| --- | --- |
| `GetMesh()->SetRenderCustomDepth(true)` | 플레이어 메시를 Custom Depth 패스에 포함 |
| `SetCustomDepthStencilValue(1)` | Stencil 값 1 = 플레이어 식별자 |
| `M_OcclusionReveal` | Post Process Material — 가려진 픽셀에 실루엣 표시 |
| `PostProcessVolume` | 레벨 전체에 M_OcclusionReveal 적용 (Infinite Extent) |
| `M_StaminaRing_NoDepth` | 스태미나 링 머티리얼 — Disable Depth Test로 항상 표시 |

## C++ 변경 (DS1Character.cpp — BeginPlay)

```cpp
// 플레이어 메시를 Custom Depth Stencil로 마킹 → Post Process에서 벽 뒤 실루엣 표시
GetMesh()->SetRenderCustomDepth(true);
GetMesh()->SetCustomDepthStencilValue(1);
```

## 에디터 설정 순서

### 1. 프로젝트 세팅

- **Edit → Project Settings → Rendering → Postprocessing**
- **Custom Depth-Stencil Pass** → `Enabled with Stencil`

### 2. M_OcclusionReveal (Post Process Material)

- **Material Domain**: `Post Process`
- **Blendable Location**: `Before Translucency`

노드 구성:

```
SceneTexture(PostProcessInput0) ──────────────────── If(A>=B) ──┐
SceneTexture(CustomDepth)       ── A ─┐                          ├── Emissive Color
SceneTexture(SceneDepth)        ── B ─┴── If ── A<B → 실루엣색 ──┘
```

- `CustomDepth < SceneDepth` = 플레이어가 벽 뒤에 있는 픽셀 → 실루엣 색 출력
- 그 외 픽셀 → 원본 씬 색 그대로 출력

### 3. Post Process Volume

- 레벨에 배치 → **Infinite Extent (Unbound)** 체크
- **Post Process Materials → Array** → `M_OcclusionReveal` 할당

### 4. M_StaminaRing_NoDepth (스태미나 링 머티리얼)

- **Blend Mode**: `Translucent` / **Shading Model**: `Unlit`
- **Disable Depth Test**: 체크
- 노드: `VertexColor.RGB` → Emissive, `VertexColor.A` → Opacity
- BP_Player → StaminaRing 컴포넌트 → Material[0] 에 할당

## 설계 의도

- NPC(`ADS1Enemy`)는 `SetRenderCustomDepth` 호출 없음 → 벽에 가려짐 유지
- 플레이어만 Stencil=1로 마킹하여 Post Process에서 선택적으로 표시
- 스태미나 링은 World Widget이므로 별도로 Depth Test 비활성화 필요

## 관련 파일

- `Source/DS1/Characters/DS1Character.cpp` — BeginPlay에서 Custom Depth 설정
- `Content/_Game/Materials/M_OcclusionReveal` — Post Process 머티리얼
- `Content/_Game/Materials/M_StaminaRing_NoDepth` — 스태미나 링 머티리얼
