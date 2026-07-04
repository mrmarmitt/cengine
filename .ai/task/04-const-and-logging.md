# 04 — `const`-correctness + remover logging da biblioteca

- **Status:** todo
- **Prioridade:** 🟡 Média
- **Categoria:** Boas práticas
- **Depende de:** 01, 02
- **Bloqueia:** 05 (o Router será redesenhado; melhor entrar nele já com a
  semântica de `const` correta)

> Nota 2026-07-04: os mutadores antigos do `IRouter`
> (`setNextState`, `goToNextScreen`) foram substituidos na tarefa 05b por
> `requestState` e `commitStateChange`, ambos nao-`const`. Esta tarefa ainda
> precisa remover o logging direto de `GameManager::cleanup()` e revisar qualquer
> `const` remanescente fora da API do router.

## Problema

### 4.1 `const` que mente

`RouterInMemory` marca como `const` métodos que **mutam estado** (através do
`shared_ptr` membro, o que escapa da verificação do compilador):

```cpp
void RouterInMemory::setNextState(std::unique_ptr<IState> state) const {
    m_sceneRepository->persistNextState(std::move(state));   // muta!
}
void RouterInMemory::goToNextScreen() const {
    m_sceneRepository->unloadScene(...);                     // muta!
    m_sceneRepository->persisteCurrentState();               // muta!
}
```

Métodos que alteram o estado observável do objeto **não devem** ser `const`.
Isso engana o leitor e impede raciocínio sobre thread-safety.

### 4.2 Biblioteca escrevendo em `std::cout`

```cpp
void GameManager::cleanup() {
    std::cout << "TerminalGameManager: cleanup" << std::endl;  // lib não deve logar
}
```

Uma engine não deve escrever direto no stdout do consumidor. Além disso,
`std::endl` força flush desnecessário.

## Objetivo

- `const` reflete fielmente se o método muta ou não o objeto.
- A biblioteca não faz I/O de log direto; ou remove, ou injeta uma abstração.

## Passos

1. Remover `const` de `setNextState`, `goToNextScreen` (e de qualquer outro
   mutador marcado indevidamente) em `IRouter` e `RouterInMemory`.
   > Atenção: mexer na assinatura na interface exige atualizar mocks (`MockRouter`).
2. Remover o `std::cout` de `GameManager::cleanup()`. Opções:
   - **Mínimo:** simplesmente remover o log.
   - **Recomendado (leve):** definir uma interface `ILogger` injetável e trocar
     `std::cout` por `m_logger->info(...)`. Se for adiar, deixar TODO e apenas
     remover o `cout` por ora.
3. Remover `#include <iostream>` que ficar órfão (ver tarefa 09).

## Critérios de aceite

- [ ] Nenhum mutador marcado como `const`.
- [ ] Nenhum `std::cout`/`std::endl` no código de produção da lib.
- [ ] Testes verdes.

## Riscos

Baixo. A mudança de `const` na interface pode exigir ajuste nos mocks — fazer
junto para não quebrar a build de teste.
