# MIR (Modern Interface & Runtime) Engine - 한국어 문서

C++20 및 SDL3를 기반으로 작성된 2D 게임 엔진입니다. 내부 저장 공간을 할당하지 않는 `zet` 컨테이너 패키지와 세대 안전성을 갖춘 엔티티-컴포넌트(ECS) 패턴을 기반으로 설계되었습니다. Lua, SDL 리소스, 파일 입출력 및 일부 표준 라이브러리 타입은 런타임 힙 할당을 수행할 수 있습니다.

---

## 주요 특징
- **C++20 기반 설계:** 최신 C++ 기능 및 SIMD 최적화 수학 모듈 활용
- **SDL3 런타임 연동:** 크로스 플랫폼 창 생성, 입력 폴링, 2D 그래픽 렌더링, `SDL3_mixer` 오디오 탑재
- **ECS (Entity-Component-System) 패턴:** `SparseSet` 기반 컴포넌트 저장소, 세대 검증 및 Command Buffer 기반 지연 처리 구조
- **Lua 스크립팅 통합:** `sol2` 라이브러리를 활용하여 런타임에 게임의 컴포넌트 제어, 사운드, 입력, 물리, 충돌을 제어 가능
- **공통 개발 표면:** `MIR.hpp`를 통해 C++와 Lua가 동일한 세대 안전 ECS·자산·시스템 기능을 사용하며, Lua에는 내부 컨테이너 핸들과 SDL 포인터를 노출하지 않음
- **최신 ZET 연동:** Xmake가 빌드 시 `zetcontainer-cpp`의 GitHub `main` 브랜치를 받아 현재 컨테이너 API로 빌드
- **빌드 자동화 및 CI:** Xmake 및 GitHub Actions를 통해 Windows, Linux, macOS에 대한 빌드 및 테스트 자동화

---

## 프로젝트 폴더 구조
```text
mirengine-cpp/
├── .github/workflows/      # GitHub Actions CI 설정
├── build/                  # 빌드 출력 디렉토리
├── engine/                 # 엔진 핵심 소스 코드
│   ├── asset/              # 리소스(텍스처, 사운드, 폰트, 애니메이션) 관리 인터페이스
│   ├── component/          # ECS 컴포넌트 정의 (Transform, Sprite 등)
│   ├── core/               # 엔진의 핵심 ECS 매니저 및 엔티티 핸들링
│   ├── device/             # 창(Window), 입력(Input), 키(Key) 제어 인터페이스
│   ├── math/               # SIMD 연산 및 행렬/벡터 수학 라이브러리
│   ├── script/             # Lua 스크립팅 바인딩 및 런타임 인터프리터 시스템
│   ├── sdl/                # SDL3를 활용한 추상화 선언의 실제 구현체
│   ├── system/             # 이벤트, 물리 및 충돌 판단과 같은 기본 서브시스템
│   ├── test/               # 단위 테스트 코드 (TestRunner.cpp)
│   └── xmake.lua           # 엔진 라이브러리 및 테스트 타겟 빌드 설정
└── xmake.lua               # 프로젝트 루트 빌드 구성 파일
```

---

## Lua 스크립팅 API 가이드
엔진은 `main.lua` 파일을 진입점으로 실행하며, Lua 스크립트 상에서 다음 API들을 직접 다룰 수 있습니다:

### 1. ECS 및 생명주기 관리
- `Manager.Instance()`: 매니저 객체 반환
- `Manager.Instance():AddEntity()` -> `Id`: 신규 엔티티 ID 생성
- `Manager.Instance():DeleteEntity(id)`: 엔티티 삭제
- `Manager.Instance():IsValidEntity(id)` -> `bool`: 엔티티가 살아있는지 검사
- `Manager.Instance():ForEachEntity(function(id) ... end)` -> `bool`: 내부 저장소 핸들을 노출하지 않고 유효 엔티티 순회

### 2. 컴포넌트 API
- **Transform** (위치 및 크기):
  - `Transform.SetPosition(id, x, y)` / `Transform.GetPositionX(id)` / `Transform.GetPositionY(id)`
  - `Transform.SetRotation(id, r)` / `Transform.GetRotation(id)`
  - `Transform.SetScale(id, s)` / `Transform.GetScale(id)`
  - `Transform.GetWorldPositionX(id)` / `Transform.GetWorldPositionY(id)` / `Transform.GetWorldRotation(id)` / `Transform.GetWorldScale(id)`
  - `Transform.IsValid(id)` / `Transform.Remove(id)`
