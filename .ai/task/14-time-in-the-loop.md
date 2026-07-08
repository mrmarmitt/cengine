# 14 — Tempo no loop: `update(dt)` separado de `render()`

- **Status:** done ✅ (opção B, 2026-07-08, branch `feature/14-time-in-the-loop`)
- **Prioridade:** 🔴 Alta (arquitetural — a maior lacuna do core)
- **Categoria:** Arquitetura / core
- **Depende de:** 12 e 13 (ciclo 0.2.0 fechado; portas estáveis)
- **Desbloqueia:** módulo `cengine::physics` (ADR 0001), jogos em tempo real
- **Decisão de referência:** [ADR 0001](../decisions/0001-modular-core-vs-modules.md)
  — nota de implementação sobre *fixed timestep*
- **Breaking:** sim — toca as portas do core (`IGameManager`, `IScene`).
  Candidato a âncora do ciclo **0.3.0**.

## Problema

O loop (`core/src/EngineManager.cpp`, `run()`) não tem conceito de tempo:

```cpp
while (m_isRunning) {
    m_windowManager->update();
    m_gameManager->onEnter();
    m_gameManager->input();
    m_gameManager->render();
    m_gameManager->onExit();
    m_isRunning = !m_gameManager->shouldExit();
}
```

1. **Gira em velocidade máxima.** Sem `dt`, sem sleep, sem vsync: um frame
   dura o que a CPU deixar. O 8Puzzle só funciona porque o `input()` das cenas
   **bloqueia** no teclado (`_getch`) — é o input que "paceia" o loop, por
   acidente. Qualquer cena que não bloqueie vira busy-loop a 100% de CPU.
2. **Não existe fase de simulação.** As cenas só têm `input()` e `draw()`;
   qualquer lógica dependente de tempo (animação, física, timers, IA) não tem
   onde viver nem como escalar pela duração do frame.
3. **Física fica impossível.** Integração numérica estável exige *fixed
   timestep* (passos de simulação de duração constante, desacoplados da taxa
   de render). O ADR 0001 adiou isso de propósito; com o roteamento resolvido
   (05→13), é a pendência estrutural que sobrou no core.

## Objetivo

O loop canônico com tempo explícito:

```
processInput → update(dt) → render
```

com `dt` medido pelo relógio monotônico, entregue às cenas via
`IGameManager::update(dt)` → `IScene::update(dt)`, e com uma estratégia de
timestep definida (e testável) no `EngineManager`.

## Opções (decidir na tarefa)

### A) Variable timestep (mínimo)

Medir `dt` real entre frames e repassar:

```cpp
auto previous = clock::now();
while (m_isRunning) {
    const auto now = clock::now();
    const Seconds dt = now - previous;   // double, segundos
    previous = now;

    m_windowManager->update();
    m_gameManager->onEnter();
    m_gameManager->input();
    m_gameManager->update(dt);           // fase nova
    m_gameManager->render();
    m_gameManager->onExit();
    ...
}
```

- ✅ Mudança pequena; suficiente para animação/timers do 8Puzzle.
- ❌ Não serve para física (dt grande → tunneling/instabilidade); adia o
  problema que a task existe para resolver.

### B) Fixed timestep com acumulador (recomendada)

O padrão "Fix Your Timestep" (Gaffer on Games): render na taxa que der,
simulação em passos fixos:

```cpp
constexpr Seconds kFixedDt{1.0 / 60.0};
Seconds accumulator{0};
auto previous = clock::now();

while (m_isRunning) {
    const auto now = clock::now();
    Seconds frameTime = now - previous;
    previous = now;
    if (frameTime > kMaxFrameTime) frameTime = kMaxFrameTime; // anti-espiral

    m_windowManager->update();
    m_gameManager->onEnter();
    m_gameManager->input();

    for (accumulator += frameTime; accumulator >= kFixedDt; accumulator -= kFixedDt) {
        m_gameManager->update(kFixedDt);   // 0..N passos por frame
    }

    m_gameManager->render();
    m_gameManager->onExit();
    ...
}
```

