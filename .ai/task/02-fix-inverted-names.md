# 02 — Corrigir nomes invertidos e semântica enganosa

- **Status:** todo
- **Prioridade:** 🔴 Alta
- **Categoria:** Boas práticas / legibilidade
- **Depende de:** 01 (rede de segurança de `override` já ativa)
- **Bloqueia:** 05 (o redesenho do Router fica mais claro com nomes corretos)

> Nota 2026-07-04: a parte 2.2 (`isNextStateEqualsToCurrentScene` ->
> `hasPendingStateChange`) foi executada dentro da tarefa 05b. Esta tarefa ainda
> precisa resolver `shouldExist()` -> `shouldExit()`.

## Problema

Dois nomes dizem o **oposto** do que o código faz — a maior fonte de confusão
ao ler o loop principal.

### 2.1 `shouldExist()` → `shouldExit()`

```cpp
// EngineManager.cpp
m_isRunning = !m_gameManager->shouldExist();   // "não deve existir"? ilegível
// GameManager.cpp
bool GameManager::shouldExist() const { return ...getCode() == "exit"; }
```

O método retorna `true` quando o jogo deve **sair**. Nome correto: `shouldExit()`.

Arquivos afetados: `include/engine/IGameManager.hpp`, `GameManager.hpp/.cpp`,
`EngineManager.cpp`, e todos os mocks/fakes de teste
(`MockGameManager.hpp`, `FakeImplementations.hpp`).

### 2.2 `isNextStateEqualsToCurrentScene()` retorna `!=`

```cpp
bool SceneRepository::isNextStateEqualsToCurrentScene() const {
    return m_nextState->getCode() != m_currentState->getCode();  // != num "isEquals"
}
```

E `RouterInMemory::hasNextScreen()` delega a esse método. O comportamento final
está correto (há próxima tela quando next ≠ current), mas o nome mente.

**Decisão:** renomear para expressar a intenção real. Sugestão:
`bool hasPendingStateChange() const` (ou `isNextStateDifferentFromCurrent()`),
retornando `next != current` — assim nome e corpo concordam.

## Objetivo

Nome de cada método concorda com o valor booleano que ele retorna.

## Passos

1. Renomear `shouldExist` → `shouldExit` na interface, implementação, chamadas e
   todos os dublês de teste. Ajustar o `!` em `EngineManager::run()`.
2. Renomear `isNextStateEqualsToCurrentScene` → `hasPendingStateChange` (ou nome
   equivalente) em `ISceneRepository`, `SceneRepository`, e no chamador em
   `RouterInMemory::hasNextScreen()`.
3. Rodar os testes; ajustar nomes nos `.cpp` de teste que referenciam esses
   métodos.

## Critérios de aceite

- [ ] Nenhum método booleano com nome contrário ao retorno.
- [ ] `EngineManager::run()` fica legível (`m_isRunning = !...shouldExit()`).
- [ ] `ctest` 100% verde.

## Riscos

Baixo — é rename mecânico. Cuidar de atualizar mocks/fakes junto para não quebrar
a compilação dos testes.
