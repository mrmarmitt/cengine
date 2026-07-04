# Build & Testes — Ambiente Verificado

Registro da verificação de que a suíte de testes do CEngine builda e roda neste
ambiente. Serve como âncora para executar o [plano de melhoria](task/README.md)
com segurança (rodar `ctest` a cada tarefa).

- **Data da verificação:** 2026-07-04
- **Plataforma:** Windows 11 Pro
- **Resultado:** ✅ **28/28 testes passaram** (build limpo, sem warnings)

## Toolchain confirmada

| Ferramenta | Versão / Local |
|-----------|----------------|
| CMake | 4.0.2 (`C:\Program Files\CMake\bin\cmake.exe`) |
| Compilador | g++ 15.1.0 — MSYS2 UCRT64 (`C:\msys64\ucrt64\bin\g++.exe`) |
| Ninja | `C:\msys64\usr\bin\ninja.exe` |
| GoogleTest | v1.17.0 (via FetchContent, já em cache) |

> MSVC (`cl`) não está disponível no PATH neste ambiente; o build usa o preset
> MinGW/MSYS2.

## Como reproduzir

O working directory precisa ser a raiz do projeto **cengine**. O preset
`msys2-mingw` exige o MSYS2 no PATH (por causa do `g++`/`ninja`).

```powershell
# 1. Garantir o toolchain MSYS2 no PATH
$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"

# 2. Entrar no diretório do projeto
Set-Location C:\Users\mrmar\Documents\projetos_de_estudo\c++\cengine

# 3. Configurar (baixa GoogleTest na 1ª vez; precisa de rede)
cmake --preset msys2-mingw

# 4. Buildar a lib + o executável de testes
cmake --build --preset msys2-mingw

# 5. Rodar os testes
Set-Location out/build/msys2-mingw
ctest --output-on-failure
```

Alternativa (rodar o binário direto):

```powershell
out/build/msys2-mingw/tests/cengine_tests.exe
```

> **Estrutura (pós-tarefa 05a):** o projeto é multi-target —
> `cengine::core` (`core/`) + `cengine::routing` (`modules/routing/`) + testes
> (`tests/`, organizados por camada). Módulos são opt-in via
> `-DCENGINE_BUILD_ROUTING=ON/OFF` e `-DCENGINE_BUILD_TESTS=ON/OFF`. O core
> builda sozinho sem o routing.

## Saída obtida

```
-- Configuring done (1.5s)
-- Generating done (0.1s)
[100%] Built target cengine_tests
...
100% tests passed, 0 tests failed out of 28
Total Test time (real) = 0.20 sec
```

## Cobertura da suíte (28 testes)

- `SceneRepositoryTest` — registro/instanciação lazy via factory, unload,
  clone de estado, comparação de estados.
- `RouterInMemoryTest` — API enxuta do Router e delegação para o repositório.
- `GameManagerTest` — ciclo `onEnter/render/input/onExit` e condição de saída.
- `EngineManagerTest` + `EngineManagerIntegrationTest` — loop principal validado
  por **call-log** (ordem exata das chamadas).

## Observações ligadas ao plano de melhoria

1. **O preset só funcionou por sorte de ambiente.** `CMakePresets.json` tem
   caminhos absolutos hardcoded (`C:/msys64/ucrt64/...`) que batem com esta
   máquina; numa máquina limpa falharia. → **[Tarefa 08](task/08-presets-and-ci.md)**.

2. **Rede de segurança sólida.** Os 28 testes (incl. integração por call-log)
   dão confiança para a cirurgia do `IRouter`. → **[Tarefa 05](task/05-redesign-irouter.md)**.

3. **Os nomes dos próprios testes carregam os bugs de nomenclatura:**
   - `GameManagerTest.ShouldExist_WhenStateIsNotExit_ReturnsFalse`
   - `SceneRepositoryTest.IsNextStateEqualsToCurrentSceneReturnsTrueIfCodesDifferent`
     (nome diz "Equals" mas retorna true quando os códigos **diferem**)

   Ambos serão renomeados junto do código na **[Tarefa 02](task/02-fix-inverted-names.md)**.

## Protocolo para as tarefas do plano

A cada tarefa concluída: rebuildar e rodar `ctest`. Nenhuma tarefa é considerada
`done` com a suíte quebrada (ver critérios de aceite em cada arquivo de tarefa).
