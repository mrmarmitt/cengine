# 13 — Separar responsabilidades: Router (navegação) × Repository (cenas)

- **Status:** todo
- **Prioridade:** 🔴 Alta (arquitetural)
- **Categoria:** Arquitetura
- **Depende de:** 05b (API do Router estável), 12 (IScene enxuta — fazer antes,
  reduz a superfície desta cirurgia)
- **Decisão de referência:** [ADR 0001](../decisions/0001-modular-core-vs-modules.md)
- **Breaking:** sim — coordenar com bump de versão (0.2.0) junto da tarefa 12.

## Problema

Hoje `RouterInMemory` e `SceneRepository` são **a mesma abstração dividida em
duas**:

1. **Pass-through puro.** Todo método de `RouterInMemory` delega uma única
   chamada ao `ISceneRepository` (`RouterInMemory.cpp`) — o router não tem
   estado nem decisão própria além do unload no commit.
2. **Repository com dois papéis.** `ISceneRepository` acumula (a) cache de
   cenas com factories lazy e (b) o par de estados atual/próximo da navegação
   em duas fases — inclusive expõe `hasPendingStateChange()`, duplicando a API
   do router uma camada abaixo.
3. **Sintomas na borda.** Nomes como `persisteCurrentState()` (semântica
   invertida: promove o *próximo* a atual) e `getCurrentStateGame()` (sufixo
   sem função) sobrevivem porque a interface é grande demais para o que faz.
4. **Prototype sem necessidade.** O par atual/próximo é mantido por
   `IState::clone()` (duas clonagens já no construtor). Com posse clara dos
   estados dentro do router, `std::move` basta e `clone()` perde o último
   caller da engine.

Pergunta de fundo (levantada na avaliação de arquitetura): **de quem é a
responsabilidade pelo gerenciamento de telas?**

## Objetivo

Uma responsabilidade por peça:

- **Router** = dono da **máquina de estados** (atual/próximo, request/commit,
  política de unload na transição).
- **SceneRepository** = **provedor de cenas** (registro de factories,
  instanciação lazy, cache, unload) — sem nenhuma noção de estado.

## Opções (decidir na tarefa)

