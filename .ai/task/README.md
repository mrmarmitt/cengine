# Plano de Melhoria — CEngine

Este diretório contém o plano de melhoria da engine, derivado de uma avaliação
técnica (arquitetura + boas práticas). Cada arquivo é uma tarefa independente,
numerada na **ordem de desenvolvimento recomendada**.

## Princípio da ordenação

A mudança de maior impacto é **tirar o roteamento do core** e reorganizar em
módulos (`cengine::core` + `cengine::routing`, ver [ADR 0001](../decisions/0001-modular-core-vs-modules.md)).
Como é grande e *breaking*, ela é feita cedo — porém **precedida de uma rede de
segurança barata** e **separando "mover" de "redesenhar"**.

**Ordem de execução recomendada** (não é estritamente a ordem numérica):

1. **Rede de segurança (01):** adicionar `override` — trivial e endurece o
   compilador **antes** da grande movimentação de código.
2. **Reorganização estrutural (05a):** mover para `core/` + `modules/routing/`,
   CMake multi-target, namespaces por camada. **Move puro, sem mudar
   comportamento** (absorve a antiga tarefa 03 de namespace).
3. **Redesenho do roteador (05b):** enxugar o `IRouter`, unificar Scene/Screen,
   desacoplar — já isolado dentro de `cengine::routing`.
4. **Correções de qualidade (02, 04, 06):** nomes invertidos, `const`/logging e
   ciclo de vida das cenas — agora no lugar final.
5. **Ecossistema/build (07–08):** padrão C++, presets, CI.
6. **Limpeza final e documentação (09–11).**

> Regra prática: manter a suíte de testes **verde a cada tarefa**. Nenhuma
> tarefa deve ser mergeada com testes quebrados.
>
> **Nota sobre a tarefa 03:** o namespace deixou de ser tarefa isolada — ele
> nasce junto da reorganização em módulos (05a), como `cengine::core` /
> `cengine::routing`. O arquivo 03 permanece como referência técnica.

## Decisões de arquitetura (ADRs)

O plano é guiado por decisões registradas em [`../decisions/`](../decisions/).
Ler antes de executar as tarefas de arquitetura:

- [ADR 0001 — Core mínimo (loop) + módulos opcionais](../decisions/0001-modular-core-vs-modules.md):
  o core é só o game loop + portas; roteamento e física são módulos opt-in no
  mesmo repo. Reformula a tarefa 05 (dividida em 05a/05b) e absorve a 03.

## Índice

| # | Tarefa | Prioridade | Categoria |
|---|--------|------------|-----------|
| 01 | [Adicionar `override` em todas as sobrescritas](01-add-override.md) | 🔴 Alta | Boas práticas |
| 02 | [Corrigir nomes invertidos (`shouldExit`, `isNextState…`)](02-fix-inverted-names.md) | 🔴 Alta | Boas práticas |
| 03 | [Introduzir namespaces](03-namespace.md) *(absorvida pela 05a)* | 🟡 Média | Arquitetura |
| 04 | [`const`-correctness + remover logging da lib](04-const-and-logging.md) | 🟡 Média | Boas práticas |
| 05a | [Reorganização estrutural: `core` + `routing`](05a-restructure-modules.md) | 🔴 Alta (arq.) | Arquitetura |
| 05b | [Redesenhar o `IRouter`](05b-redesign-irouter.md) | 🔴 Alta (arq.) | Arquitetura |
| 06 | [Corrigir ciclo de vida / referências de cena](06-scene-lifetime.md) | 🟡 Média | Arquitetura |
| 07 | [Alinhar padrão C++ entre cengine e consumidores](07-cpp-standard.md) | 🟡 Média | Ecossistema |
| 08 | [Presets portáveis + CI (Debug + sanitizers)](08-presets-and-ci.md) | 🟢 Baixa | Ecossistema |
| 09 | [Higiene de código (includes, código morto)](09-code-hygiene.md) | 🟢 Baixa | Boas práticas |
| 10 | [Eliminar magic string e estados *stringly-typed*](10-typed-states.md) | 🟢 Baixa | Boas práticas |
| 11 | [Documentação (Doxygen + uso no README)](11-documentation.md) | 🟢 Baixa | Documentação |

## Legenda de status

Marque no topo de cada arquivo conforme avança:
`todo` → `in-progress` → `done`