- **Sprite** (렌더링 리소스):
  - `Sprite.SetTexture(id, "path")` / `Sprite.GetTexture(id)`
  - `Sprite.SetSourceSize(id, w, h)` / `Sprite.SetDestinationSize(id, w, h)`
  - `Sprite.SetSourceRect(id, x, y, w, h)` (스프라이트 시트 프레임 선택)
  - `Sprite.SetAnchor(id, x, y)` / `Sprite.SetTint(id, r, g, b)`
  - `Sprite.SetZindex(id, z)` / `Sprite.GetZindex(id)`
  - `Sprite.SetAlpha(id, a)` / `Sprite.GetAlpha(id)`
  - `Sprite.IsValid(id)` / `Sprite.Remove(id)`
- **Rigidbody** (물리 설정):
  - `Rigidbody.SetVelocity(id, vx, vy)` / `Rigidbody.GetVelocityX(id)` / `Rigidbody.GetVelocityY(id)`
  - `Rigidbody.SetGravity(id, g)` / `Rigidbody.GetGravity(id)`
  - `Rigidbody.SetOnGround(id, bool)` / `Rigidbody.IsOnGround(id)`
  - `Rigidbody.IsValid(id)` / `Rigidbody.Remove(id)`
- **Collider** (충돌체 설정):
  - `Collider.SetBound(id, w, h)` / `Collider.GetBoundX(id)` / `Collider.GetBoundY(id)`
  - `Collider.SetOffset(id, x, y)` / `Collider.GetOffsetX(id)` / `Collider.GetOffsetY(id)`
  - `Collider.SetShouldTrigger(id, bool)` / `Collider.GetShouldTrigger(id)`
  - `Collider.IsValid(id)` / `Collider.Remove(id)`
- **Tag** (엔티티 분류 태그):
  - `Tag.Set(id, "TagName")` / `Tag.Get(id)` / `Tag.IsValid(id)` / `Tag.Remove(id)`
- **Hierarchy** (ECS 부모-자식 관계):
  - `Hierarchy.SetParent(child, parent)` / `Hierarchy.ClearParent(child)`
  - `Hierarchy.GetParent(id)` / `Hierarchy.GetFirstChild(id)` / `Hierarchy.GetNextSibling(id)`
  - 관계 변경은 Command Buffer를 통해 반영되며, 부모 삭제 시 자식은 월드 루트로 분리됩니다.

### 3. 디바이스 입력 및 사운드
- **Input & Key**:
  - `Input.IsPressed(Key.W)` / `Input.IsJustPressed(Key.Space)` / `Input.IsJustReleased(Key.Enter)`
  - `Key.F1`~`Key.F12`, `Input.IsMousePressed(MouseButton.Left)`, `Input.IsMouseJustPressed(...)`, `Input.IsMouseJustReleased(...)`
  - `Input.GetMouseX()` / `Input.GetMouseY()`
- **Sound**:
  - `Sound.Load("path")` / `Sound.Play("name", vol, pitch)` / `Sound.PlayBgm("name", vol, loop)` / `Sound.StopBgm()` / `Sound.StopAll()`

### 4. GPU 쉐이더 (SDL_GPU API)
엔진에 연동된 SDL3.0 GPU 서브시스템을 통해 커스텀 쉐이더 및 파이프라인을 생성할 수 있습니다.
* **디바이스 관리**:
  * `GPU.CreateDevice(formats, debugMode)` -> `GPUDevice`
  * `device:Destroy()` / `device:ClaimWindow()` / `device:ReleaseWindow()` / `device:GetSwapchainFormat()`
  * Lua에는 SDL 포인터 주소를 노출하지 않습니다.
* **상수 및 포맷**:
  * `GPUShaderStage.Vertex` / `GPUShaderStage.Fragment`
  * `GPU_SHADERFORMAT_SPIRV` / `GPU_SHADERFORMAT_DXIL` / `GPU_SHADERFORMAT_MSL`
* **Shader & GPUPipeline**:
  * `Shader.new()`: 신규 쉐이더 객체 생성
    * `Shader:LoadFromFile(device, filepath, entrypoint, stage, numSamplers, numUniformBuffers)` -> `bool`
    * `Shader:Destroy()`
  * `GPUPipeline.new()`: 신규 파이프라인 생성
    * `GPUPipeline:Create(device, vs, fs, renderTargetFormat)` -> `bool`
    * `GPUPipeline:Destroy()`

