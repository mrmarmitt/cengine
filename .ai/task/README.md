# Plano de Melhoria — CEngine

Este diretório contém o plano de melhoria da engine, derivado de uma avaliação
técnica (arquitetura + boas práticas). Cada arquivo é uma tarefa independente,
numerada na **ordem de desenvolvimento recomendada**.

## Princípio da ordenação

A mudança de maior impacto é **tirar o roteamento do core** e reorganizar em
módulos (`cengine::core` + `cengine::routing`, ver [ADR 0001](../decisions/0001-modular-core-vs-modules.md)).
Como é grande e *breaking*, ela é feita cedo — porém **precedida de uma rede de
segurança barata** e **separando "mover" de "redesenhar"**.

**Ordem de execução recomendada** (não é estritamente a ordem numérica):

1. **Rede de segurança (01):** adicionar `override` — trivial e endurece o
   compilador **antes** da grande movimentação de código.
2. **Reorganização estrutural (05a):** mover para `core/` + `modules/routing/`,
   CMake multi-target, namespaces por camada. **Move puro, sem mudar
   comportamento** (absorve a antiga tarefa 03 de namespace).
3. **Redesenho do roteador (05b):** enxugar o `IRouter`, unificar Scene/Screen,
   desacoplar — já isolado dentro de `cengine::routing`.
4. **Correções de qualidade (02, 04, 06):** nomes invertidos, `const`/logging e
   ciclo de vida das cenas — agora no lugar final.
5. **Ecossistema/build (07–08):** padrão C++, presets, CI.
6. **Limpeza final e documentação (09–11).**
7. **Ciclo 0.2.0 — design de API (12 → 13):** primeiro tirar a contabilidade
   de `onEnter` da `IScene` (12, cirurgia pequena), depois separar Router
   (máquina de estados) de Repository (provedor de cenas) (13). As duas são
   *breaking* — agrupar no bump 0.2.0. ✅ (release 0.2.0 publicado)
8. **Ciclo 0.3.0 — tempo no loop (14):** `update(dt)` com fixed timestep no
   `EngineManager` — a última lacuna estrutural do core e o pré-requisito do
   futuro `cengine::physics` (ADR 0001). *Breaking* nas portas
   `IGameManager`/`IScene` — âncora do bump 0.3.0. ✅ (release 0.3.0 publicado)
9. **Ciclo 0.4.0 — modo hospedado (15):** `frame(dt)` para hosts com inversão
   de controle (The-Forge). Desenho validado pela fase 1 da PoC The-Forge no
   8Puzzle; adaptador `8PuzzleForge` migrado. ✅ (release 0.4.0 publicado)
10. **Ciclo 0.5.0 — fim do quadro na janela (16):** `IWindowManager::present()`
    para o modo próprio fechar/apresentar o quadro depois do `render()` —
    pré-requisito da fase 2 da PoC The-Forge (task 02 do 8Puzzle, modo
    biblioteca). *Breaking* pequeno na interface — âncora do bump 0.5.0.
    ✅ (release 0.5.0 publicado; validado pelo degrau 2 da fase 2 —
    `TheForgeWindowManager` com o quadro de GPU no par `update()`/`present()`)
11. **Ciclo 0.6.0 — janela obrigatória (21) + FlowRouter de carona (19):**
    remover a hipótese do `nullptr` no `EngineManager` (*breaking*, âncora
    do bump — saiu pela **opção B**: factories `owned()`/`hosted()`) e
    extrair a mecânica da fachada de navegação (`FlowRouter<TFlow>`,
    opt-in) na mesma visita. O aceite mudou em relação ao proposto:
    **8puzzle e spaceinvaders foram estacionados na 0.5.0 como documentação
    viva (ADR 0003)** — quem valida o novo desenho é a suíte da cengine e o
    asteroids, que nasce como terceiro consumidor. A task 20 segue
    **estacionada** (gate não disparou — ver ADR 0002).
    ✅ (release 0.6.0 publicado; suíte 49/49 verde)

> Regra prática: manter a suíte de testes **verde a cada tarefa**. Nenhuma
> tarefa deve ser mergeada com testes quebrados.
>
> **Nota sobre a tarefa 03:** o namespace deixou de ser tarefa isolada — ele
> nasce junto da reorganização em módulos (05a), como `cengine::core` /
> `cengine::routing`. O arquivo 03 permanece como referência técnica.

## Decisões de arquitetura (ADRs)

O plano é guiado por decisões registradas em [`../decisions/`](../decisions/).
Ler antes de executar as tarefas de arquitetura:

- [ADR 0001 — Core mínimo (loop) + módulos opcionais](../decisions/0001-modular-core-vs-modules.md):
  o core é só o game loop + portas; roteamento e física são módulos opt-in no
  mesmo repo. Reformula a tarefa 05 (dividida em 05a/05b) e absorve a 03.
