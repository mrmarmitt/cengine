# 05b — Redesenhar o `IRouter` (dentro de `cengine::routing`)

- **Status:** todo
- **Prioridade:** 🔴 Alta (arquitetural)
- **Categoria:** Arquitetura
- **Depende de:** 05a (estrutura modular já pronta)
- **Bloqueia:** 06
- **Decisão de referência:** [ADR 0001](../decisions/0001-modular-core-vs-modules.md)

> A 05a já **moveu** o roteamento para `cengine::routing` sem mudar comportamento.
> Esta tarefa faz a **limpeza de design** do roteador — agora isolada no módulo,
> sem risco de contaminar o core.

## Problemas de desenho a corrigir

Arquivo (após a 05a): `modules/routing/…/IRouter.hpp`

1. **Interface gorda (viola ISP).** ~12 métodos, com 6 pares quase idênticos
   `getCurrent* / getNext*`. Os `...Name` e `...Code` são só delegação para
   `IState::getName()/getCode()` — não precisam estar na interface do Router.

2. **Acoplamento entre portas.** `IRouter.hpp` inclui
   `repository/ISceneRepository.hpp`. Resolver com forward declaration + injeção.

3. **Vocabulário Scene vs Screen.** Unificar em **Scene** (alinha com
   `IScene`/`SceneRepository`), eliminando "Screen"
   (`getCurrentCachedScreen`→`currentScene`, `hasNextScreen`, `goToNextScreen`).

## Direção de design proposta (revisar antes de codar)

```cpp
namespace cengine::routing {
class IRouter {
public:
    virtual ~IRouter() = default;

    void requestState(std::unique_ptr<IState> next);          // ex-setNextState
    [[nodiscard]] bool hasPendingStateChange() const;         // ex-hasNextScreen
    void commitStateChange();                                 // ex-goToNextScreen

    [[nodiscard]] IState& currentState() const;
    [[nodiscard]] IScene& currentScene() const;
};
}
```

- `...Name`/`...Code` saem: quem precisar chama `currentState().getName()/getCode()`.
- `getNext*` só permanece se houver caso de uso real de espiar a próxima cena
  antes do commit; senão, remover (YAGNI).
- Confirmar o conjunto mínimo consultando **todos** os chamadores reais
  (hoje: `GameManager`).

## Passos

1. Levantar (grep) os chamadores reais de cada método de `IRouter`.
2. Validar o desenho enxuto acima.
3. Reescrever `RouterInMemory`; mover as delegações `...Name/...Code` para os
   chamadores.
4. Remover o include de `ISceneRepository.hpp` do header de `IRouter`.
5. Renomear Screen → Scene em toda a cadeia (Router, GameManager, testes).
6. Atualizar `MockRouter` e os testes.
7. Atualizar o consumidor de referência (8Puzzle) para a nova API + link
   `cengine::routing`.

## Critérios de aceite

- [ ] `IRouter` com no máximo ~5–6 métodos coesos.
- [ ] `IRouter.hpp` não inclui `ISceneRepository.hpp`.
- [ ] Vocabulário único (Scene).
- [ ] Testes de integração do loop passam **sem mudança de comportamento**.

## Riscos

Médio/Alto — mexe em interface pública do módulo. Mitigadores: 01–04 estabilizaram
o terreno, 05a isolou o roteamento, e o `EngineManagerIntegrationTest` trava
regressão de comportamento por call-log.
