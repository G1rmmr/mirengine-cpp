# MIR (Modern Interface & Runtime) Engine - English Documentation

A C++20 and SDL3 2D game engine built around ZET containers and a generation-safe Entity-Component-System (ECS). Core ZET-backed storage performs no hidden heap allocation; Lua, SDL resources, file I/O, and some standard-library value types may allocate at runtime.

---

## Key Features
- **Modern C++20 Core:** Utilizes the latest C++ features and custom SIMD-optimized math modules.
- **SDL3 Runtime Integration:** Employs SDL3 for cross-platform window creation, input polling, 2D graphic rendering, and `SDL3_mixer` for audio.
- **ECS (Entity-Component-System) Architecture:** Uses `SparseSet` storage with generation validation and a fixed-capacity Command Buffer for deferred deletion and component assignment.
- **Lua Scripting Integration:** Integrates `sol2` library to control game components, inputs, audio, and systems dynamically at runtime.
- **Shared Development Surface:** `MIR.hpp` gives C++ and Lua access to the same generation-safe ECS, asset, and system capabilities without exposing container handles or SDL pointers to Lua.
- **Current ZET Integration:** Xmake fetches the `main` branch of `zetcontainer-cpp` from GitHub at build time, so the project builds against the current container API.
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
- `Manager.Instance():ForEachEntity(function(id) ... end)` -> `bool`: Iterates valid entities without exposing storage handles

### 2. Component APIs
- **Transform** (Position & scale):
  - `Transform.SetPosition(id, x, y)` / `Transform.GetPositionX(id)` / `Transform.GetPositionY(id)`
  - `Transform.SetRotation(id, r)` / `Transform.GetRotation(id)`
  - `Transform.SetScale(id, s)` / `Transform.GetScale(id)`
  - `Transform.GetWorldPositionX(id)` / `Transform.GetWorldPositionY(id)` / `Transform.GetWorldRotation(id)` / `Transform.GetWorldScale(id)`
  - `Transform.IsValid(id)` / `Transform.Remove(id)`
- **Sprite** (Graphic resources):
  - `Sprite.SetTexture(id, "path")` / `Sprite.GetTexture(id)`
  - `Sprite.SetSourceSize(id, w, h)` / `Sprite.SetDestinationSize(id, w, h)`
  - `Sprite.SetSourceRect(id, x, y, w, h)` (selects a sprite-sheet frame)
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
- **Tag** (Persistent entity classification):
  - `Tag.Set(id, "TagName")` / `Tag.Get(id)` / `Tag.IsValid(id)` / `Tag.Remove(id)`
- **Hierarchy** (ECS parent-child relations):
  - `Hierarchy.SetParent(child, parent)` / `Hierarchy.ClearParent(child)`
  - `Hierarchy.GetParent(id)` / `Hierarchy.GetFirstChild(id)` / `Hierarchy.GetNextSibling(id)`
  - Relation changes commit through the Command Buffer; deleting a parent detaches its children to the world root.

### 3. Device Inputs & Audio
- **Input & Key**:
  - `Input.IsPressed(Key.W)` / `Input.IsJustPressed(Key.Space)` / `Input.IsJustReleased(Key.Enter)`
  - `Key.F1` through `Key.F12`, plus `Input.IsMousePressed(MouseButton.Left)`, `Input.IsMouseJustPressed(...)`, and `Input.IsMouseJustReleased(...)`
  - `Input.GetMouseX()` / `Input.GetMouseY()`
- **Sound**:
  - `Sound.Load("path")` / `Sound.Play("name", vol, pitch)` / `Sound.PlayBgm("name", vol, loop)` / `Sound.StopBgm()` / `Sound.StopAll()`

### 4. GPU Shaders (SDL_GPU API)
Allows managing custom shaders and pipelines using the integrated SDL3.0 GPU subsystem.
* **Device Management**:
  * `GPU.CreateDevice(formats, debugMode)` -> `GPUDevice`
  * `device:Destroy()` / `device:ClaimWindow()` / `device:ReleaseWindow()` / `device:GetSwapchainFormat()`
  * Raw SDL pointer addresses are deliberately not exposed to Lua.
* **Constants & Formats**:
  * `GPUShaderStage.Vertex` / `GPUShaderStage.Fragment`
  * `GPU_SHADERFORMAT_SPIRV` / `GPU_SHADERFORMAT_DXIL` / `GPU_SHADERFORMAT_MSL`
* **Shader & GPUPipeline**:
  * `Shader.new()`: Instantiates a new shader object
    * `Shader:LoadFromFile(device, filepath, entrypoint, stage, numSamplers, numUniformBuffers)` -> `bool`
    * `Shader:Destroy()`
  * `GPUPipeline.new()`: Instantiates a new graphics pipeline
    * `GPUPipeline:Create(device, vs, fs, renderTargetFormat)` -> `bool`
    * `GPUPipeline:Destroy()`