- [ADR 0002 — Critério de promoção (filtro anti-depósito)](../decisions/0002-criterio-de-promocao-anti-deposito.md):
  código só entra na engine se for mecanismo puro (sem vocabulário de jogo),
  com ≥ 2 **evidências reais** de necessidade e testável na própria cengine.
  Governa as tarefas 17–21 e toda promoção futura.
  **Emenda 1 (2026-07-14):** um jogo estacionado continua valendo como
  evidência — congelar suspende a manutenção, não o aprendizado. O pedágio é
  que a suíte da engine precisa **encarnar o caso de uso do jogo congelado**
  (foi o que destravou a tarefa 17).
- [ADR 0003 — Consumidores estacionados como documentação viva](../decisions/0003-consumidores-estacionados-documentacao-viva.md):
  8puzzle e spaceinvaders ficam pinados na cengine 0.5.0 e saem dos
  critérios de aceite; o asteroids é o consumidor de validação do 0.6.0 em
  diante.

## Índice

| # | Tarefa | Prioridade | Categoria |
|---|--------|------------|-----------|
| 01 | [Adicionar `override` em todas as sobrescritas](01-add-override.md) | 🔴 Alta | Boas práticas |
| 02 | [Corrigir nomes invertidos (`shouldExit`, `isNextState…`)](02-fix-inverted-names.md) | 🔴 Alta | Boas práticas |
| 03 | [Introduzir namespaces](03-namespace.md) *(absorvida pela 05a)* | 🟡 Média | Arquitetura |
| 04 | [`const`-correctness + remover logging da lib](04-const-and-logging.md) | 🟡 Média | Boas práticas |
| 05a | [Reorganização estrutural: `core` + `routing`](05a-restructure-modules.md) | 🔴 Alta (arq.) | Arquitetura |
| 05b | [Redesenhar o `IRouter`](05b-redesign-irouter.md) | 🔴 Alta (arq.) | Arquitetura |
| 06 | [Corrigir ciclo de vida / referências de cena](06-scene-lifetime.md) | 🟡 Média | Arquitetura |
| 07 | [Alinhar padrão C++ entre cengine e consumidores](07-cpp-standard.md) | 🟡 Média | Ecossistema |
| 08 | [Presets portáveis + CI (Debug + sanitizers)](08-presets-and-ci.md) | 🟢 Baixa | Ecossistema |
| 09 | [Higiene de código (includes, código morto)](09-code-hygiene.md) | 🟢 Baixa | Boas práticas |
| 10 | [Eliminar magic string e estados *stringly-typed*](10-typed-states.md) | 🟢 Baixa | Boas práticas |
| 11 | [Documentação (Doxygen + uso no README)](11-documentation.md) | 🟢 Baixa | Documentação |
| 12 | [Tirar a contabilidade de ativação (`onEnter`) da `IScene`](12-scene-activation-bookkeeping.md) | 🟡 Média | Arquitetura |
| 13 | [Separar responsabilidades: Router × Repository](13-router-repository-responsibilities.md) | 🔴 Alta (arq.) | Arquitetura |
| 14 | [Tempo no loop: `update(dt)` separado de `render()`](14-time-in-the-loop.md) | 🔴 Alta (arq.) | Arquitetura |
| 15 | [Modo hospedado: dirigir o loop de fora (`frame(dt)`)](15-hosted-loop-mode.md) | 🟡 Média | Arquitetura |
| 16 | [Fim do quadro na janela: `IWindowManager::present()`](16-window-present-hook.md) | 🟡 Média | Arquitetura |
| 17 | [Colisão 2D: detecção opt-in (AABB + círculo)](17-collision2d-detection.md) ✅ 0.7.0 | 🟡 Média | Arquitetura |
| 18 | [Scene stack e overlays](18-scene-stack-overlays.md) | 🟢 Baixa/Média (estacionada — gate avaliado pelo breakout: 1 de 2 evidências) | Arquitetura |
| 19 | [FlowRouter: extrair a mecânica da fachada de navegação](19-flow-router-facade.md) ✅ 0.6.0 | 🟢 Baixa (carona) | Arquitetura |
| 20 | [Vocabulário de input como porta](20-input-vocabulary-port.md) ✅ 0.8.0 | 🟡 Média | Arquitetura |
| 21 | [`IWindowManager` obrigatório: remover a hipótese do `nullptr`](21-window-manager-mandatory.md) ✅ 0.6.0 | 🟡 Média (breaking, 0.6.0) | Arquitetura |
| 22 | [Colisão 2D: resolução (penetração/MTV)](22-collision2d-resolution.md) | 🟢 Baixa/Média (estacionada — gate avaliado pelo mario-bros: 1 de 2 evidências, e sinal de que resolver é política) | Arquitetura |
| 23 | [Câmera / viewport (mundo→tela + culling)](23-camera-viewport.md) | 🟢 Baixa (estacionada — 1 de 2 evidências: mario degrau 4; só a transformada+culling é candidata, o seguimento é feel) | Arquitetura |
| 24 | [Áudio como porta (`play(id)`), backend na plataforma](24-audio-port.md) | 🟢 Baixa (estacionada — 1 de 2 evidências: breakout; backend fica na plataforma) | Arquitetura |

