# 10 — Eliminar magic string e estados *stringly-typed*

- **Status:** done ✅ (opção 1, 2026-07-04, branch `feature/cleanup-02-04-09-10`)

> Executada a **opção 1** (mínima): a string mágica `"exit"` virou a constante
> nomeada `cengine::routing::kExitStateCode` (`std::string_view`) em
> `StateCodes.hpp`, usada em `GameManager::shouldExit()` e nos testes. As opções
> 2/3 (sinal de saída tipado / `enum` em vez de `std::string` no `IState`) ficam
> como evolução futura, se houver dor real com strings.
- **Prioridade:** 🟢 Baixa
- **Categoria:** Boas práticas / design
- **Depende de:** 02 (nomes já corrigidos), 05 (desenho do Router estável)

## Problema

### 10.1 Magic string `"exit"`

```cpp
bool GameManager::shouldExit() const {   // (após tarefa 02)
    return m_routerService->getCurrentStateGameCode() == "exit";
}
```

O código de saída `"exit"` é uma string mágica embutida na lógica de controle do
loop. Se o consumidor escrever `"Exit"` ou `"quit"`, o jogo nunca sai, sem erro
de compilação.

### 10.2 Estados identificados por `std::string`

`IState` expõe `getCode()`/`getName()` como `std::string`. Comparar estados por
string é frágil (typos silenciosos), mais lento e sem checagem em tempo de
compilação.

## Objetivo

Tornar a identificação de estados robusta a erros de digitação e explícita.

## Opções (decidir na tarefa)

1. **Mínimo:** extrair `"exit"` para uma constante nomeada
   (`constexpr std::string_view kExitStateCode = "exit";`) em um header central.
   Resolve o magic string sem redesenhar `IState`.
2. **Intermediário:** o consumidor sinaliza saída por um mecanismo tipado — ex.:
   o Router/GameManager expõe `requestExit()` e o loop consulta um `bool`, em vez
   de comparar código de estado por string.
3. **Amplo:** repensar `IState` para identificação por `enum class` ou tipo forte
   em vez de `std::string` livre. Maior impacto na API — avaliar se compensa para
   um projeto de estudo.

> Recomendação: começar pela opção 1 (barata) e avaliar a 2 como evolução natural
> do desenho do Router (tarefa 05). A opção 3 só se houver dor real com strings.

## Passos

1. Extrair `"exit"` para constante nomeada e usar em todos os pontos.
2. (Se optar por 2) expor sinal de saída tipado no Router/GameManager.
3. Atualizar testes que dependem do código `"exit"`.

## Critérios de aceite

- [ ] Nenhuma string literal de código de estado espalhada na lógica.
- [ ] Um typo em código de estado é detectável (constante única / tipo forte).
- [ ] Testes verdes.

## Riscos

Baixo na opção 1; médio se mexer em `IState` (opção 3) — coordenar com 05.
