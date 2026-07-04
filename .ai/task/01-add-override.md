# 01 — Adicionar `override` em todas as sobrescritas

- **Status:** todo
- **Prioridade:** 🔴 Alta
- **Categoria:** Boas práticas / correção
- **Depende de:** —
- **Bloqueia:** É a rede de segurança que protege as tarefas 02, 05 e 06.

## Problema

Várias classes concretas sobrescrevem métodos virtuais **sem** a palavra-chave
`override`. Sem ela, um erro de assinatura (parâmetro, `const`, tipo de retorno)
não vira erro de compilação — vira silenciosamente um método novo, e a chamada
virtual continua indo para a versão abstrata/errada.

Ocorrências conhecidas:

- `src/main/engine/service/impl/RouterInMemory.hpp` — **nenhum** dos ~11 métodos
  tem `override` (ex.: `void setNextState(std::unique_ptr<IState> state) const;`).
- `src/main/engine/GameManager.hpp` — `void cleanup();` é override de
  `IGameManager::cleanup()`, mas não está marcado.

Faça uma varredura por todas as classes que herdam de uma interface
(`RouterInMemory`, `SceneRepository`, `GameManager`, `EngineManager` não herda)
e confirme que **toda** sobrescrita tem `override`.

## Objetivo

Toda função que sobrescreve um método virtual deve estar marcada com `override`
(e destrutores virtuais com `override` quando aplicável).

## Passos

1. Em `RouterInMemory.hpp`, adicionar `override` a todos os métodos herdados de
   `IRouter`.
2. Em `GameManager.hpp`, marcar `cleanup()` como `override`.
3. Revisar `SceneRepository.hpp` (já usa `override` na maioria — confirmar 100%).
4. Compilar. **Se algum `override` gerar erro de compilação, ótimo:** achamos um
   bug de assinatura escondido — corrigir a assinatura, não remover o `override`.

## Critérios de aceite

- [ ] `grep` por métodos herdados sem `override` retorna vazio.
- [ ] Projeto compila em Debug e Release.
- [ ] `ctest` continua 100% verde.

## Riscos

Baixíssimo. No pior caso, expõe um bug pré-existente de assinatura — o que é o
resultado desejado.
