# MIR (Modern Interface & Runtime) Engine - 한국어 문서

C++20 및 SDL3를 기반으로 작성된 고성능, 초경량 2D 게임 엔진입니다. 메모리 할당 최소화를 위해 개발된 `zet` 컨테이너 패키지와 엔티티-컴포넌트(ECS) 패턴을 기반으로 설계되었습니다.

---

## 주요 특징
- **C++20 기반 설계:** 최신 C++ 기능 및 SIMD 최적화 수학 모듈 활용
- **SDL3 런타임 연동:** 크로스 플랫폼 창 생성, 입력 폴링, 2D 그래픽 렌더링, `SDL3_mixer` 오디오 탑재
- **ECS (Entity-Component-System) 패턴:** 캐시 효율성을 극대화하는 `SparseSet` 기반 컴포넌트 저장소 및 Command Buffer 기반 지연 처리 구조
- **Lua 스크립팅 통합:** `sol2` 라이브러리를 활용하여 런타임에 게임의 컴포넌트 제어, 사운드, 입력, 물리, 충돌을 제어 가능
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
엔진은 `script/main.lua` 파일을 진입점으로 실행하며, Lua 스크립트 상에서 다음 API들을 직접 다룰 수 있습니다:

### 1. ECS 및 생명주기 관리
- `Manager.Instance()`: 매니저 객체 반환
- `Manager.Instance():AddEntity()` -> `Id`: 신규 엔티티 ID 생성
- `Manager.Instance():DeleteEntity(id)`: 엔티티 삭제
- `Manager.Instance():IsValidEntity(id)` -> `bool`: 엔티티가 살아있는지 검사

### 2. 컴포넌트 API
- **Transform** (위치 및 크기):
  - `Transform.setPosition(id, x, y)` / `Transform.getPositionX(id)` / `Transform.getPositionY(id)`
  - `Transform.setRotation(id, r)` / `Transform.getRotation(id)`
  - `Transform.setScale(id, s)` / `Transform.getScale(id)`
  - `Transform.isValid(id)` / `Transform.remove(id)`
- **Sprite** (렌더링 리소스):
  - `Sprite.setTexture(id, "path")` / `Sprite.getTexture(id)`
  - `Sprite.setSourceSize(id, w, h)` / `Sprite.setDestinationSize(id, w, h)`
  - `Sprite.setAnchor(id, x, y)` / `Sprite.setTint(id, r, g, b)`
  - `Sprite.setZindex(id, z)` / `Sprite.getZindex(id)`
  - `Sprite.setAlpha(id, a)` / `Sprite.getAlpha(id)`
  - `Sprite.isValid(id)` / `Sprite.remove(id)`
- **Rigidbody** (물리 설정):
  - `Rigidbody.setVelocity(id, vx, vy)` / `Rigidbody.getVelocityX(id)` / `Rigidbody.getVelocityY(id)`
  - `Rigidbody.setGravity(id, g)` / `Rigidbody.getGravity(id)`
  - `Rigidbody.setOnGround(id, bool)` / `Rigidbody.isOnGround(id)`
  - `Rigidbody.isValid(id)` / `Rigidbody.remove(id)`
- **Collider** (충돌체 설정):
  - `Collider.setBound(id, w, h)` / `Collider.getBoundX(id)` / `Collider.getBoundY(id)`
  - `Collider.setOffset(id, x, y)` / `Collider.getOffsetX(id)` / `Collider.getOffsetY(id)`
  - `Collider.setShouldTrigger(id, bool)` / `Collider.getShouldTrigger(id)`
  - `Collider.isValid(id)` / `Collider.remove(id)`
- **Tag** (이벤트 트리거 태그):
  - `Tag.set(id, "TagName")` / `Tag.get(id)` / `Tag.isValid(id)` / `Tag.remove(id)`

### 3. 디바이스 입력 및 사운드
- **Input & Key**:
  - `Input.isPressed(Key.W)` / `Input.isJustPressed(Key.Space)` / `Input.isJustReleased(Key.Enter)`
  - `Input.getMouseX()` / `Input.getMouseY()`
- **Sound**:
  - `Sound.load("path")` / `Sound.play("name", vol, pitch)` / `Sound.playBgm("name", vol, loop)` / `Sound.stopBgm()` / `Sound.stopAll()`

### 4. 서브시스템
- `Movement.update(id, deltaTime)`: 물리 이동 업데이트 적용
- `Collision.update(lhsId, rhsId)` -> `bool`: AABB 박스 충돌 판단 검사

---

## 의존성 및 패키지
엔진은 빌드 시 Xmake 패키지 관리자를 통해 다음 의존성을 자동으로 내려받고 링크합니다:
- **`zet`**: Zero-allocated Execution Toolkit (컨테이너 라이브러리)
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
