# 05a — Reorganização estrutural: `cengine::core` + `cengine::routing`

- **Status:** done ✅ (2026-07-04, branch `feature/05a-restructure-modules`)
- **Prioridade:** 🔴 Alta (arquitetural)
- **Categoria:** Arquitetura / build
- **Depende de:** 01 (rede de segurança de `override` ativa)
- **Absorve:** [Tarefa 03](03-namespace.md) (namespaces por camada nascem aqui)
- **Bloqueia:** 05b, 06
- **Decisão de referência:** [ADR 0001](../decisions/0001-modular-core-vs-modules.md)

> **Move puro, sem mudança de comportamento.** Esta tarefa move arquivos e
> reconfigura o CMake para o layout modular do ADR 0001. **Nenhuma lógica muda.**
> O critério de aceite é brutal: `ctest` continua **30/30 idêntico**. O redesenho
> do roteador (enxugar `IRouter`, Scene/Screen, desacoplar) fica para a 05b.

## Objetivo

Sair de uma única lib (`cengine_lib`) para o layout modular:

```
cengine/
├── core/                    → cengine::core   (namespace cengine::core)
│   └── include/cengine/core/…   EngineManager, IWindowManager, IGameManager, IScene
├── modules/
│   └── routing/             → cengine::routing (namespace cengine::routing)
│       └── include/cengine/routing/…  IRouter, RouterInMemory, ISceneRepository,
│                                       SceneRepository, IState, GameManager
├── tests/
└── CMakeLists.txt
```

## Passos

1. **Definir a fronteira do core.** Fica no core: `EngineManager`,
   `IWindowManager`, `IGameManager`, `IScene`. Vai para routing: `IRouter`,
   `RouterInMemory`, `ISceneRepository`, `SceneRepository`, `IState` e o
   `GameManager` (router-based).
2. **Mover os arquivos** para `core/` e `modules/routing/` conforme o layout.
   Ajustar todos os `#include` (fim dos caminhos relativos monstruosos — casa com
   a tarefa 09, mas aqui é consequência natural do move).
3. **Aplicar namespaces por camada** (absorve a tarefa 03): `cengine::core::*` e
   `cengine::routing::*`. Atualizar testes/mocks para qualificar os nomes.
4. **CMake multi-target:**
   - `add_library(cengine_core …)` expondo `core/include` como `PUBLIC`;
     alias `cengine::core`.
   - `add_library(cengine_routing …)` com
     `target_link_libraries(cengine_routing PUBLIC cengine::core)`;
     alias `cengine::routing`.
   - **Gating opcional** do módulo routing (ex.: `option(CENGINE_BUILD_ROUTING …)`)
     para não buildar quando o consumidor não linka.
5. **Reorganizar os testes** por camada: testes do loop (`EngineManager`) sob o
   core; testes de router/cenas/estado sob routing. Manter os dois na suíte
   `ctest`.
6. **Não redesenhar nada.** Se bater vontade de "já que estou aqui, arrumo o
   IRouter" — resistir. Isso é a 05b.

## Critérios de aceite

- [x] `cengine::core` e `cengine::routing` existem como targets separados, com
      `routing` dependendo `PUBLIC` do `core`.
- [x] Namespaces `cengine::core` / `cengine::routing` aplicados.
- [x] `ctest` continua **30/30**, comportamento idêntico (é move, não redesign).
- [x] Um consumidor consegue linkar só `cengine::core` e buildar o loop sem routing.

## Resultado da execução

- Novo layout: `core/` (cengine::core) + `modules/routing/` (cengine::routing,
  dep `PUBLIC` no core) + `tests/` (por camada: `core/`, `routing/`, `mock/`).
- Includes migrados para `<cengine/core/…>` e `<cengine/routing/…>` (fim dos
  caminhos relativos `../../../`). `IState` migrado de core → routing.
- Namespaces `cengine::core` / `cengine::routing` aplicados em toda a base.
- CMake: top-level agregador com `option(CENGINE_BUILD_ROUTING)` e
  `option(CENGINE_BUILD_TESTS)`; `target_compile_features(cengine_core PUBLIC
  cxx_std_23)`.
- Verificação: build limpo, **`ctest` 30/30 verde**, e core buildando **sozinho**
  com `-DCENGINE_BUILD_ROUTING=OFF` (opt-in comprovado).
- Absorveu a tarefa 03 (namespaces por camada).

## Riscos

Médio — concentrado no CMake (multi-target + gating), não na lógica. **É breaking
para o 8Puzzle:** `cengine_lib` deixa de existir; o consumidor passará a linkar
`cengine::core` (+ `cengine::routing` se quiser). Coordenar com bump `0.1.0`.
Atualizar o 8Puzzle é parte da 05b (quando o desenho final estabilizar) ou um
passo imediato aqui, se preferir mantê-lo compilando.