### 5. 서브시스템
- `Movement.Update(id, deltaTime)`: 물리 이동 업데이트 적용
- `Collision.Update(lhsId, rhsId)` -> `bool`: AABB 박스 충돌 판단 검사
- `Animation.Register(name, { AnimationFrame.new(x, y, width, height), ... })` / `Animation.Play(id, name, speed, loop)` / `Animation.Stop(id)`
- `Resource.Register(name, path)` / `Resource.GetPath(name)` / `Resource.Unregister(name)`
- `Scene.Register(name, callback)` / `Scene.Load(name)`
- `Event.On(name, callback)` / `Event.Emit(id, name)`: Simulation 단계에서 자동 전달
- `Timer.After(seconds, callback)` / `Timer.Every(seconds, callback)` / `Timer.Cancel(handle)`
- `System.Register(callback, SystemPhase.Simulation|PostCommit)` / `System.AddDependency(before, after)`: 고정 용량 Lua 시스템 순서 그래프
- `Profiler`, `Debug`, `Border`, `Label`, `Button`, `Camera`(위치·추적·줌·흔들기), 런타임 안전 `Window` 설정도 Lua에서 사용 가능합니다.

### 6. C++ 개발 API

`engine/MIR.hpp`를 포함하면 Lua와 같은 세대 안전 API를 C++에서도 사용할 수 있습니다. C++ 시스템은 `core::Manager::RegisterSystem`으로, Lua 시스템은 별도의 고정 용량 dispatcher로 등록하므로 서로의 내부 핸들을 만들거나 해석할 수 없습니다.

```cpp
#include "MIR.hpp"

auto& manager = mir::core::Manager::Instance();
const mir::Id player = manager.AddEntity();
if (!mir::transform::SetPosition(player, 100.f, 80.f)) {
    // 명령 버퍼가 가득 찼거나 엔티티가 유효하지 않습니다.
}
mir::event::On("spawn", [](mir::Id id) { /* ... */ });
mir::event::Emit(player, "spawn");
```

구조 변경 setter는 모두 `bool`을 반환합니다. `true`는 명령이 큐에 들어갔다는 뜻이며, 값은 다음 `Manager::UpdateSystem()` commit 이후 관찰됩니다.

### 7. 실행 모델과 API 경계

- `Manager::RegisterSystem(fn, SystemPhase::Simulation|PostCommit)`은 C++ 시스템을 등록하고, `AddSystemDependency(before, after)`로 같은 단계 안의 실행 순서를 고정합니다. 단계 사이에는 commit 장벽이 있으므로 교차 단계 의존성은 허용되지 않습니다.
- `Event.Emit`과 `Timer.After`/`Timer.Every`는 각각 Simulation 시스템에 자동 연결됩니다. 매 프레임 수동으로 `Event.Update`나 `Timer.Update`를 호출하지 마십시오.
- `Resource`와 `Scene`은 고정 용량 레지스트리입니다. `Register`/`Load`의 `false`는 이름 오류·용량 초과·등록되지 않은 항목을 뜻합니다. `Resource.Clear`, `Scene.Clear`, `Timer.Clear`는 게임 종료나 명시적 재시작 경계에서 사용합니다.
- `GPUDevice`는 이동 전용 SDL GPU 디바이스 소유자입니다. C++ 저수준 통합에서만 `Raw()`를 사용하며, Lua는 소유형 userdata만 전달합니다.
- `Window.Init`, 렌더 시작/표시, 이벤트 펌프, 종료는 `main.cpp`의 루프가 소유합니다. Lua에는 창 제목·크기·모드·해상도·FPS·닫기처럼 프레임 안전한 제어만 노출됩니다.
- 비디오 재생은 아직 기능이 아닙니다. `runtime/sdl/asset/Video.hpp`는 디코더 백엔드가 없는 레거시 선언이며, FFmpeg 등의 디코더와 배포 라이선스를 선택하기 전까지 C++/Lua 공개 API로 지원하지 않습니다.

