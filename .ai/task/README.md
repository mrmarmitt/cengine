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
| 22 | [Colisão 2D: resolução (recorte a decidir)](22-collision2d-resolution.md) | 🟢 Baixa/Média (2/2 para eixo-separado: mario + zelda; 0 consumidores de penetração/MTV — comparar antes de promover) | Arquitetura |
| 23 | [Câmera / viewport (mundo→tela + culling)](23-camera-viewport.md) | ✅ done (0.10.0 — `cengine::camera2d`: transformada+culling; seguimento ficou nos jogos; zelda valida, mario pinado 0.9.0) | Arquitetura |
| 24 | [Áudio como porta (`play(id)`), backend na plataforma](24-audio-port.md) ✅ 0.9.0 | 🟡 Média (gate disparou com 2/2: breakout + mario@0fab493; mario valida a 0.9.0) | Arquitetura |
| 25 | [Clip de animação de sprite (frames sobre tempo)](25-sprite-animation-clip.md) | ✅ done (0.10.0 — `cengine::anim`: máquina clip+frame+acumulador; seleção/vocabulário ficam nos jogos; zelda valida, mario pinado 0.9.0; spaceinvaders segue sem linkar — opt-in) | Arquitetura |

## Candidatas e estado dos gates

Tasks que registram candidatas a crescer a engine para o aprendizado não se
perder entre os projetos. Um gate disparado autoriza desenhar a extração; não
autoriza promover política nem implementar uma API diferente da evidência real.
Ver [ADR 0002](../decisions/0002-criterio-de-promocao-anti-deposito.md).

- **18 (scene stack/overlays)** — 1/2: breakout ganhou pausa e resolveu com um
  `bool` local. Espera um 2º consumidor que o `bool` não resolva.
- **22 (resolução de colisão)** — **2/2 para o padrão eixo-separado**: mario e
  zelda movem/resolvem X e depois Y. Isso dispara a comparação, mas não a API
  de penetração/MTV originalmente imaginada, que segue com 0 consumidores. A
  task permanece estacionada até identificar um núcleo puro que ambos usariam;
  reflexão do breakout, `grounded`, one-way e dano continuam política.
- **23 (câmera/viewport)** — **GATE DISPARADO (2/2)**: mario e zelda usam a
  mesma transformada mundo→janela e o mesmo culling; o Zelda adiciona rolagem
  vertical sem mudar o mecanismo. A candidata está pronta para desenho da
  extração. O SEGUIMENTO (âncora/limites/eixos) é feel e fica nos jogos.
- **24 (áudio como porta)** — **PROMOVIDA (0.9.0, 2026-07-16)**: o gate
  disparou com 2/2 (breakout + mario `0fab493`, cópias quase idênticas — o
  discriminador do input) e a porta subiu: `cengine::audio::Player` com
  `play(id)` e mais nada, backend segue nas plataformas. É a prova de que uma
  estacionada não é uma recusa: é uma espera com critério.
- **25 (clip de animação de sprite)** — 1/2: o mario trouxe o `PlayerAnimator`
  (ciclo de frames dirigido pelo TEMPO: clip = frames + fps + loop). O
  spaceinvaders anima SEM relógio (a pose deriva do passo da marcha,
  `animFrame()`) — forma diferente, o mesmo sinal de divergência da 22. Espera
  um 2º jogo com ciclo de frames dirigido pelo tempo; a ESCOLHA do clip
  (Idle/Walk/Jump, facing) é política e fica no jogo.

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
  evidência inicialmente apontava contra (breakout e mario resolvem
  DIFERENTE). O Zelda depois forneceu a segunda evidência do recorte
  eixo-separado, registrada na task 22, mas não tornou gravidade, pulo,
  `grounded`, tiles one-way ou dano mecanismos genéricos. "Física na engine"
  continuaria congelando política e formato de mundo; apenas o núcleo comum de
  resolução pode ser reavaliado.
- **`Events` por quadro** (o struct de contadores que o World zera e preenche a
  cada `update` — `brk::Events` no breakout, `mario::Events` no mario) — o
  PADRÃO se repete (fatos, não sons; a cena decide o significado) e deve
  continuar se repetindo nos próximos jogos, mas os CAMPOS são o vocabulário de
  cada jogo (tijolos/vidas/fase vs pulo/moeda/pisão/bandeira). Mesmo caso do
  `PlaySession`: struct de valor, não mecanismo. O padrão é disciplina de
  projeto, documentada nas tasks dos jogos; não vira tipo da engine.
- **Formatação de tempo** — o common já tem `formatMillis` (hh:mm:ss.mmm, do
  8puzzle) e o mario criou `ui::formatTime` (M:SS.cc) SEM reusar: os formatos
  divergem de propósito (cronômetro de puzzle vs HUD de arcade). Formato de
  exibição é feel; duas cópias com saídas diferentes não são o mesmo mecanismo.

Sweep de 2026-07-15 (ao fechar o degrau 2 do mario): nenhuma candidata nova além
das estacionadas acima; nenhum math/Vec2/RNG/timer próprio duplicado (os jogos
usam `collision2d::Vec2`).

Sweep de 2026-07-16 (ao fechar o mario completo — degraus 1-5, com goombas,
recordes por pontos/tempo e bandeira): nenhuma candidata nova; física de
plataforma avaliada e vetada (acima); tasks 22/23 seguem estacionadas com os
gates inalterados (o pisão no goomba REFORÇA a leitura da 22: mais um contato
resolvido com regra própria do jogo). A 24 promoveu logo em seguida (0.9.0).

Sweep de 2026-07-17 (revisão pós-0.9.0, antes do 6º jogo): uma candidata nova
na ENGINE — o clip de animação de sprite (task 25, estacionada 1/2, mario
`PlayerAnimator`). Duas candidatas novas no nível da PLATAFORMA, registradas no
backlog do platform-theforge-common (lá é a casa delas, não aqui): o backend
XAudio2 da porta de áudio (2 cópias idênticas: breakout + mario) e o escritor
de DDS dos geradores de atlas (3 cópias idênticas: spaceinvaders + breakout +
mario). Vetados novos abaixo: `Events` por quadro e formatação de tempo.

Sweep de 2026-07-18 (Zelda tasks 02–03 concluídas): a task 23 atingiu 2/2 e
está liberada para desenhar a extração de transformada+culling. A task 22
também chegou a 2/2 para resolução eixo-separada, mas a evidência não usa o MTV
proposto originalmente; antes de implementar, a candidata precisa ser
recortada pela comparação Mario×Zelda. Seguimento de câmera e regras de colisão
continuam nos jogos.

## Legenda de status

Marque no topo de cada arquivo conforme avança:
`todo` → `in-progress` → `done`
