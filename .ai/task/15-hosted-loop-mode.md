# 15 — Modo hospedado: dirigir o loop da engine de fora (`frame(dt)`)

- **Status:** todo (aguardando aprendizado da PoC The-Forge do 8Puzzle)
- **Prioridade:** 🟡 Média (sobe quando a fase 2 da PoC The-Forge começar)
- **Categoria:** Arquitetura / core
- **Depende de:** 14 (tempo no loop) ✅; PoC The-Forge fase 1 (8Puzzle,
  `.ai/task/01-theforge-poc.md` daquele repo) para validar o desenho com um
  host real.
- **Breaking:** não necessariamente — pode ser aditivo (candidato a 0.4.0).

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

Questões a fechar na execução:

1. **`IWindowManager` no modo hospedado**: o host (The-Forge) é dono da
   janela — `window.update()` dentro de `frame()` deve existir? Opções:
   (a) o consumidor passa um `IWindowManager` no-op; (b) `frame()` não toca a
   janela e `run()` chama `window.update()` antes de `frame()`. A opção (b)
   é mais honesta (janela é do dono do loop), mas muda a decomposição do
   quadro — decidir com o caso real na mão.
2. **`init()`/`cleanup()` no modo hospedado**: quem chama? Provável:
   `startHosted()`/`stopHosted()` explícitos, ou documentar que o host chama
   `cleanup()` — alinhar com o ciclo `Init/Exit` do The-Forge.
3. **Clamp do host**: o The-Forge já clampa o dt dele (0,05 s se > 0,15 s);
   o `m_maxFrameTime` da engine aplica por cima. Dois clamps são inofensivos,
   mas documentar.

## Passos (provisórios — refinar com o aprendizado da PoC)

1. Extrair o corpo do quadro de `run()` para `frame(Seconds)`; acumulador
   vira membro. `run()` reimplementado sobre `frame()` — os testes existentes
   de call-log **não podem mudar** (mesma sequência observável).
2. Testes novos do modo hospedado: `frame()` com dt controlado reproduz os
   casos da task 14 (0 updates, N updates com mesmo dt, clamp, resto
   acumulado entre quadros); `frame()` retorna false quando o estado é exit.
3. Decidir e implementar o tratamento de janela/cleanup no modo hospedado
   (questões 1-2 acima).
4. Doxygen + README (seção sobre hosts com inversão de controle).
5. Atualizar o adaptador da PoC The-Forge para consumir `frame()` no lugar
   das chamadas fase-a-fase (fecha o custo 1 e 2 do problema).

## Critérios de aceite

- [ ] `frame(dt)` público executando o quadro completo com o acumulador de
      fixed timestep interno (mesma garantia da task 14 nos dois modos).
- [ ] `run()`/`start()` reimplementados sobre `frame()` sem mudança observável
      (suíte de call-log intacta).
- [ ] Testes do modo hospedado cobrindo os casos de timestep e a condição de
      saída.
- [ ] Adaptador The-Forge (8Puzzle) consumindo `frame()` — nenhuma lógica de
      fase/acumulador duplicada fora da engine.
- [ ] Suíte verde (CI 3 jobs).

## Riscos

Baixo em código (extração de método com testes em cima); o risco real é
desenhar a API **antes** do caso de uso — por isso a task espera a fase 1 da
PoC The-Forge validar o formato do host. Não implementar por especulação.

## Relacionado

- Task 14 — criou o acumulador que esta task torna reutilizável.
- 8Puzzle `.ai/task/01-theforge-poc.md` — fase 1 (adaptador manual) e fase 2
  (modo biblioteca, que consome esta task).
- ADR 0001 — physics consome `update(dt)` em qualquer um dos modos.