- ✅ Determinístico e estável — é o contrato que física exige; `update` pode
  rodar 0 ou N vezes por frame e o jogo não percebe.
- ✅ O clamp de `frameTime` (ex.: 250ms) evita a espiral da morte (frame lento
  → mais passos → frame mais lento).
- ❌ Interpolação de render (`render(alpha)`) fica de fora nesta task (ver
  não-objetivos); sem ela pode haver micro-stutter visual — irrelevante para
  jogos de terminal.
- ⚠️ `kFixedDt` deve ser configurável na construção do `EngineManager`
  (default 1/60), não hardcoded.

### C) Fixed timestep + interpolação de render

B + `render(alpha)` com estado anterior/atual interpolado.

- ❌ Exige que as cenas guardem dois estados de simulação — complexidade que
  nenhum consumidor atual paga. Registrar como evolução, não fazer agora.

> Recomendação: **B**. A diferença de custo entre A e B é um acumulador e um
> `for`; a diferença de valor é "física possível vs. impossível". C só quando
> houver um consumidor gráfico de verdade.

## Decisões de design a fechar na execução

1. **Tipo do tempo:** `std::chrono` de ponta a ponta.
   `std::chrono::steady_clock` (monotônico — nunca `system_clock`) e
   `using Seconds = std::chrono::duration<double>;` na API pública
   (`update(Seconds dt)`), evitando double cru e unidade ambígua.
2. **Onde o tick entra nas portas:**
   - `IGameManager::update(Seconds dt)` — novo puro.
   - `IScene::update(Seconds dt)` — novo; a cena ativa recebe o tick do
     `routing::GameManager` (mesma delegação de `render`/`input`).
   - `IWindowManager` **não** ganha tempo (eventos/apresentação seguem 1x por
     frame).
3. **Ordem no frame:** `input → update(dt) → render` (input antes para o
   update do mesmo frame reagir; hoje input também vem antes de render).
4. **Sem sleep/cap de FPS nesta task:** com o input bloqueante do 8Puzzle o
   busy-loop não se manifesta; throttling (sleep até o próximo frame, vsync da
   plataforma) é decisão da janela/plataforma e fica documentado como fora do
   escopo — anotar como possível task futura junto de C.
5. **`onEnter` não recebe dt** — ativação não é simulação. O primeiro `update`
   após a ativação recebe o `kFixedDt` normal.

## Passos

1. `IScene::update(Seconds dt)` e `IGameManager::update(Seconds dt)` (novos
   métodos puros; Doxygen com o contrato "0..N chamadas por frame, dt fixo").
2. `routing::GameManager::update(dt)` delegando à cena atual (mesmo padrão de
   `render()`).
3. `EngineManager::run()` com relógio monotônico + acumulador + clamp
   anti-espiral; `kFixedDt` configurável no construtor (default 1/60,
   `kMaxFrameTime` default 250ms).
4. Testes:
   - **Unidade (GameManager):** `update` delega à cena atual.
   - **Loop (EngineManager, via call-log):** a ordem do frame vira
     `window.update → onEnter → input → update* → render → onExit`;
     com frame simulado longo, `update` roda N vezes com o MESMO dt; com frame
     curto, roda 0 vezes e o render acontece mesmo assim.
   - Para tornar o loop testável sem dormir de verdade, injetar a fonte de
     tempo: um `std::function<TimePoint()>` (ou porta `IClock` mínima) com
     default `steady_clock::now` — decidir a forma mais barata na execução.
   - **Mocks:** `MockScene`/`MockGameManager` ganham `update`.
5. Atualizar README (loop com update), ADR 0001 (nota de implementação
   resolvida → referenciar esta task) e os Doxygen das portas.
