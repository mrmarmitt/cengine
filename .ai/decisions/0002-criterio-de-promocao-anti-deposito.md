# ADR 0002 — Critério de promoção de código para a engine (filtro anti-depósito)

- **Status:** Aceito
- **Data:** 2026-07-13
- **Contexto de decisão:** com dois jogos completos consumindo a cengine
  (8puzzle e spaceinvaders), o fechamento da PoC de sprites gerou uma leva
  de candidatos a promoção — e a pergunta: o que entra na engine e o que
  fica nos jogos?

## Contexto

O fim da PoC do Space Invaders (task 01 daquele repo) expôs código repetido
entre os dois jogos consumidores: recordes (Record/Service/Repository TSV
quase idênticos), a fachada de navegação (GameRouter/StateGameFlow), o
vocabulário de input (`Key`/`KeyEvent` copiado em três pontes de
plataforma), primitivas de colisão AABB, utilitários de tempo (timer de
cadência, cooldown) e RNG determinístico.

A tentação natural é promover tudo que se repete. O risco nomeado na
discussão: **transformar a cengine em um depósito de código** — uma engine
que cresce por acúmulo, não por responsabilidade. Duplicação entre jogos
não transfere responsabilidade para a engine automaticamente: engine boa
fornece **mecanismo**; **política e dados são do jogo** (coerente com o
ADR 0001: core mínimo, módulos opt-in, jogo dono das regras).

## Decisão

Um candidato só entra na cengine se passar nos **três** critérios:

1. **Mecanismo puro** — zero vocabulário de jogo no código promovido
   (nenhum `menu`, `gameOver`, `score`, `record`...). Se o nome de um
   conceito do jogo aparece, é política: fica no jogo.
2. **≥ 2 consumidores reais** — código já duplicado em pelo menos dois
   jogos/plataformas *hoje*, não "vai que alguém precisa". Extração, nunca
   especulação.
3. **Testável dentro da própria cengine** — a suíte da engine cobre o
   mecanismo sem precisar de um jogo ou de uma plataforma gráfica.

Tasks de promoção devem citar este ADR e demonstrar os três critérios no
contexto. Candidatos que falham em algum critério podem ser registrados
como task **estacionada com gate explícito** (critérios objetivos de
começar), nunca implementados "para já deixar pronto".

Corolário: duplicação real entre jogos que *falha* no critério 1 tem outro
destino — uma lib compartilhada de jogos (game-commons) ou a aceitação da
cópia; nunca a engine.

## Consequências

Aplicações do filtro na leva que originou esta decisão:

| Candidato | Veredito | Critério decisivo |
|---|---|---|
| Colisão AABB (task 17) | ✅ aprovado (a *detecção*; consequências ficam no jogo) | passa nos 3 |
| FlowRouter — mecânica da fachada (task 19) | ✅ aprovado (vocabulário fica nos jogos) | passa nos 3 |
| IWindowManager obrigatório (task 21) | ✅ aprovado (mecanismo do core) | passa nos 3 |
| Recordes (Record/Service/TSV) | ❌ fica nos jogos | falha no 1 (métrica/persistência são política do jogo) |
| Vocabulário de input (task 20) | ⏸ estacionada com gate | falha no 2 (cenas por plataforma: sem consumidor para a porta) |
| Pontes ForgeUi/ForgeSpriteUi | ❌ destino seria platform-common | falha no 1 e no 3 (conhecem GPU/The-Forge) |

Custo aceito: alguma duplicação convive nos jogos até o filtro liberar a
promoção — é preferível a abstração sem consumidor.
