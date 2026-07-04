# MIR Engine - 한국어 문서

C++20 및 SDL3를 기반으로 작성된 고성능, 초경량 2D 게임 엔진입니다. 메모리 할당 최소화를 위해 개발된 `zet` 컨테이너 패키지와 엔티티-컴포넌트(ECS) 패턴을 기반으로 설계되었습니다.

---

## 주요 특징
- **C++20 기반 설계:** 최신 C++ 기능 및 SIMD 최적화 수학 모듈 활용
- **SDL3 런타임 연동:** 크로스 플랫폼 창 생성, 입력 폴링, 2D 그래픽 렌더링, `SDL3_mixer` 오디오 탑재
- **ECS (Entity-Component-System) 패턴:** 캐시 효율성을 극대화하는 `SparseSet` 기반 컴포넌트 저장소 및 Command Buffer 기반 지연 처리 구조
- **Lua 스크립팅 통합:** `sol2` 라이브러리를 활용하여 런타임에 게임 로직 제어 가능
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
│   ├── sdl/                # SDL3를 활용한 추상화 선언의 실제 구현체
│   ├── system/             # 시스템 로직 및 Lua 스크립트 시스템
│   ├── test/               # 단위 테스트 코드 (TestRunner.cpp)
│   └── xmake.lua           # 엔진 라이브러리 및 테스트 타겟 빌드 설정
├── main.cpp                # 프로그램 진입점 및 메인 게임 루프
└── xmake.lua               # 프로젝트 루트 빌드 구성 파일
```

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
  sudo apt-get install -y libasound2-dev libpulse-dev libx11-dev libxext-dev libgl1-mesa-dev libpng-dev libjpeg-dev
  ```

### 1. 프로젝트 설정 및 컴파일
```bash
# 빌드 구성 설정 (릴리즈 모드)
xmake f -m release

# 전체 타겟 빌드
xmake
```

### 2. 엔진 실행
```bash
xmake run mirengine
```

### 3. 단위 테스트 빌드 및 실행
엔진 수학(Math) 모듈 및 코어 ECS 시스템 검증을 위한 테스트 바이너리를 실행합니다.
```bash
# 테스트 타겟 빌드
xmake build mirengine-tests

# 테스트 실행
xmake run mirengine-tests
```