### 8. 게임 개발 코드 예시 (main.lua)
```lua
local player = nil
local ground = nil
local speed = 250.0
local jumpForce = -600.0

function Init()
    -- 리소스 사전 로딩 및 BGM 재생
    Texture.Load("assets/textures/player.png")
    Texture.Load("assets/textures/tiles.png")
    Sound.Load("assets/sounds/jump.wav")
    Sound.Load("assets/sounds/bgm.mp3")
    Sound.PlayBgm("assets/sounds/bgm.mp3", 50.0, true)

    local manager = Manager.Instance()

    -- 플레이어 세팅
    player = manager:AddEntity()
    Transform.SetPosition(player, 100.0, 100.0)
    Sprite.SetTexture(player, "assets/textures/player.png")
    Sprite.SetSourceSize(player, 64.0, 64.0)
    Sprite.SetDestinationSize(player, 64.0, 64.0)
    Sprite.SetAnchor(player, 0.5, 0.5)
    Rigidbody.SetVelocity(player, 0.0, 0.0)
    Rigidbody.SetGravity(player, 980.0)
    Rigidbody.SetOnGround(player, false)
    Collider.SetBound(player, 50.0, 60.0)
    Collider.SetOffset(player, -25.0, -30.0)
    Collider.SetShouldTrigger(player, true)

    -- 바닥 세팅
    ground = manager:AddEntity()
    Transform.SetPosition(ground, 0.0, 500.0)
    Sprite.SetTexture(ground, "assets/textures/tiles.png")
    Sprite.SetSourceSize(ground, 800.0, 50.0)
    Sprite.SetDestinationSize(ground, 800.0, 50.0)
    Collider.SetBound(ground, 800.0, 50.0)
    Collider.SetShouldTrigger(ground, true)
end

function Update(deltaTime)
    local vx = 0.0
    local vy = Rigidbody.GetVelocityY(player)

    if Input.IsPressed(Key.A) or Input.IsPressed(Key.Left) then
        vx = -speed
    elseif Input.IsPressed(Key.D) or Input.IsPressed(Key.Right) then
        vx = speed
    end

    if Input.IsJustPressed(Key.Space) and Rigidbody.IsOnGround(player) then
        vy = jumpForce
        Rigidbody.SetOnGround(player, false)
        Sound.Play("assets/sounds/jump.wav", 100.0, 1.0)
    end

    Rigidbody.SetVelocity(player, vx, vy)
    Movement.Update(player, deltaTime)

    -- 충돌 처리
    if Collision.Update(player, ground) then
        if vy >= 0.0 then
            Rigidbody.SetOnGround(player, true)
            Rigidbody.SetVelocity(player, vx, 0.0)
            Transform.SetPosition(player, Transform.GetPositionX(player), Transform.GetPositionY(ground) - 30.0)
        end
    end
end

function Shutdown()
    Sound.StopAll()
end
```

### 9. 커스텀 컴포넌트 & 시스템 설계 예시
```lua
-- Lua 커스텀 컴포넌트 테이블
local HealthComponent = {}
local EnemyAIComponent = {}

function AddHealthComponent(entity, hp)
    HealthComponent[entity] = { hp = hp, maxHp = hp }
end

function AddEnemyAI(entity, targetX)
    EnemyAIComponent[entity] = { state = "Patrol", targetX = targetX }
end

-- 루아 커스텀 시스템
function HealthSystem(deltaTime)
    for entityId, health in pairs(HealthComponent) do
        if not Manager.Instance():IsValidEntity(entityId) or health.hp <= 0 then
            Manager.Instance():DeleteEntity(entityId)
            HealthComponent[entityId] = nil
        end
    end
end

function EnemyAISystem(deltaTime)
    for entityId, ai in pairs(EnemyAIComponent) do
        if not Manager.Instance():IsValidEntity(entityId) then
            EnemyAIComponent[entityId] = nil
        else
        local currentX = Transform.GetPositionX(entityId)
        if ai.state == "Patrol" then
            if currentX >= ai.targetX then
                Rigidbody.SetVelocity(entityId, -50.0, 0.0)
                ai.targetX = 200.0
            elseif currentX <= ai.targetX and Rigidbody.GetVelocityX(entityId) < 0 then
                Rigidbody.SetVelocity(entityId, 50.0, 0.0)
                ai.targetX = 600.0
            end
        end
        end
    end
end
```

## 엔진 설정 커스터마이징 (`config.lua`)

MIR Engine의 ECS 저장소와 명령 버퍼는 고정 용량으로 동작하므로 관련 설정은 **컴파일 타임**에 결정됩니다. 용량 초과 시 숨겨진 힙 확장을 하지 않으며 명령은 실패로 보고됩니다. 윈도우 창 제목이나 크기 등은 엔진 재빌드 없이 **런타임**에 변경할 수 있습니다.

게임 프로젝트(예: `canvasguard-lua`)의 루트 디렉토리에 **`config.lua`** 파일을 생성하여 아래와 같이 설정할 수 있습니다.

