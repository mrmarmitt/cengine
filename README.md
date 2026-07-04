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