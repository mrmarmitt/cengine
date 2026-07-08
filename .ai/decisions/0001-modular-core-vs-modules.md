# ADR 0001 — Core mínimo (loop) + módulos opcionais no mesmo repo

- **Status:** Aceito
- **Data:** 2026-07-04
- **Contexto de decisão:** definição de escopo do CEngine e de onde vivem
  responsabilidades como roteamento de telas e física.

## Contexto

O CEngine nasceu como projeto de estudo com uma mentalidade específica:

> Uma engine que **não escolhe uma biblioteca gráfica**. O CEngine resolve o
> **game loop** e entrega um **framework** para os jogos. No projeto do jogo ficam
> as **regras de negócio**, a **construção das telas** e o **gráfico em si**. O
> **motor do loop** fica no CEngine.

O código atual, porém, embute no core uma máquina opinativa de **roteamento de
telas / estados**: `GameManager` (concreto), `IRouter` + `RouterInMemory`,
`ISceneRepository` + `SceneRepository`, `IState`. Isso faz o core ser mais que "o
loop" — ele já decide *como* navegar entre telas. Essa camada foi identificada
como a maior fonte de dívida arquitetural da engine.

Surgiu então a pergunta: onde colocar roteamento (e, no futuro, física)?
Repositório separado? Um módulo opcional `cengine::routing`? Dentro do core?

## Decisão

**Separar duas questões que estavam misturadas:**

1. **Direção de dependência (o que importa):**
   > O core (loop + portas) não depende de ninguém. Módulos (routing, physics)
   > dependem do core. O jogo escolhe quais módulos linkar.

2. **Empacotamento (logística):** **monorepo com múltiplos targets CMake**
   (monolito modular). **Não** criar repositórios separados agora.

### Fronteira de responsabilidades

```
┌────────────────── cengine::core (framework / loop) ──────────────────┐
│  EngineManager    → dono do game loop (o "motor")                     │
│  IWindowManager   → porta de plataforma/janela                        │
│  IGameManager     → porta "o que fazer a cada frame"                  │
│  IScene           → porta onEnter/input/render/onExit                 │
│  (core não depende de nenhum módulo)                                  │
└───────────────────────────────────────────────────────────────────────┘
        ▲                              ▲
 cengine::routing              cengine::physics        (targets opcionais)
 (Router, SceneRepository,     (a definir)
  IState, máquina de estados)
        ▲                              ▲
        └──────────────┬───────────────┘
             projeto do jogo (ex.: 8Puzzle)
   regras de negócio + telas + gráfico + escolhe o que linkar
```

### Layout de repositório alvo

```
cengine/
├── core/                    → cengine::core
│   └── include/cengine/core/…
├── modules/
│   ├── routing/             → cengine::routing  (PUBLIC dep: cengine::core)
│   │   └── include/cengine/routing/…
│   └── physics/             → cengine::physics  (futuro)
├── tests/
└── CMakeLists.txt
```

### Como o jogo escolhe (opt-in por target)

```cmake
FetchContent_MakeAvailable(cengine)

target_link_libraries(8Puzzle PRIVATE
    cengine::core        # sempre
    cengine::routing     # opt-in
    # cengine::physics   # não linkado → não entra no binário
)
```

Quem não quer o roteador não linka e implementa a própria navegação. O core
continua utilizável sozinho.

## Consequências

### Positivas

- Core enxuto e fiel à mentalidade "engine = loop + portas".
- Módulos são opt-in; nenhum jogo é forçado a carregar o que não usa.
- Fronteiras de target limpas tornam a extração futura para repo separado
  quase gratuita (mover uma pasta).

### Negativas / custos

- **Breaking change** para consumidores: `IRouter`, `SceneRepository`, `IState`
  saem do core (mudam de include path/namespace e viram target à parte). O
  8Puzzle terá de linkar `cengine::routing`. Coordenar com bump de versão
  (provável `0.1.0`) e com o ADR de namespace (tarefa 03).
- Configuração CMake mais elaborada (múltiplos targets, gating opcional dos
  módulos para não buildar o que não é linkado).

## Critérios para revisitar (quando abrir repo separado)

Extrair um módulo para repositório próprio **só** quando surgir um gatilho real:

- Cadência de release independente do core;
- **Dependência externa pesada** (ex.: física com Box2D/Jolt) que não se quer
  forçar sobre todo consumidor do core — este é o gatilho mais provável;
- Licença distinta por módulo;
- Donos/times distintos.

Nenhum se aplica hoje → monorepo, **projetado para ser extraível**.

## Notas de implementação

- **Física tem uma sutileza:** costuma exigir *fixed timestep* (separar
  `update(dt)` de `render()`). ~~Hoje o loop (`EngineManager::run()`) não tem
  conceito de tempo/`dt`.~~ **Resolvido na
  [Tarefa 14](../task/14-time-in-the-loop.md)**: o loop mede tempo com relógio
  monotônico e executa `update(dt)` em passos fixos (acumulador + teto
  anti-espiral) — o contrato que um futuro `cengine::physics` consome.
- `IState` (máquina de estados) é considerado conceito de **roteamento**, não de
  core — vai para `cengine::routing`.

## Relacionado

- Reformula a **[Tarefa 05](../task/05-redesign-irouter.md)**: de "redesenhar o
  `IRouter`" para "extrair o roteamento do core para `cengine::routing`".
- Impacta a **[Tarefa 03](../task/03-namespace.md)** (namespaces por camada:
  `cengine::core`, `cengine::routing`).
- Impacta a **[Tarefa 06](../task/06-scene-lifetime.md)** (ciclo de vida das
  cenas migra junto do roteamento).
- Impacta a **[Tarefa 07](../task/07-cpp-standard.md)** (contrato entre core e
  consumidores/módulos).