### 1. 컴파일 타임 설정 (엔진 재빌드 필요)
Zero-allocation 자료구조 크기에 영향을 주는 설정들입니다. 변경 시 엔진이 다시 빌드되어 적용됩니다.
* **`MAX_ENTITY`**: 최대 엔티티 및 컴포넌트 인덱스 범위 (기본값 `4096`)
* **`MAX_COMPONENT`**: 최대 컴포넌트 종류 개수 (기본값 `128`)
* **`MAX_SYSTEM`**: 최대 시스템 개수 (기본값 `64`)
* **`MAX_SYSTEM_DEPENDENCIES`**: 시스템 실행 그래프의 최대 의존성 간선 수 (기본값 `MAX_SYSTEM * 4`)
* **`COMMAND_BUFFER_BYTES`**: 한 프레임에 예약 가능한 지연 명령 payload 크기 (기본값 `1048576`)

### 2. 런타임 설정 (엔진 재빌드 불필요)
이미 빌드된 엔진 바이너리(`mirengine`)가 실행 시점에 동적으로 로드하는 설정들입니다.
* **`WINDOW_TITLE`**: 게임 창 제목 (기본값 `MIR Engine`)
* **`WINDOW_MODE`**: 창 모드 (기본값 `Windowed`)
  * 설정 가능 값: `Windowed`, `Fullscreen`, `Borderless`, `Desktop`
* **`WINDOW_RESOLUTION`**: 해상도 프리셋 (기본값 `HD`)
  * 설정 가능 값: `HD` (1280x720), `FHD` (1920x1080), `QHD` (2560x1440), `UHD` (3840x2160), `Custom`
* **`WINDOW_WIDTH`**: `Custom` 해상도 선택 시 창 너비 (기본값 `1280`)
* **`WINDOW_HEIGHT`**: `Custom` 해상도 선택 시 창 높이 (기본값 `720`)

---

### 설정 예시 (`config.lua`)

```lua
-- ==========================================
-- 1. Compile-time Configs (Triggers rebuild)
-- ==========================================
MAX_ENTITY = 2000
MAX_COMPONENT = 256
MAX_SYSTEM = 128
MAX_SYSTEM_DEPENDENCIES = 512
COMMAND_BUFFER_BYTES = 2097152

-- ==========================================
-- 2. Runtime Configs (Dynamic loading)
-- ==========================================
WINDOW_TITLE = "My Awesome Lua Game"
WINDOW_MODE = WindowMode.Windowed
WINDOW_RESOLUTION = WindowResolution.Custom
WINDOW_WIDTH = 1600
WINDOW_HEIGHT = 900
```

---

---

## 의존성 및 패키지
엔진은 빌드 시 Xmake 패키지 관리자를 통해 다음 의존성을 자동으로 내려받고 링크합니다:
- **`zet`**: Zero-allocated Execution Toolkit (컨테이너 라이브러리)
  - 루트 `xmake.lua`는 GitHub `main`을 추적합니다. 재현 가능한 릴리스가 필요하면 검증한 커밋 SHA 또는 태그로 고정하십시오.
- **`lua 5.4.x` / `sol2`**: 스크립팅 바인딩
- **`libsdl3` / `libsdl3_image` / `libsdl3_ttf` / `libsdl3_mixer`**: 창 제어, 이미지, 폰트, 오디오 시스템

---

## 빌드 및 실행 방법

### 사전 요구 사항
- [Xmake](https://xmake.io/) 설치
- C++20 컴파일러 (gcc 11+, clang 12+, MSVC 2019+)
- (Linux 한정) 오디오 및 창 생성을 위한 시스템 라이브러리 개발용 헤더:
  ```bash
  sudo apt-get update
  sudo apt-get install -y libasound2-dev libpulse-dev libx11-dev libxext-dev libgl1-mesa-dev libpng-dev libjpeg-dev libxrender-dev libxi-dev libxfixes-dev libxrandr-dev libxcursor-dev libxinerama-dev libwayland-dev libxss-dev xutils-dev
  ```

### 1. 프로젝트 설정 및 컴파일
```bash
# 빌드 구성 설정 (릴리즈 모드)
xmake f -m release

# 전체 타겟 빌드
xmake
```

### 2. 단위 테스트 빌드 및 실행
엔진 수학(Math) 모듈 및 코어 ECS 시스템 검증을 위한 테스트 바이너리를 실행합니다.
```bash
# 테스트 타겟 빌드
xmake build mirengine-tests

# 테스트 실행
xmake run mirengine-tests
```
