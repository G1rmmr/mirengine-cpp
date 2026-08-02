# MIR (Modern Interface & Runtime) Engine

A C++20 and SDL3 2D game engine built around a generation-safe Entity-Component-System (ECS) architecture and ZET containers that perform no hidden heap allocation. Lua, SDL, file I/O, and standard-library value types may allocate at runtime.

---

### Documentation / 문서
Select your preferred language for detailed setup, features, structure, and execution guides:

- **[한국어 문서 (Korean Docs)](docs/README_KR.md)**
- **[English Documentation (English Docs)](docs/README_EN.md)**

---

### Quick Start / 빠른 시작
```bash
# Configure and build the project
xmake f -m release
xmake

# Run the game engine
xmake run mirengine

# Run unit tests
xmake build mirengine-tests
xmake run mirengine-tests
```
