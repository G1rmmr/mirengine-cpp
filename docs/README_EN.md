# MIR (Modern Interface & Runtime) Engine - English Documentation

A high-performance, ultra-lightweight 2D game engine built on C++20 and SDL3. It is designed around the `zet` zero-allocated container library and an Entity-Component-System (ECS) pattern.

---

## Key Features
- **Modern C++20 Core:** Utilizes the latest C++ features and custom SIMD-optimized math modules.
- **SDL3 Runtime Integration:** Employs SDL3 for cross-platform window creation, input polling, 2D graphic rendering, and `SDL3_mixer` for audio.
- **ECS (Entity-Component-System) Architecture:** Employs cache-efficient `SparseSet` container for components and Command Buffer for deferred deletion and component assignment.
- **Lua Scripting Integration:** Integrates `sol2` library to control game components, inputs, audio, and systems dynamically at runtime.
- **CI/CD:** Automated builds and unit tests on Windows, Linux, and macOS using Xmake and GitHub Actions.

---

## Project Directory Structure
```text
mirengine-cpp/
├── .github/workflows/      # GitHub Actions CI configurations
├── build/                  # Build output directory
├── engine/                 # Engine core source code
│   ├── asset/              # Resource (Texture, Sound, Font, Animation) managers
│   ├── component/          # ECS Component definitions (Transform, Sprite, etc.)
│   ├── core/               # ECS Engine Core manager and entity handling
│   ├── device/             # Windowing, Input polling, and Key mapping
│   ├── math/               # SIMD-accelerated math, matrices, and vectors
│   ├── script/             # Lua scripting binding and runtime interpreter system
│   ├── sdl/                # SDL3 implementations for abstract interfaces
│   ├── system/             # Basic subsystems such as movement, collisions and events
│   ├── test/               # Unit tests (TestRunner.cpp)
│   └── xmake.lua           # Engine library and test target build definitions
└── xmake.lua               # Project root build configuration file
```

---

## Lua Scripting API Guide
The engine boots up with `main.lua` as its entrypoint, where you can call the following APIs:

### 1. ECS & Lifecycle
- `Manager.Instance()`: Returns the ECS manager instance
- `Manager.Instance():AddEntity()` -> `Id`: Generates a new Entity ID
- `Manager.Instance():DeleteEntity(id)`: Deletes the entity
- `Manager.Instance():IsValidEntity(id)` -> `bool`: Returns true if the entity is valid

### 2. Component APIs
- **Transform** (Position & scale):
  - `Transform.SetPosition(id, x, y)` / `Transform.GetPositionX(id)` / `Transform.GetPositionY(id)`
  - `Transform.SetRotation(id, r)` / `Transform.GetRotation(id)`
  - `Transform.SetScale(id, s)` / `Transform.GetScale(id)`
  - `Transform.IsValid(id)` / `Transform.Remove(id)`
- **Sprite** (Graphic resources):
  - `Sprite.SetTexture(id, "path")` / `Sprite.GetTexture(id)`
  - `Sprite.SetSourceSize(id, w, h)` / `Sprite.SetDestinationSize(id, w, h)`
  - `Sprite.SetAnchor(id, x, y)` / `Sprite.SetTint(id, r, g, b)`
  - `Sprite.SetZindex(id, z)` / `Sprite.GetZindex(id)`
  - `Sprite.SetAlpha(id, a)` / `Sprite.GetAlpha(id)`
  - `Sprite.IsValid(id)` / `Sprite.Remove(id)`
- **Rigidbody** (Physics dynamics):
  - `Rigidbody.SetVelocity(id, vx, vy)` / `Rigidbody.GetVelocityX(id)` / `Rigidbody.GetVelocityY(id)`
  - `Rigidbody.SetGravity(id, g)` / `Rigidbody.GetGravity(id)`
  - `Rigidbody.SetOnGround(id, bool)` / `Rigidbody.IsOnGround(id)`
  - `Rigidbody.IsValid(id)` / `Rigidbody.Remove(id)`
- **Collider** (Collision bounds):
  - `Collider.SetBound(id, w, h)` / `Collider.GetBoundX(id)` / `Collider.GetBoundY(id)`
  - `Collider.SetOffset(id, x, y)` / `Collider.GetOffsetX(id)` / `Collider.GetOffsetY(id)`
  - `Collider.SetShouldTrigger(id, bool)` / `Collider.GetShouldTrigger(id)`
  - `Collider.IsValid(id)` / `Collider.Remove(id)`
- **Tag** (Event tagging):
  - `Tag.Set(id, "TagName")` / `Tag.Get(id)` / `Tag.IsValid(id)` / `Tag.Remove(id)`

### 3. Device Inputs & Audio
- **Input & Key**:
  - `Input.IsPressed(Key.W)` / `Input.IsJustPressed(Key.Space)` / `Input.IsJustReleased(Key.Enter)`
  - `Input.GetMouseX()` / `Input.GetMouseY()`
