# 19 - FlowRouter: extrair a mecanica da fachada de navegacao

- **Status:** todo
- **Prioridade:** baixa - micro-task; executar de carona na proxima mexida no
  modulo `routing` (ex.: task 17 ou 18), nao abrir ciclo proprio para ela.
- **Categoria:** Arquitetura / routing
- **Depende de:** 13 done (Router x Repository). Consumidores reais: 8puzzle
  e spaceinvaders (dois `GameRouter` quase identicos).
- **Breaking:** nao - helper novo opt-in; os jogos migram quando quiserem.

## Contexto

O padrao GameRouter + StateGameFlow se repetiu identico em dois jogos
(eram tres plataformas no 8puzzle; agora tambem o spaceinvaders): uma
fachada de dominio sobre o `IRouter` da cengine, com uma maquina de estados
cujas transicoes agendam a proxima cena.

A fachada tem duas metades:

- **Vocabulario** (do jogo, NAO entra na cengine): os metodos
  `menu()/game()/gameOver()/...` e os estados `MenuSG/GameSG/...`.
- **Mecanica** (repetida byte a byte entre os jogos): guardar o
  `shared_ptr<IRouter>`, fazer o `dynamic_cast` do estado atual para o tipo
  de fluxo do jogo (com erro claro se nao for), e delegar `setNextState`
  para `requestState`. ~15 linhas por jogo, zero palavras de jogo.

Passa no filtro anti-deposito combinado para a cengine: (1) mecanismo puro
sem vocabulario de jogo; (2) dois consumidores reais; (3) testavel dentro
da propria cengine.

## Objetivo

Um helper pequeno no `cengine::routing`, algo como:

```cpp
namespace cengine::routing {

// TFlow: o tipo-base da maquina de fluxo do jogo (deriva de IState).
template <typename TFlow>
class FlowRouter {
    std::shared_ptr<IRouter> m_router;

public:
    explicit FlowRouter(std::shared_ptr<IRouter> router);

    // Estado atual ja castado para o fluxo do jogo; lanca se o estado
    // corrente nao for um TFlow.
    [[nodiscard]] const TFlow& current() const;

    // Agenda a proxima cena (chamado pelas transicoes do fluxo).
    void setNextState(std::unique_ptr<IState> state) const;
};

} // namespace cengine::routing
```

O jogo escreve so o vocabulario:

```cpp
class GameRouter final : public cengine::routing::FlowRouter<StateGameFlow> {
public:
    using FlowRouter::FlowRouter;
    void menu()     { current().menu(*this); }
    void game()     { current().game(*this); }
    void gameOver() { current().gameOver(*this); }
    // ...
};
```

## Escopo Proposto

1. `FlowRouter<TFlow>` no modulo `routing` + testes (cast valido, cast
   invalido lanca, setNextState delega).
2. Migrar os dois consumidores (8puzzle e spaceinvaders) apagando a copia
   local — e o teste de aceite real.

## Fora do Escopo

- Mover estados/transicoes de qualquer jogo para a cengine.
- Mudar a semantica de navegacao (replace/push/pop — isso e a task 18).
- Gerar os metodos de vocabulario por macro/reflexao (a fachada explicita
  do jogo e uma feature, nao boilerplate).

## Criterios de Aceite

- [ ] `FlowRouter<TFlow>` com testes na suite da cengine.
- [ ] 8puzzle e spaceinvaders consumindo e sem a mecanica duplicada.
- [ ] Nenhum vocabulario de jogo dentro da cengine.

## Riscos

- Quase nenhum pelo tamanho; o unico cuidado e nao deixar o helper crescer
  alem da mecanica (qualquer coisa com nome de jogo fica de fora).

## Relacionado

- Task 13 - a separacao que criou o `IRouter` que a fachada envolve.
- Task 18 - se a navegacao ganhar push/pop, o vocabulario dos jogos cresce
  (ex.: `pause()` empilha), mas a mecanica extraida aqui nao muda — as duas
  tasks nao conflitam.
- 8puzzle `src/8puzzle/game/GameRouter.*` e spaceinvaders
  `src/spaceinvaders/game/GameRouter.*` - as duas copias de hoje.
