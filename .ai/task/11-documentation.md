# 11 — Documentação (Doxygen nos headers + uso no README)

- **Status:** todo
- **Prioridade:** 🟢 Baixa
- **Categoria:** Documentação
- **Depende de:** todas as anteriores (documentar a API **já estabilizada**, para
  não escrever doc de código que vai mudar)

## Problema

A documentação atual cobre "como buildar/testar", mas quase nada do "o que a API
faz e como usá-la":

- Os headers públicos (`include/engine/*.hpp`) **não têm comentários de API**
  (nem Doxygen). O contrato do ciclo de vida das cenas
  (`onEnter → draw/input → onExit`) e a ordem esperada de chamadas não estão
  documentados em lugar nenhum.
- O `README.md` da raiz descreve a filosofia do projeto, mas **não mostra um
  exemplo de uso** — nem que o consumo é via `FetchContent` (o 8Puzzle faz isso).
- Não existe pasta `documentation/` (o 8Puzzle tem uma; inconsistência entre os
  dois projetos).

## Objetivo

Um consumidor (inclusive "você do futuro") consegue entender e usar a engine só
pela documentação, sem ler a implementação.

## Passos

1. **Doc-comments (Doxygen) nos headers públicos** de `include/engine/`:
   - Contrato e ordem do ciclo de vida em `IScene`
     (`onEnter`/`onEnterExecuted`/`isOnEnterExecuted`/`draw`/`input`/`onExit`).
   - Semântica de `IGameManager` (incluindo `shouldExit`, já renomeado na
     tarefa 02).
   - `IWindowManager`, `IState` (incluindo o porquê do `clone()` — prototype),
     e a `IRouter` já enxuta (tarefa 05).
2. **Seção "Uso" no README** com exemplo mínimo:
   - como declarar a dependência via `FetchContent` (espelhando o 8Puzzle);
   - montar `EngineManager` com um `WindowManager` e um `GameManager`;
   - registrar cenas via factory e navegar por estados.
3. (Opcional) criar `documentation/` com um diagrama das camadas
   (EngineManager → GameManager → Router → SceneRepository → Scene/State),
   alinhando com o padrão do 8Puzzle.
4. (Opcional) configurar geração Doxygen no CMake/CI.

## Critérios de aceite

- [ ] Todos os headers públicos com doc-comment explicando contrato.
- [ ] README com exemplo de consumo via FetchContent e montagem da engine.
- [ ] Nenhum código comentado remanescente (coordenar com tarefa 09).

## Riscos

Nenhum funcional. O cuidado é **cronológico**: documentar por último, quando a
API (especialmente o Router) já estiver estável, para não retrabalhar a doc.
