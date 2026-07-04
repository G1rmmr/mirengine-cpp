# MIR Engine - English Documentation

A high-performance, ultra-lightweight 2D game engine built on C++20 and SDL3. It is designed around the `zet` zero-allocated container library and an Entity-Component-System (ECS) pattern.

---

## 📌 Key Features
- **Modern C++20 Core:** Utilizes the latest C++ features and custom SIMD-optimized math modules.
- **SDL3 Runtime Integration:** Employs SDL3 for cross-platform window creation, input polling, 2D graphic rendering, and `SDL3_mixer` for audio.
- **ECS (Entity-Component-System) Architecture:** Employs cache-efficient `SparseSet` container for components and Command Buffer for deferred deletion and component assignment.
- **Lua Scripting Integration:** Integrates `sol2` library to control game logic dynamically at runtime.
- **CI/CD:** Automated builds and unit tests on Windows, Linux, and macOS using Xmake and GitHub Actions.

---

## 📂 Project Directory Structure
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
│   ├── sdl/                # SDL3 implementations for abstract interfaces
│   ├── system/             # System updates and Lua script binding system
│   ├── test/               # Unit tests (TestRunner.cpp)
│   └── xmake.lua           # Engine library and test target build definitions
├── main.cpp                # Main entry point and game loop
└── xmake.lua               # Project root build configuration file
```

---

## 🛠️ Dependencies & Packages
The engine automatically downloads and links the following dependencies via Xmake:
- **`zet`**: Zero-allocated Execution Toolkit (Container Library)
- **`lua 5.4.x` / `sol2`**: Scripting bindings
- **`libsdl3` / `libsdl3_image` / `libsdl3_ttf` / `libsdl3_mixer`**: Windowing, image, font, and audio rendering systems

---

## 🚀 Build and Run

### Prerequisites
- [Xmake](https://xmake.io/) installed
- C++20 compatible compiler (gcc 11+, clang 12+, MSVC 2019+)
- (Linux Only) System headers for audio and display rendering:
  ```bash
  sudo apt-get update
  sudo apt-get install -y libasound2-dev libpulse-dev libx11-dev libxext-dev libgl1-mesa-dev libpng-dev libjpeg-dev
  ```

### 1. Configure and Compile
```bash
# Configure the build mode (release mode)
xmake f -m release

# Build all default targets
xmake
```

### 2. Run the Engine
```bash
xmake run mirengine
```

### 3. Build and Run Unit Tests
Builds and runs unit tests verifying the custom math module and the ECS manager.
```bash
# Build the test target
xmake build mirengine-tests

# Run tests
xmake run mirengine-tests
```