### 5. Subsystems
- `Movement.Update(id, deltaTime)`: Runs the movement update logic
- `Collision.Update(lhsId, rhsId)` -> `bool`: Runs AABB collision intersection check
- `Animation.Register(name, { AnimationFrame.new(x, y, width, height), ... })` / `Animation.Play(id, name, speed, loop)` / `Animation.Stop(id)`
- `Resource.Register(name, path)` / `Resource.GetPath(name)` / `Resource.Unregister(name)`
- `Scene.Register(name, callback)` / `Scene.Load(name)`
- `Event.On(name, callback)` / `Event.Emit(id, name)`: dispatched automatically during the simulation phase
- `Timer.After(seconds, callback)` / `Timer.Every(seconds, callback)` / `Timer.Cancel(handle)`
- `System.Register(callback, SystemPhase.Simulation|PostCommit)` / `System.AddDependency(before, after)`: bounded Lua-only ordering graph
- `Profiler`, `Debug`, `Border`, `Label`, `Button`, `Camera` (position, follow, zoom, shake), and runtime-safe `Window` setters are also script-facing.

### 6. C++ Development API
Include `engine/MIR.hpp` to use the same public, generation-safe APIs as Lua. C++ systems are registered through `core::Manager::RegisterSystem`; Lua systems use a separate bounded dispatcher so neither side can forge the other's handles.

```cpp
#include "MIR.hpp"

auto& manager = mir::core::Manager::Instance();
const mir::Id player = manager.AddEntity();
if (!mir::transform::SetPosition(player, 100.f, 80.f)) {
    // The command buffer is full or the entity is invalid.
}
mir::event::On("spawn", [](mir::Id id) { /* ... */ });
mir::event::Emit(player, "spawn");
```

All structural setters return `bool`: `true` means the command was queued. The new value becomes observable after the next `Manager::UpdateSystem()` commit barrier.

### 7. Runtime Model and API Boundaries

- `Manager::RegisterSystem(fn, SystemPhase::Simulation|PostCommit)` registers C++ systems; `AddSystemDependency(before, after)` defines a stable order within one phase. A commit barrier separates phases, so cross-phase dependencies are rejected.
- `Event.Emit` and `Timer.After`/`Timer.Every` attach to the simulation phase automatically. Do not manually call `Event.Update` or `Timer.Update` every frame.
- `Resource` and `Scene` are fixed-capacity registries. `false` from `Register` or `Load` means an invalid name, exhausted capacity, or an unregistered item. Use `Resource.Clear`, `Scene.Clear`, and `Timer.Clear` at shutdown or an explicit restart boundary.
- `GPUDevice` is a move-only owner of an SDL GPU device. Use `Raw()` only for lower-level C++ SDL integration; Lua receives and passes an owning userdata rather than a pointer value.
- The loop in `main.cpp` owns `Window.Init`, event polling, rendering, presentation, and shutdown. Lua is deliberately limited to frame-safe settings such as title, size, mode, resolution, FPS, and closing the window.
- Video playback is not yet an engine feature. `runtime/sdl/asset/Video.hpp` is a legacy declaration without a decoder backend, and is not published to C++ or Lua until a decoder and distribution license (for example FFmpeg) are selected.

### 8. Game Code Example (main.lua)
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

### 9. Custom Component & System Example
```lua
-- Lua Custom Components
local HealthComponent = {}
local EnemyAIComponent = {}

function AddHealthComponent(entity, hp)
    HealthComponent[entity] = { hp = hp, maxHp = hp }
end

function AddEnemyAI(entity, targetX)
    EnemyAIComponent[entity] = { state = "Patrol", targetX = targetX }
end

-- Lua Custom Systems
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

## Engine Configuration Customization (`config.lua`)

MIR's ECS storage and command buffer use fixed capacities determined at **compile time**. They do not fall back to hidden heap growth when full. Window settings such as title and resolution remain configurable at **runtime**.

You can configure these limits by creating a **`config.lua`** file in the root directory of your game project (e.g., `canvasguard-lua`):

### 1. Compile-time Configuration (Requires Engine Rebuild)
These settings determine the size of static, zero-allocation container sizes. Changing them will trigger an engine rebuild.
* **`MAX_ENTITY`**: Maximum number of entities (Default `4096`)
* **`MAX_COMPONENT`**: Maximum number of component types (Default `128`)
* **`MAX_SYSTEM`**: Maximum number of subsystems (Default `64`)
* **`MAX_SYSTEM_DEPENDENCIES`**: Maximum dependency edges in the system execution graph (Default `MAX_SYSTEM * 4`)
* **`COMMAND_BUFFER_BYTES`**: Maximum deferred-command payload bytes per frame (Default `1048576`)

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

## Dependencies & Packages
The engine automatically downloads and links the following dependencies via Xmake:
- **`zet`**: Zero-allocated Execution Toolkit (Container Library)
  - The root `xmake.lua` follows GitHub `main`. Pin a reviewed commit SHA or tag before creating a reproducible release.
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
