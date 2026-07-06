# 07 — Alinhar padrão C++ entre cengine e consumidores

- **Status:** done ✅ (2026-07-04, branch `feature/build-ecosystem-07-08`)

> Executado: `target_compile_features(... PUBLIC cxx_std_23)` agora em
> `cengine_core` (desde a 05a) **e** `cengine_routing` — o requisito de padrão
> propaga automaticamente para quem linkar os targets. Documentado no README
> (seção *Building*): CEngine exige C++23 e consumidores não precisam setar o
> padrão na mão. Alinhar o 8Puzzle (C++20) fica para quando ele consumir a nova
> tag.
- **Prioridade:** 🟡 Média
- **Categoria:** Ecossistema / compatibilidade
- **Depende de:** — (independente do código; coordenar com 03, que já é breaking)

## Problema

- **cengine** compila em **C++23** (`set(CMAKE_CXX_STANDARD 23)` +
  `CXX_STANDARD 23` no target).
- O consumidor **8Puzzle** usa **C++20** (`set(CMAKE_CXX_STANDARD 20)`), e puxa a
  engine via `FetchContent` na tag `0.0.1`.

Hoje os headers públicos **não usam** features de C++23, então "passa". Mas é uma
bomba-relógio: no dia em que um header público (`include/engine/*`) usar algo de
C++23, o build do 8Puzzle quebra — porque o header é compilado no padrão do
consumidor, não no da lib.

## Objetivo

Definir e documentar um contrato de padrão C++ claro entre a lib e seus
consumidores, evitando incompatibilidade silenciosa.

## Opções

1. **Rebaixar a lib para C++20** — máxima compatibilidade com o 8Puzzle atual.
2. **Subir o 8Puzzle para C++23** — se você quer usar features novas na engine.
3. **Manter C++23 na lib, mas garantir headers públicos ≤ C++20** e propagar o
   requisito via `target_compile_features(cengine_lib PUBLIC cxx_std_23)` para
   que o CMake force o padrão nos consumidores (em vez de só uma propriedade
   local do target).

> Recomendação: decidir o padrão-alvo do ecossistema e usar
> `target_compile_features(... PUBLIC ...)` para que o requisito **propague**
> automaticamente a quem linkar `cengine_lib`, em vez de depender de cada
> consumidor configurar o padrão certo na mão.

## Passos

1. Decidir o padrão-alvo (recomendo alinhar ambos e usar `PUBLIC` compile
   features).
2. Ajustar `CMakeLists.txt` do cengine para propagar o requisito.
3. Validar buildando o 8Puzzle contra a nova tag da engine.
4. Documentar no README o padrão mínimo exigido.

## Critérios de aceite

- [ ] Padrão C++ propagado via `target_compile_features(... PUBLIC ...)`.
- [ ] 8Puzzle builda contra a engine sem configurar padrão manualmente.
- [ ] README documenta o requisito.

## Riscos

Baixo. Envolve decisão de produto (quais features usar) mais que técnica.