## Candidatas estacionadas (esperando evidência)

Tasks que **não se implementam ainda**: registram uma candidata a crescer a
engine para o aprendizado não se perder entre os projetos, com o gate (ADR 0002)
que precisa disparar antes de começar. Ver [ADR 0002](../decisions/0002-criterio-de-promocao-anti-deposito.md).

- **18 (scene stack/overlays)** — 1/2: breakout ganhou pausa e resolveu com um
  `bool` local. Espera um 2º consumidor que o `bool` não resolva.
- **22 (resolução de colisão)** — 1/2: mario resolve eixo-separado; breakout
  resolve reflexão (`reflectOff`) — formas DIFERENTES, sinal de que resolver é
  política. Espera um 2º consumidor com o MESMO mecanismo.
- **23 (câmera/viewport)** — 1/2: o mario degrau 4 (commit `4a8f825`) trouxe a
  câmera. Só a TRANSFORMADA mundo→janela + culling é candidata; o SEGUIMENTO
  (âncora/limites) é feel e fica no jogo. Espera um 2º consumidor com o mesmo
  modelo de projeção.
- **24 (áudio como porta)** — 1/2: breakout trouxe som (XAudio2, no jogo). Subiria
  como PORTA (`play(id)`, ao lado do input), com o BACKEND ficando na plataforma;
  espera um 2º jogo com som (mario é o candidato).

## Consideradas e vetadas (política de jogo — NÃO reabrir sem argumento novo)

Padrões que se REPETEM entre os jogos e, ainda assim, **não sobem** — o veredito
já foi dado (ADR 0002: a duplicação é o custo aceito quando a forma se repete mas
o SIGNIFICADO é do jogo). Registrado aqui para ninguém reabrir "e isto, não é
candidato?" daqui a alguns jogos. O discriminador: o input subiu porque as 4
cópias eram o MESMO dado puro (o enum `Key`); os itens abaixo têm cópias
estruturalmente parecidas mas **semanticamente diferentes**. Semelhança de forma
≠ identidade de mecanismo.

- **Recordes** (`Record` + `RecordService` + `RecordRepository` + `FileRecordRepository`)
  — em 4 jogos (8puzzle, spaceinvaders, asteroids, breakout), a MAIOR duplicação
  do ecossistema. Vetado explicitamente (asteroids task 05, breakout task 07):
  o que é um recorde, quantos guardar, a ordem (movimentos↓ do 8puzzle vs score↑
  dos arcades) e onde persistir são decisões DO JOGO. Confirmado por diff: as
  portas DIFEREM entre jogos — não é o mesmo mecanismo.
- **`PlaySession`** (carregador do resultado da última partida entre cenas) — em
  3 jogos, ~10 linhas, quase idêntico (score+wave/level). É um struct de valor,
  não um mecanismo: promover seria "um `shared_ptr` que o composition root já
  segura". O conteúdo (o que é um "resultado") é política.
- **Wrap-around / toro** (arena que dá a volta) — ficou no asteroids; fora de
  escopo declarado na task 17. Formato do mundo é política; sem 2º consumidor (o
  mario não dá a volta). Promover daria à engine uma opinião sobre o formato do
  mundo.
- **Física de plataforma** (gravidade/terminal, impulso de pulo, integração,
  resolução sobre grade de tiles — o `World` do mario) — avaliada ao FECHAR o
  mario (2026-07-16), a pedido do dono. Vetada em três camadas: as CONSTANTES
  são feel (kJumpSpeed=340 existe para o pulo dar ~4 tiles "a cara do
  original", e foi retunada em playtest); a INTEGRAÇÃO é trivial e cada jogo
  integra diferente (inércia do asteroids, reflexão do breakout, degraus do
  spaceinvaders, eixo-separado do mario); a RESOLUÇÃO já é a task 22, e a
  evidência aponta contra (breakout e mario resolvem DIFERENTE). Não há duas
  cópias idênticas de nada — o discriminador do input não dispara. O único
  recorte mecanismo é a penetração/MTV, que É a task 22; "física na engine"
  congelaria até o formato de mundo (rampas tipo Sonic estouram o AABB).
  Reabre se um jogo futuro copiar o `World` do mario quase literalmente.

Sweep de 2026-07-15 (ao fechar o degrau 2 do mario): nenhuma candidata nova além
das estacionadas acima; nenhum math/Vec2/RNG/timer próprio duplicado (os jogos
usam `collision2d::Vec2`).

Sweep de 2026-07-16 (ao fechar o mario completo — degraus 1-5, com goombas,
recordes por pontos/tempo e bandeira): nenhuma candidata nova; física de
plataforma avaliada e vetada (acima); tasks 22/23/24 seguem estacionadas com os
gates inalterados (o pisão no goomba REFORÇA a leitura da 22: mais um contato
resolvido com regra própria do jogo).

## Legenda de status

Marque no topo de cada arquivo conforme avança:
`todo` → `in-progress` → `done`
