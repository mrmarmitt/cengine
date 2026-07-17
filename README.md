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
| `CENGINE_BUILD_COLLISION2D` | `ON` | Build `cengine::collision2d` (2D collision **detection**) |
| `CENGINE_BUILD_INPUT` | `ON` | Build `cengine::input` (keyboard **vocabulary**) |
| `CENGINE_BUILD_AUDIO` | `ON` | Build `cengine::audio` (the `play(id)` **port**; backends live in platforms) |
| `CENGINE_BUILD_TESTS` | `ON` | Build the test suite |

A consumer links only what it needs:

```cmake
target_link_libraries(my_game PRIVATE
    cengine::core         # always
    cengine::routing      # optional
    cengine::collision2d  # optional
    cengine::input        # optional
    cengine::audio        # optional
)
```

### `cengine::input` — the contract, never the capture

`Key`/`KeyEvent` is the vocabulary the *scenes* speak; `Keyboard` is the
mechanism between the platform (which pushes) and the scene (which reads):

```cpp
#include <cengine/input/Keyboard.hpp>

using namespace cengine::input;

// platform (WndProc, terminal, bindings — never the engine's business):
keyboard.pushKey({ Key::Escape });        // an edge: "the player pressed"
keyboard.pushHeldKey(Key::Left, true);    // continuous: "it is down right now"
keyboard.clearHeldKeys();                 // focus lost: the KEYUP never arrives

// scene:
if (keyboard.readKey().key == Key::Escape) { /* at most ONE event per input() */ }
paddle.move(keyboard.heldAxis(Key::Left, Key::Right));
```

The two readings do not substitute for one another. The **edge queue** navigates
menus and types text — and its one-event-per-`input()` cap is *semantics*: it is
what stops a repeated key from walking through three menu items in a single
frame. The **held state** moves things: a ship does not travel on edges, it
travels while the arrow is down, every frame.

### `cengine::audio` — the port, never the speaker

`Player` is the contract the *scenes* speak: *play this sound from the game's
catalog*. The backend (synthesis, voice pools, XAudio2, a device) lives in the
platform and implements the port — the exact same cut as input, in the opposite
direction (there the platform pushes and the scene reads; here the scene asks
and the platform delivers):

```cpp
#include <cengine/audio/Player.hpp>

enum class Sound : uint8_t { Jump, Coin, Stomp };  // the GAME's catalog

// scene (holds a cengine::audio::Player& injected by the composition root):
audio.play(Sound::Coin);   // enum sugar — what crosses the port is the id
```

Mute is normal degradation, not an error: without a sound device, `play()` is a
safe no-op. The engine never knows what a sound *means* or how it is made — the
catalog, the event→sound mapping and the synthesis recipes are all the game's
and the platform's business. The port is deliberately minimal (`play(id)` and
nothing else): its two real consumers (breakout and mario-bros) confirmed
neither volume, priority nor stop was ever needed.

### `cengine::collision2d` — detection, not physics

The module answers exactly one question — *do these two shapes touch?* — over
`Aabb`, `Circle` and the mixed pair:

```cpp
#include <cengine/collision2d/Intersects.hpp>

using namespace cengine::collision2d;

if (intersects(Circle{ shipPos, shipRadius }, Circle{ rockPos, rockRadius }))
{
    // The GAME decides what a hit means: score, lose a life, split the rock,
    // end the run. The engine never owns entities and never resolves anything.
}
```

What the module deliberately does **not** know:

- **the shape of the world.** A wrapping arena (a torus) is *game* policy: the
  consumer computes the shortest delta in its own topology and asks the engine
  with an already-corrected position. A game whose arena does not wrap just asks
  directly.
- **the axis convention.** Y-up or Y-down makes no difference to an overlap.
- **units.** Pixels, cells or arena units are the game's business.

That line — mechanism in the engine, policy in the game — is
[ADR 0002](.ai/decisions/0002-criterio-de-promocao-anti-deposito.md); the module's
test suite carries the two real cases that justified it (Space Invaders' AABB
and Asteroids' circle).

## Usage

CEngine is consumed via CMake's `FetchContent` (this is how the sibling
[8Puzzle](https://github.com/cengine-dev/8puzzle) project uses it):

```cmake
include(FetchContent)

FetchContent_Declare(
  cengine
  GIT_REPOSITORY https://github.com/cengine-dev/cengine.git
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
    //    owned() = engine-owned mode: the engine owns the window and the loop.
    auto engine = core::EngineManager::owned(
        std::make_unique<MyWindowManager>(),
        std::move(gameManager));
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

Each frame runs `input()` → `update(dt)` → `draw()` on the active scene. In
engine-owned mode the window's frame *wraps* the game phases, with a symmetric
begin/end:

```
window.update()   // OS events, frame setup (acquire image, begin command buffer...)
game phases       // onEnter -> input -> update(fixedDt) 0..N -> render -> onExit
window.present()  // close & present the frame (end cmd, submit, queuePresent)
```

`present()` runs after every frame — including the last one, when the game has
just routed to the exit state: what was drawn is presented before `cleanup()`.
Platforms with no present concept (a plain terminal) implement it empty.

The loop uses a **fixed timestep** ("fix your timestep"): frame time is measured
with a monotonic clock and consumed in constant `dt` steps — `update` runs
**0..N times per frame, always with the same `dt`** (default 1/60 s,
configurable in the `owned()`/`hosted()` factories). Put simulation (animations,
timers, physics) in `update(dt)`; never measure time inside a scene.

### Hosted mode (`frame(dt)`)

Some platforms invert control: a framework owns the loop and calls *you* back
each frame (The-Forge's `IApp::Update/Draw`, editors, browsers). For those
hosts, skip `start()` and drive the engine with **`frame(dt)`** — it runs one
complete frame (`onEnter` → `input` → `update(fixedDt)` 0..N times → `render`
→ `onExit`) with the same internal fixed-timestep accumulator, and returns
`false` when the game routed to the exit state:

```cpp
// window, pacing and input pump belong to the HOST — hosted() takes no window.
auto engine = core::EngineManager::hosted(std::move(gameManager));

// inside the host's per-frame callback (dt measured by the host):
if (!engine.frame(core::Seconds{dt})) {
    // stop calling, shut the host down, and call engine.cleanup() on teardown
}
```

`start()` is implemented on top of `frame()`, so the fixed-timestep guarantees
are identical in both modes. The mode is decided at construction: calling
`start()` on a `hosted()` engine throws `std::logic_error`, and neither factory
accepts null collaborators. If the host clamps its own `dt`, the engine's
`maxFrameTime` clamp composes harmlessly on top.

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