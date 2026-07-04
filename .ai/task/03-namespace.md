# 03 — Introduzir namespaces por camada

- **Status:** done ✅ (executada dentro da [05a](05a-restructure-modules.md), 2026-07-04)
- **Prioridade:** 🟡 Média
- **Categoria:** Arquitetura / distribuição
- **Depende de:** 01
- **Nota:** esta tarefa **deixou de ser executada isoladamente**. O namespace por
  camada (`cengine::core` / `cengine::routing`) nasce junto da reorganização em
  módulos — ver [05a](05a-restructure-modules.md) e [ADR 0001](../decisions/0001-modular-core-vs-modules.md).
  Este arquivo permanece como referência técnica do que precisa ser namespaceado.

## Problema

Todo o código está no **namespace global**: `EngineManager`, `IState`,
`GameManager`, `IRouter`, etc. Como a engine é **consumida via FetchContent**
(o projeto 8Puzzle faz exatamente isso), esses nomes poluem o escopo global do
consumidor e podem colidir com símbolos dele.

## Objetivo

Toda a API pública e implementação interna dentro de `namespace cengine { ... }`.

## Passos

1. Envolver os headers públicos (`include/engine/*.hpp`) em `namespace cengine`.
2. Envolver a implementação interna (`src/main/engine/**`) no mesmo namespace.
3. Atualizar testes, mocks e fakes para qualificar (`cengine::EngineManager`) ou
   usar `using namespace cengine;` no escopo de teste.
4. **Namespaces por camada, alinhados ao [ADR 0001](../decisions/0001-modular-core-vs-modules.md):**
   o core em `cengine::core` e o roteamento (quando extraído na tarefa 05) em
   `cengine::routing`. Isso prepara o terreno para a separação em módulos — sem
   overengineering, mas já com a fronteira desenhada nos namespaces.

## Critérios de aceite

- [ ] Nenhum símbolo público da engine no namespace global.
- [ ] Testes compilam e passam com os nomes qualificados.
- [ ] Um consumidor (ex.: 8Puzzle) precisaria escrever `cengine::EngineManager`.

## Riscos

Médio em *volume* de mudança (toca muitos arquivos), baixo em *complexidade*.
É uma boa tarefa para fazer isolada, num commit só, antes da refatoração 05.

## Nota de compatibilidade

Isso é uma **quebra de API** para consumidores existentes (o 8Puzzle terá de
qualificar os nomes). Coordenar com a tarefa 07 e com o versionamento
(provável bump para `0.1.0`).