- **Sound**:
  - `Sound.Load("path")` / `Sound.Play("name", vol, pitch)` / `Sound.PlayBgm("name", vol, loop)` / `Sound.StopBgm()` / `Sound.StopAll()`

### 4. Subsystems
- `Movement.Update(id, deltaTime)`: Runs the movement update logic
- `Collision.Update(lhsId, rhsId)` -> `bool`: Runs AABB collision intersection check

### 5. Game Code Example (main.lua)
```lua
local player = nil
local ground = nil
local speed = 250.0
local jumpForce = -600.0

function Init()
    -- Load resources
    Texture.Load("assets/textures/player.png")
    Texture.Load("assets/textures/tiles.png")
    Sound.Load("assets/sounds/jump.wav")
    Sound.Load("assets/sounds/bgm.mp3")
    Sound.PlayBgm("assets/sounds/bgm.mp3", 50.0, true)

    local manager = Manager.Instance()

    -- Player entity Setup
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

    -- Ground setup
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

    -- Resolve collision
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

### 6. Custom Component & System Example
```lua
-- Lua Custom Components
local HealthComponent = {}
local EnemyAIComponent = {}

function AddHealthComponent(entity, hp)
    HealthComponent[entity.index] = { hp = hp, maxHp = hp }
end

function AddEnemyAI(entity, targetX)
    EnemyAIComponent[entity.index] = { state = "Patrol", targetX = targetX }
end

-- Lua Custom Systems
function HealthSystem(deltaTime)
    for index, health in pairs(HealthComponent) do
        local entityId = Id(index, 1)
        if health.hp <= 0 then
            Manager.Instance():DeleteEntity(entityId)
            HealthComponent[index] = nil
        end
    end
end

function EnemyAISystem(deltaTime)
    for index, ai in pairs(EnemyAIComponent) do
        local entityId = Id(index, 1)
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
```

## Engine Configuration Customization (`config.lua`)

Since MIR Engine utilizes a zero-allocation architecture, performance-critical settings such as entity pools and system array sizes are determined at **compile time**. However, window settings (like title and resolution) can be customized at **runtime** without rebuilding the engine.

You can configure these limits by creating a **`config.lua`** file in the root directory of your game project (e.g., `canvasguard-lua`):

### 1. Compile-time Configuration (Requires Engine Rebuild)
These settings determine the size of static, zero-allocation container sizes. Changing them will trigger an engine rebuild.
* **`MAX_ENTITY`**: Maximum number of entities (Default `4096`)
* **`MAX_COMPONENT`**: Maximum number of component types (Default `128`)
* **`MAX_SYSTEM`**: Maximum number of subsystems (Default `64`)

### 2. Runtime Configuration (No Rebuild Needed)
These settings are loaded dynamically at startup by the prebuilt engine binary (`mirengine`).
* **`WINDOW_TITLE`**: Game window title (Default `MIR Engine`)
* **`WINDOW_MODE`**: Window mode (Default `Windowed`)
  * Supported values: `Windowed`, `Fullscreen`, `Borderless`, `Desktop`
* **`WINDOW_RESOLUTION`**: Resolution preset (Default `HD`)
  * Supported values: `HD` (1280x720), `FHD` (1920x1080), `QHD` (2560x1440), `UHD` (3840x2160), `Custom`
* **`WINDOW_WIDTH`**: Custom screen width (Default `1280`, used when `WINDOW_RESOLUTION=Custom`)
* **`WINDOW_HEIGHT`**: Custom screen height (Default `720`, used when `WINDOW_RESOLUTION=Custom`)

---

### Configuration Example (`config.lua`)

```lua
-- ==========================================
-- 1. Compile-time Configs (Triggers rebuild)
-- ==========================================
MAX_ENTITY = 2000
MAX_COMPONENT = 256
MAX_SYSTEM = 128

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

## Dependencies & Packages
The engine automatically downloads and links the following dependencies via Xmake:
- **`zet`**: Zero-allocated Execution Toolkit (Container Library)
- **`lua 5.4.x` / `sol2`**: Scripting bindings
- **`libsdl3` / `libsdl3_image` / `libsdl3_ttf` / `libsdl3_mixer`**: Windowing, image, font, and audio rendering systems

---

## Build and Run

### Prerequisites
- [Xmake](https://xmake.io/) installed
- C++20 compatible compiler (gcc 11+, clang 12+, MSVC 2019+)
- (Linux Only) System headers for audio and display rendering:
  ```bash
  sudo apt-get update
  sudo apt-get install -y libasound2-dev libpulse-dev libx11-dev libxext-dev libgl1-mesa-dev libpng-dev libjpeg-dev libxrender-dev libxi-dev libxfixes-dev libxrandr-dev libxcursor-dev libxinerama-dev libwayland-dev libxss-dev xutils-dev
  ```

### 1. Configure and Compile
```bash
# Configure the build mode (release mode)
xmake f -m release

# Build all default targets
xmake
```

### 2. Build and Run Unit Tests
Builds and runs unit tests verifying the custom math module and the ECS manager.
```bash
# Build the test target
xmake build mirengine-tests

# Run tests
xmake run mirengine-tests
```
