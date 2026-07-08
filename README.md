# CEngine

CEngine is a modular, lightweight game engine written in modern C++ as a learning and experimentation project.
It provides a basic architecture for managing game states, scenes, and objects, using patterns like State and Component to separate concerns and encourage scalability.

This project is not intended for production use, but rather serves as a case study in building a game engine from scratch — focusing on scene management, object lifecycle, and rendering loops, with flexibility to evolve into an ECS architecture.

Ideal for exploring concepts such as:

* Scene and state-based design
* Game object hierarchies
* Separation of engine and game logic
* Component-based architecture foundations

Note: CEngine is part of a larger personal project and is under active experimentation.

## Building

CEngine uses CMake (>= 3.23) and requires a **C++23** compiler. The standard is
propagated automatically via `target_compile_features(... PUBLIC cxx_std_23)`, so
anything linking `cengine::core` / `cengine::routing` does not need to set the
standard by hand.

### Presets

Portable presets (no machine-specific paths) live in `CMakePresets.json`:

| Preset | Purpose |
|--------|---------|
| `debug` | Debug build (Ninja) |
| `release` | Release build (Ninja) |
| `asan` | Debug + AddressSanitizer/UBSan (best on Linux/CI) |

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

Machine-specific presets (custom compiler paths, e.g. MSYS2/MinGW) belong in an
untracked `CMakeUserPresets.json` (git-ignored).

> **MSYS2/MinGW note:** the `debug`/`release`/`asan` presets use the Ninja
> generator. MSYS2's bundled `ninja` runs commands through `/bin/sh`, which
> mangles native (`C:\...`) compiler paths. On such setups, use a machine preset
> with the `MinGW Makefiles` generator in your `CMakeUserPresets.json` instead
> (or install a native Ninja).

### Modules (opt-in)

`cengine::core` (the game loop + ports) is always built. Optional modules are
gated by CMake options:

| Option | Default | Effect |
|--------|---------|--------|
| `CENGINE_BUILD_ROUTING` | `ON` | Build `cengine::routing` (scene/state routing) |
| `CENGINE_BUILD_TESTS` | `ON` | Build the test suite |

A consumer links only what it needs:

```cmake
target_link_libraries(my_game PRIVATE
    cengine::core        # always
    cengine::routing     # optional
)
```

## Usage

CEngine is consumed via CMake's `FetchContent` (this is how the sibling
[8Puzzle](https://github.com/mrmarmitt/8puzzle) project uses it):

```cmake
include(FetchContent)

FetchContent_Declare(
  cengine
  GIT_REPOSITORY https://github.com/mrmarmitt/cengine.git
  GIT_TAG        main   # pin a tagged release once available
)

# Consumers don't need cengine's own test suite. Turn it off before making the
# dependency available — otherwise cengine fetches GoogleTest at configure time
# (CENGINE_BUILD_TESTS defaults to ON), which is wasteful and fails offline.
set(CENGINE_BUILD_TESTS OFF)
FetchContent_MakeAvailable(cengine)

add_executable(my_game src/main.cpp)
target_link_libraries(my_game PRIVATE
    cengine::core        # game loop + ports (always)
    cengine::routing     # scene/state routing (optional)
)
```

> The `cengine::core` / `cengine::routing` targets require the modular layout on
> `main`. The `0.0.1` tag predates it (single `cengine_lib` target).

### Minimal assembly

The engine is **graphics-agnostic**: you implement the ports (window + scenes)
and the engine drives the loop. A minimal game registers its scene factories in
a repository, hands its ownership to the router (which owns the state machine),
and gives everything to the `EngineManager`:

```cpp
#include <memory>
#include <cengine/core/EngineManager.hpp>
#include <cengine/routing/GameManager.hpp>
#include <cengine/routing/RouterInMemory.hpp>
#include <cengine/routing/SceneRepository.hpp>

using namespace cengine;

int main() {
    // 1. Repository = scene provider: register a factory per state code
    //    (lazy instantiation). You provide MenuScene / GameplayScene : IScene.
    auto repository = std::make_unique<routing::SceneRepository>();
    repository->registerFactory("main_menu", [] { return std::make_unique<MenuScene>(); });
    repository->registerFactory("gameplay",  [] { return std::make_unique<GameplayScene>(); });

    // 2. Router = state machine. It takes OWNERSHIP of the repository and the
    //    initial state (you provide MyState : IState).
    auto router = std::make_shared<routing::RouterInMemory>(
        std::move(repository), std::make_unique<MyState>("main_menu"));
    auto gameManager = std::make_unique<routing::GameManager>(router);

    // 3. Run the loop (you provide MyWindowManager : cengine::core::IWindowManager).
    core::EngineManager engine{
        std::make_unique<MyWindowManager>(),
        std::move(gameManager)
    };
    engine.start(); // blocks until a scene routes to the exit state
    return 0;
}
```

To navigate, a scene calls `router->requestState(std::make_unique<MyState>("gameplay"))`;
the switch is committed at the end of the frame (`onExit`). Requesting the
*current* state's code is a deliberate reload (the scene is destroyed, recreated
and re-entered). Routing to the `cengine::routing::kExitStateCode` state stops
the loop.

### The frame and time

Each frame runs `input()` → `update(dt)` → `draw()` on the active scene. The
loop uses a **fixed timestep** ("fix your timestep"): frame time is measured
with a monotonic clock and consumed in constant `dt` steps — `update` runs
**0..N times per frame, always with the same `dt`** (default 1/60 s,
configurable in the `EngineManager` constructor). Put simulation (animations,
timers, physics) in `update(dt)`; never measure time inside a scene.

The public headers carry Doxygen comments describing each contract — start from
[`IScene`](core/include/cengine/core/IScene.hpp) and
[`IGameManager`](core/include/cengine/core/IGameManager.hpp).

## Working context (`.ai/`)

The [`.ai/`](.ai/) folder is the starting point for understanding **what is being
worked on and where we are** in the engine's improvement effort. Read it before
making changes:

* [`.ai/task/README.md`](.ai/task/README.md) — the improvement plan: one file per
  task, ordered by recommended development sequence (from the highest-impact
  architectural work down to cleanup), each with problem, steps, and acceptance
  criteria.
* [`.ai/decisions/`](.ai/decisions/) — Architecture Decision Records (ADRs) that
  guide the plan. Start with
  [ADR 0001 — minimal core (loop) + optional modules](.ai/decisions/0001-modular-core-vs-modules.md):
  the core is just the game loop + ports; routing and physics are opt-in modules
  in the same repo.
* [`.ai/build-and-test.md`](.ai/build-and-test.md) — verified build & test
  environment (toolchain, reproducible commands, current status). The test suite
  is the safety net: rebuild and run `ctest` after every task.