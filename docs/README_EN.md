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
The engine boots up with `script/main.lua` as its entrypoint, where you can call the following APIs:

### 1. ECS & Lifecycle
- `Manager.Instance()`: Returns the ECS manager instance
- `Manager.Instance():AddEntity()` -> `Id`: Generates a new Entity ID
- `Manager.Instance():DeleteEntity(id)`: Deletes the entity
- `Manager.Instance():IsValidEntity(id)` -> `bool`: Returns true if the entity is valid

### 2. Component APIs
- **Transform** (Position & scale):
  - `Transform.setPosition(id, x, y)` / `Transform.getPositionX(id)` / `Transform.getPositionY(id)`
  - `Transform.setRotation(id, r)` / `Transform.getRotation(id)`
  - `Transform.setScale(id, s)` / `Transform.getScale(id)`
  - `Transform.isValid(id)` / `Transform.remove(id)`
- **Sprite** (Graphic resources):
  - `Sprite.setTexture(id, "path")` / `Sprite.getTexture(id)`
  - `Sprite.setSourceSize(id, w, h)` / `Sprite.setDestinationSize(id, w, h)`
  - `Sprite.setAnchor(id, x, y)` / `Sprite.setTint(id, r, g, b)`
  - `Sprite.setZindex(id, z)` / `Sprite.getZindex(id)`
  - `Sprite.setAlpha(id, a)` / `Sprite.getAlpha(id)`
  - `Sprite.isValid(id)` / `Sprite.remove(id)`
- **Rigidbody** (Physics dynamics):
  - `Rigidbody.setVelocity(id, vx, vy)` / `Rigidbody.getVelocityX(id)` / `Rigidbody.getVelocityY(id)`
  - `Rigidbody.setGravity(id, g)` / `Rigidbody.getGravity(id)`
  - `Rigidbody.setOnGround(id, bool)` / `Rigidbody.isOnGround(id)`
  - `Rigidbody.isValid(id)` / `Rigidbody.remove(id)`
- **Collider** (Collision bounds):
  - `Collider.setBound(id, w, h)` / `Collider.getBoundX(id)` / `Collider.getBoundY(id)`
  - `Collider.setOffset(id, x, y)` / `Collider.getOffsetX(id)` / `Collider.getOffsetY(id)`
  - `Collider.setShouldTrigger(id, bool)` / `Collider.getShouldTrigger(id)`
  - `Collider.isValid(id)` / `Collider.remove(id)`
- **Tag** (Event tagging):
  - `Tag.set(id, "TagName")` / `Tag.get(id)` / `Tag.isValid(id)` / `Tag.remove(id)`

### 3. Device Inputs & Audio
- **Input & Key**:
  - `Input.isPressed(Key.W)` / `Input.isJustPressed(Key.Space)` / `Input.isJustReleased(Key.Enter)`
  - `Input.getMouseX()` / `Input.getMouseY()`
- **Sound**:
  - `Sound.load("path")` / `Sound.play("name", vol, pitch)` / `Sound.playBgm("name", vol, loop)` / `Sound.stopBgm()` / `Sound.stopAll()`

### 4. Subsystems
- `Movement.update(id, deltaTime)`: Runs the movement update logic
- `Collision.update(lhsId, rhsId)` -> `bool`: Runs AABB collision intersection check

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