1. **Router dono dos estados; Repository só provê cenas (recomendada).**

   ```cpp
   // ISceneRepository enxuto: provisionamento, zero navegação
   class ISceneRepository {
   public:
       virtual ~ISceneRepository() = default;
       virtual void registerFactory(const std::string& name,
                                    std::function<std::unique_ptr<core::IScene>()> factory) = 0;
       [[nodiscard]] virtual core::IScene& getScene(const std::string& name) = 0;
       virtual void unloadScene(const std::string& name) = 0;
       virtual void unloadAll() = 0;
   };

   // RouterInMemory: dono da máquina de estados
   class RouterInMemory final : public IRouter {
       std::shared_ptr<ISceneRepository> m_scenes;
       std::unique_ptr<IState> m_currentState;
       std::unique_ptr<IState> m_nextState; // nullptr = nada pendente
   public:
       RouterInMemory(std::shared_ptr<ISceneRepository> scenes,
                      std::unique_ptr<IState> initialState);

       void requestState(std::unique_ptr<IState> state) override; // m_nextState = move(state)
       bool hasPendingStateChange() const override;               // m_nextState != nullptr
       void commitStateChange() override;  // unload(atual) + m_currentState = move(m_nextState)
       // currentState()/currentScene() como hoje
   };
   ```

   Consequências boas que vêm de graça:
   - `hasPendingStateChange()` existe em **um** lugar (`m_nextState != nullptr`
     em vez de comparar códigos — também corrige o caso "request para o mesmo
     código" ser indistinguível de "nada pendente").
   - `persisteCurrentState`/`persistNextState`/`getCurrentStateGame`/
     `getNextStateGame` deixam de existir — os nomes ruins morrem com a
     interface.
   - `IState::clone()` fica **sem caller na engine** → pode ser removida da
     interface (fecha a evolução prevista na tarefa 10, opção 3, sem esforço
     extra).
   - A `IRouter` (porta pública) **não muda** — o redesenho é interno ao módulo.

2. **Fusão total: Router absorve factories e cache; Repository some.** Menos
   peças, porém mistura política de navegação com política de caching — e a
   dupla fase request/commit passa a conviver com mapa de factories na mesma
   classe. Ganha pouco sobre a opção 1 e perde o ponto de extensão de
   provisionamento (ex.: repositório que pré-carrega cenas).

3. **Tirar o gerenciamento de cenas da engine (fica no jogo).** O módulo
   routing passa a conter só a máquina de estados; a resolução estado→cena é
   injetada pelo jogo (ex.: `std::function<core::IScene&(const IState&)>` no
   router, ou o jogo implementa `ISceneRepository` por conta própria). É a
   leitura mais radical de "talvez sair desse projeto": a engine não opina
   sobre caching/factory de telas. Custo: todo consumidor reimplementa
   provisionamento (e os bugs de lifetime que a tarefa 06 já pagou para
   resolver); o valor do módulo routing cai pela metade.

> Recomendação: **opção 1**. Mantém o módulo útil e o breaking mínimo (a porta
> `IRouter` não muda; muda a fiação no `main` do jogo). A opção 3 fica
> registrada como direção possível caso o routing vire estorvo — o desenho da
> opção 1 não a impede: com o repository já reduzido a provedor, trocá-lo por
> uma implementação do jogo é plugável.

## Passos

1. Enxugar `ISceneRepository` para os 4 métodos de provisionamento; mover
   atual/próximo para dentro de `RouterInMemory` (com `m_nextState = nullptr`
   como "nada pendente"; `commitStateChange()` mantém o unload da cena que sai).
2. Novo construtor `RouterInMemory(repository, initialState)`; atualizar a
   montagem nos testes.
3. Remover `IState::clone()` (sem caller) e simplificar implementações nos
   testes/mocks.
4. Atualizar `MockSceneRepository` (encolhe) e redistribuir os testes:
   máquina de estados → `RouterInMemoryTest`; factories/cache/unload →
   `SceneRepositoryTest`. `SceneLifetimeTest` deve continuar passando sem
   mudança de asserções (o contrato de lifetime não muda).
5. Revisar o Doxygen das duas interfaces (a divisão de papéis vira a primeira
   linha de cada uma) e o diagrama do ADR 0001 se necessário.

## Critérios de aceite

- [ ] **Eviction da cena ativa resolvido** (achado do review do PR #11): com o
      router dono da política de unload, descarregar a cena do estado ativo
      sem navegação deve (a) ser impossível pela API pública, ou (b) disparar
      reativação (`onEnter()` na instância recriada). Hoje a recriação lazy
      após `unloadScene`/`unloadAll` devolve uma cena que nunca recebe
      `onEnter()`, porque o tracking do `GameManager` é por código de estado
      (rastrear por ponteiro não serve: o alocador tende a reusar o endereço
      na recriação imediata). Cobrir com teste de integração.
- [ ] `ISceneRepository` sem nenhum método de estado/navegação.
- [ ] `RouterInMemory` é o único dono do par atual/próximo;
      `hasPendingStateChange()` implementado por ponteiro nulo, não por
      comparação de código.
- [ ] `IState` sem `clone()`.
- [ ] Porta `IRouter` inalterada; `GameManager` inalterado (fora renomes).
- [ ] Suíte verde (incluindo `SceneLifetimeTest` sem mudanças de asserção).

## Riscos

Médio: mexe em interface com mocks e testes em cima, e quebra a fiação do
consumidor. Mitigação: a porta `IRouter` congelada limita o raio da explosão;
fazer depois da 12 (IScene menor = menos mocks para tocar duas vezes).

## Pendências fora do escopo

- **8Puzzle:** o `main.cpp` semeia o `SceneRepository` com o estado inicial e o
  compartilha com o `GameRouter` do jogo. Na nova fiação, o estado inicial vai
  para o `RouterInMemory`, e a fachada `GameRouter` deve passar a envolver o
  `IRouter` (não o repositório). Ajustar ao consumir a 0.2.0.
- **Política de keep-alive de cenas** (não destruir a cena que sai no commit):
  fica mais fácil depois desta tarefa (é decisão isolada dentro de
  `commitStateChange()`), mas é tarefa própria — depende da 12 para `onEnter`
  voltar a rodar em cenas mantidas vivas.
