# 05b - Redesenhar o `IRouter` (dentro de `cengine::routing`)

- **Status:** done (2026-07-04, branch `feature/05b-redesign-irouter`)
- **Prioridade:** Alta (arquitetural)
- **Categoria:** Arquitetura
- **Depende de:** 05a (estrutura modular ja pronta)
- **Bloqueia:** 06
- **Decisao de referencia:** [ADR 0001](../decisions/0001-modular-core-vs-modules.md)

> A 05a moveu o roteamento para `cengine::routing` sem mudar comportamento.
> Esta tarefa faz a limpeza de design do roteador dentro do modulo.

## Problemas corrigidos

Arquivo: `modules/routing/include/cengine/routing/IRouter.hpp`

1. **Interface gorda (viola ISP).** A interface saiu de ~12 metodos para 5:
   `requestState`, `hasPendingStateChange`, `commitStateChange`, `currentState`
   e `currentScene`.
2. **Acoplamento entre portas.** `IRouter.hpp` nao inclui mais
   `ISceneRepository.hpp`; usa forward declarations para `IState` e `IScene`.
3. **Vocabulario Scene vs Screen.** A API publica do router passou a falar
   `Scene`: `currentScene`, `hasPendingStateChange`, `commitStateChange`.

## API resultante

```cpp
namespace cengine::routing {
class IRouter {
public:
    virtual ~IRouter() = default;

    virtual void requestState(std::unique_ptr<IState> state) = 0;
    [[nodiscard]] virtual bool hasPendingStateChange() const = 0;
    virtual void commitStateChange() = 0;

    [[nodiscard]] virtual const IState& currentState() const = 0;
    [[nodiscard]] virtual core::IScene& currentScene() = 0;
};
}
```

Notas de design:

- `currentState()` retorna `const IState&`, porque callers nao devem mutar o
  estado atual pelo router.
- `currentScene()` nao e `const`, porque o acesso pode instanciar a cena lazy via
  repository e retorna uma cena mutavel para o ciclo `onEnter/input/render/onExit`.
- `getNext*` foi removido por falta de caller real.
- `...Name`/`...Code` foram removidos; callers usam `currentState().getCode()`
  ou `currentState().getName()` quando necessario.

## Resultado da execucao

- `IRouter` reduzido para 5 metodos coesos.
- `RouterInMemory` reescrito para a nova API.
- `GameManager` atualizado para `currentScene()`, `hasPendingStateChange()` e
  `commitStateChange()`.
- `ISceneRepository::isNextStateEqualsToCurrentScene()` renomeado para
  `hasPendingStateChange()` para alinhar nome e semantica.
- `MockRouter`, `MockSceneRepository` e testes de routing atualizados.
- Testes de delegacao removidos quando cobriam apenas metodos que deixaram de
  existir (`getCurrentStateGameName`, `getCurrentStateGameCode`).

## Criterios de aceite

- [x] `IRouter` com no maximo ~5-6 metodos coesos.
- [x] `IRouter.hpp` nao inclui `ISceneRepository.hpp`.
- [x] Vocabulario unico (Scene) na API do router.
- [x] Testes de integracao do loop passam sem mudanca de comportamento.

## Verificacao

- `cmake --build --preset msys2-mingw`
- `ctest --test-dir out/build/msys2-mingw --output-on-failure`
- Resultado: 28/28 testes passaram.

## Pendencias fora do escopo

- Atualizar o consumidor de referencia 8Puzzle para a nova API do router e para
  o target `cengine::routing` quando esse repositorio estiver no workspace.
- `IGameManager::shouldExist()` continua com nome invertido; fica para a tarefa
  02.
- `GameManager::cleanup()` ainda escreve em `std::cout`; fica para a tarefa 04.
- A tarefa 06 deve decidir a estrategia de lifetime das cenas sobre a API nova.
