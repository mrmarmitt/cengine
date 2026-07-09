# Changelog

All notable changes to CEngine are documented here.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.4.0] - 2026-07-09

Hosted loop mode (task 15 of the [improvement plan](.ai/task/README.md)):
frameworks with inversion of control (The-Forge's `IApp`, editors, browsers)
own the loop and cannot call the blocking `start()` — they now drive the
engine with `frame(dt)`, one call per frame, keeping every fixed-timestep
guarantee from 0.3.0. Design validated against a real host (phase 1 of the
8Puzzle The-Forge PoC).

**Non-breaking release** — purely additive; no consumer changes required.

### Added

- **`EngineManager::frame(Seconds frameTime)`**: runs ONE complete frame
  (`onEnter` → `input` → `update(fixedDt)` 0..N times → `render` → `onExit`)
  consuming the host-measured `frameTime` in the internal fixed-timestep
  accumulator (now a member — the remainder persists across calls). Returns
  `false` when the game routed to the exit state; shutdown is the host's
  decision, and the host calls `cleanup()` on its teardown.
- **Hosted-mode tests** (6): phase order, short frame (0 updates, render still
  runs), accumulator persisting across `frame()` calls, `maxFrameTime` clamp,
  exit condition, `cleanup()` without a window manager.
- **README section** on hosted mode with the per-frame callback recipe.

### Changed

- **`run()` is now a consumer of `frame()`** — both modes share the exact same
  frame logic; the existing call-log suite passes unchanged.
- **`windowManager == nullptr` is supported** (hosted mode has no engine
  window: the host owns window, message pump and pacing). `run()` gained the
  missing null guard; `frame()` never touches the window by design.
- `maxFrameTime` clamping composes harmlessly with a host-side `dt` clamp
  (documented); the injectable clock is not consulted in hosted mode.

## [0.3.0] - 2026-07-08

Time in the loop (task 14 of the [improvement plan](.ai/task/README.md)): the
game loop gains a time concept — `update(dt)` with a **fixed timestep** (the
"fix your timestep" pattern). This closes the last structural gap in the core
flagged by ADR 0001 and provides the contract a future `cengine::physics`
module requires.

> **Breaking release.** `IScene` and `IGameManager` gain a pure virtual
> `update(Seconds dt)` — every consumer must implement it (an empty body is
> fine). See _Migrating from 0.2.0_ below.

### Added

- **`cengine/core/Time.hpp`** with
  `using Seconds = std::chrono::duration<double>;` — `std::chrono` end to end
  in the public API (no raw `double` with ambiguous units).
- **`IScene::update(Seconds dt)` / `IGameManager::update(Seconds dt)`**: the
  simulation phase. Fixed-timestep contract: called **0..N times per frame,
  always with the same `dt`** — scenes must not assume one call per frame nor
  measure time themselves.
- **Fixed-timestep loop** in `EngineManager::run()`: frame time is measured
  with the monotonic `steady_clock` and consumed by an accumulator in constant
  `fixedDt` steps; the remainder carries over to the next frame. Frame time is
  clamped to `maxFrameTime` (anti death-spiral).
- **Configurable timing** in the `EngineManager` constructor: `fixedDt`
  (default 1/60 s) and `maxFrameTime` (default 250 ms); non-positive values
  throw `std::invalid_argument` at construction.
- **Injectable time source** (`clockNow`, default `steady_clock::now`) — loop
  tests advance a fake clock manually; nothing sleeps or depends on real time.

### Changed

- **Frame order** is now `window.update → onEnter → input → update(dt)* →
  render → onExit` (README section _The frame and time_ documents the
  contract).

### Migrating from 0.2.0

- Add `void update(cengine::core::Seconds dt) override {}` to every `IScene`
  implementation (and to direct `IGameManager` implementations, if any) —
  mechanical change; leave the body empty if the scene has no simulation.
- Move any time-dependent logic (animations, timers, stopwatches) into
  `update(dt)` instead of measuring time inside the scene.
- Optionally tune the loop:
  `EngineManager{window, game, Seconds{1.0 / 120.0}}` for a 120 Hz simulation.

## [0.2.0] - 2026-07-08

API design cycle (tasks 12–13 of the [improvement plan](.ai/task/README.md)):
leaner ports and single-owner responsibilities in the routing module.

> **Breaking release.** The `IScene`, `IState` and `ISceneRepository` contracts
> changed, and so did the `RouterInMemory` wiring. See _Migrating from 0.1.0_
> below.

### Changed

- **`IScene` slimmed to 4 methods** (`onEnter` / `input` / `draw` / `onExit`):
  the activation bookkeeping pair `onEnterExecuted()` / `isOnEnterExecuted()`
  was removed from the port. The "runs exactly once per activation" guarantee
  is now enforced by `cengine::routing::GameManager` (tracked by state code,
  reset when a navigation is committed) — scenes no longer carry a flag.
- **`RouterInMemory` owns the state machine**: the current/next state pair
  moved from the repository into the router, and `hasPendingStateChange()` now
  means "a next state is scheduled" (null check) instead of a code comparison.
- **`RouterInMemory` takes exclusive ownership of the repository**: the
  constructor now receives `unique_ptr<ISceneRepository>` plus the initial
  state. No external reference can unload scenes behind the navigation cycle
  (closes the active-scene-eviction hole found in review).
- **Requesting the current state's code is now a valid transition** — a
  deliberate reload: the scene is unloaded, recreated and re-entered.
  Previously this was indistinguishable from "no pending change".
- **`ISceneRepository` is a pure scene provider**: `registerFactory` /
  `getScene` / `unloadScene` / `unloadAll`. The state accessors
  (`getCurrentStateGame`, `getNextStateGame`, `persisteCurrentState`,
  `persistNextState`, `hasPendingStateChange`) were removed with the
  responsibility.

### Removed

- **`IState::clone()`** — states are moved (`unique_ptr`) into the router; the
  Prototype pattern is no longer required from consumers.

### Added

- **Integration regression tests**: navigating A→B→A re-activates the returning
  scene, and requesting the current state reloads + re-activates it (both meant
  to run under the ASan CI job).

### Migrating from 0.1.0

- Remove `onEnterExecuted()` / `isOnEnterExecuted()` (and the backing flag)
  from your `IScene` implementations.
- Remove `clone()` from your `IState` implementations.
- Update the assembly: register factories on the repository, then **move** it
  into the router along with the initial state:
  ```cpp
  auto repository = std::make_unique<routing::SceneRepository>();
  repository->registerFactory("main_menu", [] { return std::make_unique<MenuScene>(); });

  auto router = std::make_shared<routing::RouterInMemory>(
      std::move(repository), std::make_unique<MyState>("main_menu"));
  auto gameManager = std::make_unique<routing::GameManager>(router);
  ```
- If your game kept a reference to the repository for domain navigation (e.g.,
  a facade), wrap the `IRouter` instead — the repository is engine-internal
  after the move.

## [0.1.0] - 2026-07-06

First release after the architecture overhaul guided by the improvement plan in
[`.ai/`](.ai/) (see [ADR 0001](.ai/decisions/0001-modular-core-vs-modules.md)).
The engine is now a **graphics-agnostic game loop + ports**, with scene/state
routing as an **opt-in module**.

> **Breaking release.** The library layout, target names, namespaces, include
> paths and parts of the public API changed. Consumers of `0.0.1` must update —
> see _Migrating from 0.0.1_ below.

### Added

- **Modular multi-target layout**: `cengine::core` (game loop + ports) and the
  optional `cengine::routing` (scene/state routing) module. Consumers link only
  what they need.
- **CMake options** to gate the build: `CENGINE_BUILD_ROUTING` (default `ON`) and
  `CENGINE_BUILD_TESTS` (default `ON`). The core builds standalone without routing.
- **C++23 requirement propagated** via `target_compile_features(... PUBLIC
  cxx_std_23)` — anything linking the targets inherits the standard.
- **Portable CMake presets** (`debug`, `release`, `asan`) in `CMakePresets.json`;
  machine-specific presets now live in a git-ignored `CMakeUserPresets.json`.
- **CI sanitizer job** (Linux, AddressSanitizer + UndefinedBehaviorSanitizer).
- **`StateCodes.hpp`** with `cengine::routing::kExitStateCode`, replacing the
  `"exit"` magic string.
- **Scene-lifetime regression test** (`SceneLifetimeTest`) exercising
  navigate → unload against the real implementations, meant to run under ASan.
- **Documentation**: Doxygen comments on all public headers; `Building` and
  `Usage` sections in the README (FetchContent consumption + minimal assembly).
- **`.ai/` working context**: improvement plan, ADR 0001, and verified
  build & test notes.

### Changed

- **Namespaces**: public types now live under `cengine::core` and
  `cengine::routing`.
- **Include paths**: headers are included as `<cengine/core/...>` and
  `<cengine/routing/...>`.
- **`IRouter` redesigned** to a leaner two-phase navigation contract
  (`requestState` / `hasPendingStateChange` / `commitStateChange` /
  `currentState` / `currentScene`).
- **`IState`** now belongs to the `cengine::routing` module (it is a routing
  concept, not a core one).

### Removed

- **`EngineManager::input()`** removed from the public API (dead code; the loop
  drives input through the game manager).

### Fixed

- **Scene-lifetime dangling window** in `GameManager::onExit()`: the scene
  reference could outlive the `commitStateChange()` that unloads it. The
  reference is now scoped so it cannot survive the unload, the lifetime contract
  is documented on `IRouter::currentScene()` / `ISceneRepository::getScene()`,
  and a regression test guards it.
- **Inverted names**: `IGameManager::shouldExist()` → `shouldExit()`, plus the
  correspondingly misleading test names.
- **Logging removed from the library** (no more `std::cout` in `GameManager`).
- **Dead code and stray includes** removed across core and routing.
- **CMake preset schema** lowered from `version: 8` (needs CMake ≥ 3.28) to
  `version: 4`, matching the declared `cmake_minimum_required(VERSION 3.23)`.

### Migrating from 0.0.1

- Replace the single `cengine_lib` link with the modular targets:
  ```cmake
  target_link_libraries(my_game PRIVATE
      cengine::core        # game loop + ports (always)
      cengine::routing     # scene/state routing (optional)
  )
  ```
- When pulling CEngine via `FetchContent`, disable its test suite before
  `FetchContent_MakeAvailable(cengine)` to avoid fetching GoogleTest at configure
  time: `set(CENGINE_BUILD_TESTS OFF)`.
- Update includes to the `<cengine/core/...>` / `<cengine/routing/...>` layout
  and qualify types with the `cengine::core` / `cengine::routing` namespaces.
- Rename `shouldExist()` to `shouldExit()` in your `IGameManager` implementations.
- Replace the `"exit"` state string with `cengine::routing::kExitStateCode`.

## [0.0.1] - Initial

- Initial single-target (`cengine_lib`) engine: game loop, scene/state
  management, and in-memory router.

[0.3.0]: https://github.com/mrmarmitt/cengine/compare/0.2.0...0.3.0
[0.2.0]: https://github.com/mrmarmitt/cengine/compare/0.1.0...0.2.0
[0.1.0]: https://github.com/mrmarmitt/cengine/compare/0.0.1...0.1.0
[0.0.1]: https://github.com/mrmarmitt/cengine/releases/tag/0.0.1
