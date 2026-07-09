# 15 — Modo hospedado: dirigir o loop da engine de fora (`frame(dt)`)

- **Status:** done (2026-07-09 — release 0.4.0; adaptador `8PuzzleForge`
  migrado para `frame(dt)` e validado em runtime)
- **Prioridade:** 🟡 Média
- **Categoria:** Arquitetura / core
- **Depende de:** 14 (tempo no loop) ✅; PoC The-Forge fase 1 ✅ (8Puzzle,
  `.ai/task/01-theforge-poc.md` daquele repo — validou o desenho com um host
  real).
- **Breaking:** não — aditivo (candidato a 0.4.0).

## Problema

O `EngineManager::start()` assume que a engine é dona do loop: bloqueia e gira
até `shouldExit()`. Frameworks de plataforma com **inversão de controle** —
The-Forge (`WindowsMain` chama `IApp::Update(dt)`/`Draw()` como callbacks),
e no futuro qualquer host tipo editor/browser — também querem ser donos do
loop. Dois donos não coexistem.

Hoje o contorno (fase 1 da PoC The-Forge) é o adaptador **bypassar o
`EngineManager`** e chamar as fases do `IGameManager` diretamente:

```cpp
// dentro do IApp do The-Forge
void Update(float dt) override {
    m_gameManager->onEnter();
    m_gameManager->input();
    m_gameManager->update(core::Seconds{dt}); // dt variável do host!
}
void Draw() override {
    m_gameManager->render();
    m_gameManager->onExit();
    if (m_gameManager->shouldExit()) requestShutdown();
}
```

Funciona, mas com dois custos:

1. **O acumulador de fixed timestep fica de fora.** O host entrega dt
   variável; a garantia da task 14 ("0..N updates por quadro, sempre com o
   MESMO dt") morre, a menos que cada adaptador reimplemente o acumulador —
   duplicando exatamente a lógica que `run()` já tem.
2. **A ordem das fases vira responsabilidade do adaptador.** Cada plataforma
   hospedada pode errar a sequência (ex.: esquecer o `onExit` no fim do
   quadro) sem nenhum teste da engine protegendo.

## Objetivo

A engine oferece os dois modos com a MESMA lógica de quadro:

- **Modo próprio** (hoje): `start()` bloqueia e dirige.
- **Modo hospedado** (novo): o host chama `frame(dt)` uma vez por quadro; a
  engine executa dentro dele a sequência completa (fases + acumulador de
  fixed timestep) e devolve se o jogo pediu para sair.

## Esboço de desenho (validar na execução)

```cpp
class EngineManager {
public:
    // ... start()/cleanup() como hoje ...

    /// Modo hospedado: executa UM quadro completo (window.update -> onEnter ->
    /// input -> update(fixedDt) 0..N vezes -> render -> onExit) consumindo
    /// @p frameTime no acumulador interno. @return false quando o jogo pediu
    /// para sair (o host deve parar de chamar e invocar shutdown proprio).
    [[nodiscard]] bool frame(Seconds frameTime);

private:
    void run() {
        // start()/run() viram consumidores do mesmo frame():
        TimePoint previous = m_clockNow();
        bool running = true;
        while (running) {
            const TimePoint now = m_clockNow();
            running = frame(now - previous);
            previous = now;
        }
        cleanup();
    }
    Seconds m_accumulator{0}; // sobe de variável local para membro
};
```

## Decisões fechadas com a fase 1 da PoC (2026-07-09)

A fase 1 (8Puzzle rodando no The-Forge via adaptador manual) respondeu as
questões abertas e revelou uma nova. Decisões:

1. **API única `frame(dt)` — sem split simulate/present.** Hosts como o
   The-Forge dividem o quadro em dois callbacks (`Update(dt)`/`Draw()`), e o
   `render()` das cenas precisa do command buffer que só existe no `Draw()`.
   Avaliamos dividir a API (`simulate(dt)` no Update, `present()` no Draw),
   mas isso devolveria ao adaptador a responsabilidade pela ordem das fases —
   exatamente o custo 2 que esta task elimina. Fica **um método só**,
   consumido dentro do `Draw()` do host; o `Update(dt)` do adaptador vira
   apenas "guardar o dt + capturar input" (a fase 1 provou que a ponte por
   fila/snapshot — `ForgeUi` — torna isso trivial). Receita documentada:

   ```cpp
   void Update(float dt) { forgeui::beginInput(...); gDt = dt; }
   void Draw() {
       /* boilerplate de GPU + beginDraw(cmd, ...) */
       if (!gEngine->frame(Seconds{gDt})) requestShutdown();
       /* submit/present */
   }
   ```

2. **`frame()` não toca janela.** No modo hospedado NÃO existe
   `IWindowManager` — janela/pump/resize são do host (o adaptador da fase 1
   nem tem window manager). O `EngineManager` aceita construção com
   `nullptr`; o `run()` (modo próprio) ganha guarda no
   `m_windowManager->update()` — hoje ele desreferencia sem checar.

3. **Ciclo de vida alinhado ao host**: no modo hospedado `start()` nunca é
   chamado; o host chama `frame()` até retornar false e invoca `cleanup()`
   no teardown dele (`Exit()` do IApp). Sem `startHosted()`/`stopHosted()`.

4. **Clamp duplo é inofensivo** (confirmado): o The-Forge clampa o dt dele e
   o `m_maxFrameTime` aplica por cima — a garantia da engine não depende do
   host. Documentar. Bônus: no modo hospedado o `ClockNowFn` não é usado
   (o tempo vem do host), simplificando os testes.

5. **Semântica**: `render()` roda todo quadro, mesmo com 0 updates (quadro
   rápido que não encheu o acumulador) — comportamento atual do `run()`,
   base para interpolação futura. Retorno `[[nodiscard]] bool`
   (false = jogo pediu saída; o shutdown é decisão do host).

## Passos

1. Extrair o corpo do quadro de `run()` para `frame(Seconds)`; acumulador
   vira membro. `run()` reimplementado sobre `frame()` — os testes existentes
   de call-log **não podem mudar** (mesma sequência observável).
2. Testes novos do modo hospedado: `frame()` com dt controlado reproduz os
   casos da task 14 (0 updates, N updates com mesmo dt, clamp, resto
   acumulado entre quadros); `frame()` retorna false quando o estado é exit.
3. Guarda de `m_windowManager` no `run()` (construção com `nullptr` passa a
   ser suportada e documentada — modo hospedado não tem janela).
4. Doxygen + README (seção sobre hosts com inversão de controle).
5. Atualizar o adaptador da PoC The-Forge para consumir `frame()` no lugar
   das chamadas fase-a-fase (fecha o custo 1 e 2 do problema).

## Critérios de aceite

- [x] `frame(dt)` público executando o quadro completo com o acumulador de
      fixed timestep interno (mesma garantia da task 14 nos dois modos).
- [x] `run()`/`start()` reimplementados sobre `frame()` sem mudança observável
      (suíte de call-log intacta — 41/41 verdes, incluindo os antigos).
- [x] Testes do modo hospedado (6 novos): fases em ordem, 0 updates com
      render, acumulador persistindo ENTRE frames, clamp, retorno false na
      saída, cleanup sem janela.
- [x] Adaptador The-Forge (8Puzzle) consumindo `frame()` — nenhuma lógica de
      fase/acumulador duplicada fora da engine (8puzzle commit 7f00334: o
      `Update()` só captura input/dt, o `Draw()` chama `frame(dt)`; o jogo
      ganhou fixed timestep de verdade no The-Forge, validado em runtime).
- [x] Suíte verde (CI nos PRs #16/#17; 41/41 local).

## Riscos

Baixo: extração de método com testes em cima, e a API foi desenhada DEPOIS do
caso de uso real (fase 1 da PoC validou o formato do host — ver seção de
decisões).

## Relacionado

- Task 14 — criou o acumulador que esta task torna reutilizável.
- 8Puzzle `.ai/task/01-theforge-poc.md` — fase 1 (adaptador manual) e fase 2
  (modo biblioteca, que consome esta task).
- ADR 0001 — physics consome `update(dt)` em qualquer um dos modos.
