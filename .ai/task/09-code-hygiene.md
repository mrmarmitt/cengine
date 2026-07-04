# 09 — Higiene de código (includes, código morto, forward-declares)

- **Status:** done ✅ (2026-07-04, branch `feature/cleanup-02-04-09-10`)

> Executado: removido `EngineManager::input()` (morto — o loop chama
> `gameManager->input()` direto) e o seu teste; removido `#include <iostream>`
> órfão de `EngineManager.cpp`; removidas as linhas comentadas de `IScene.hpp`.
> Os caminhos relativos `../../../` já haviam sido eliminados na 05a, e os
> forward-declares redundantes do `IRouter` na 05b. Remover `input()` é breaking
> na API pública do core (coordenar bump `0.1.0`).
- **Prioridade:** 🟢 Baixa
- **Categoria:** Boas práticas / limpeza
- **Depende de:** 05 (fazer a limpeza depois da refatoração grande, para não
  limpar código que vai ser reescrito)

## Problemas (agrupados — limpeza de baixo risco)

1. **Include com caminho relativo frágil.**
   `src/main/engine/service/repository/SceneRepository.hpp`:
   ```cpp
   #include "../../../../../include/engine/IScene.hpp"
   ```
   Cinco níveis de `../`. Trocar por `<engine/IScene.hpp>` (já está no include
   path público).

2. **Código morto / não utilizado.**
   - `EngineManager::input()` é `public` mas ninguém chama (o loop usa
     `m_gameManager->input()` direto). Remover ou justificar.
   - `#include <iostream>` órfão em `EngineManager.cpp` (nenhum uso).
     > Nota: o `iostream` de `GameManager.cpp` sai na tarefa 04.

3. **Linhas comentadas (dead comments).**
   - `include/engine/IScene.hpp` linhas 4, 10-11.
   - `GameManager.cpp` (comentário `// std::println("")`).
   Remover — o histórico do git guarda, não precisa ficar no código.

4. **Forward-declares redundantes.**
   `IRouter.hpp` faz `#include <engine/IState.hpp>` **e** `class IState;` na mesma
   unidade. Escolher um: se usa o tipo completo, mantém o include e tira o
   forward; se só usa referência/ponteiro, prefere forward e tira o include.
   Revisar também `RouterInMemory.hpp` e `ISceneRepository.hpp`.

## Objetivo

Base de código sem includes frágeis, sem símbolos mortos e sem ruído de
comentários obsoletos.

## Passos

1. Trocar o include relativo por `<engine/IScene.hpp>`.
2. Remover `EngineManager::input()` (após confirmar que nada externo usa).
3. Remover includes órfãos (`<iostream>`).
4. Apagar linhas comentadas obsoletas.
5. Racionalizar forward-declares vs includes.
6. (Opcional) rodar `include-what-you-use` ou `clang-tidy` para varredura ampla.

## Critérios de aceite

- [ ] Sem caminhos `../../../` em includes.
- [ ] Sem métodos públicos sem uso / includes órfãos.
- [ ] Testes verdes.

## Riscos

Baixo. Prestar atenção só para não remover algo que a tarefa 05 passou a usar.