6. **8Puzzle (fora do escopo, registrar pendência):** cenas ganham
   `update(dt) {}` vazio — compilam sem lógica nova. Oportunidade imediata:
   cronômetro de partida no `GameScene` via dt em vez de relógio ad-hoc.

## Critérios de aceite

- [x] `EngineManager::run()` mede tempo com relógio monotônico e executa
      `update(dt)` em passos fixos (acumulador + clamp anti-espiral).
- [x] `dt` tipado com `std::chrono` na API pública (nada de `double` cru).
- [x] Ordem do frame coberta por teste de call-log, incluindo os casos
      "0 updates" e "N updates com dt idêntico".
- [x] Fonte de tempo injetável (testes não dependem de sleep/tempo real).
- [x] `kFixedDt` configurável na construção; defaults documentados.
- [x] Portas atualizadas com Doxygen do novo contrato; ADR 0001 anotado.
- [x] Suíte verde (CI 3 jobs, incluindo ASan).

## Resultado da execução

Executada a **opção B** (fixed timestep com acumulador), com as decisões de
design da seção anterior:

- **`core/Time.hpp`** novo: `using Seconds = std::chrono::duration<double>;`.
- **`IScene::update(Seconds)`** e **`IGameManager::update(Seconds)`** novos
  (Doxygen com o contrato "0..N chamadas por quadro, mesmo dt");
  `routing::GameManager::update` delega à cena atual.
- **`EngineManager`**: construtor ganhou `fixedDt` (default `kDefaultFixedDt`
  = 1/60 s), `maxFrameTime` (default 250 ms) e `clockNow` injetável (default
  `steady_clock::now` — `std::function<TimePoint()>`, a forma mais barata).
  Configuração não-positiva lança `std::invalid_argument` (um `fixedDt <= 0`
  travaria o loop interno). `run()` implementa acumulador + teto anti-espiral.
- **Testes (35/35)** — 6 novos:
  - unidade: quadro longo → N updates com o MESMO dt; quadro curto → 0 updates
    (render acontece mesmo assim); clamp do frameTime; construção inválida
    lança; `GameManager::update` repassa o dt à cena.
  - integração (call-log): o acumulador carrega o resto entre quadros
    (25 ms/quadro com passo de 10 ms → 2 updates no quadro 1, 3 no quadro 2),
    na ordem `input → update* → render`.
  - Os fixtures existentes injetam relógio congelado (frameTime = 0) — os
    call-logs antigos continuam válidos e determinísticos.
- README (seção "The frame and time") e ADR 0001 (nota resolvida) atualizados.

## Pendências fora do escopo (atualizado na execução)

- **8Puzzle** (ao consumir a 0.3.0): cenas ganham `update(Seconds) {}`;
  oportunidade: cronômetro de partida no `GameScene` via dt.
- Throttling/cap de FPS (sleep/vsync) — decisão de plataforma, task futura.
- Interpolação de render (opção C) — quando houver consumidor gráfico.

## Riscos

- **Breaking nas duas portas centrais** — todo consumidor implementa
  `update`. Mitigação: mudança mecânica (métodos vazios), pendência do 8Puzzle
  documentada, agrupar no bump 0.3.0.
- **Testes de tempo frágeis** — mitigado pela injeção da fonte de tempo (o
  teste avança o relógio manualmente; nenhum sleep).
- **Escopo escorregando para física/interpolação** — física é outro módulo
  (ADR 0001) e interpolação é a opção C; esta task entrega só o contrato de
  tempo do core.

## Relacionado

- [ADR 0001](../decisions/0001-modular-core-vs-modules.md) — nota de
  implementação sobre fixed timestep (este arquivo a resolve).
- Task 13 — fechou o desenho do routing; esta task fecha o desenho do loop.
- Futuro: `cengine::physics` (consome `update(dt)` fixo); interpolação de
  render (opção C); throttling/cap de FPS na plataforma.
