# 08 — Presets portáveis + CI (Debug + sanitizers)

- **Status:** done ✅ (2026-07-04, branch `feature/build-ecosystem-07-08`)

> Executado:
> - `CMakePresets.json` agora tem presets **portáveis** (`debug`/`release`/`asan`,
>   Ninja, sem paths hardcoded). O preset machine-specific `msys2-mingw` migrou
>   para `CMakeUserPresets.json` (gitignorado).
> - CI ganhou o job `build-and-test-linux-sanitizers` (Debug + ASan/UBSan).
> - Documentado no README (seção *Building*).
> - Ressalva conhecida: no MSYS2 desta máquina o `ninja` embutido mastiga paths
>   nativos, então os presets Ninja são validados em CI/outras máquinas; local
>   valida-se pelo `msys2-mingw`. Presets portáveis confirmados via
>   `cmake --list-presets`.
- **Prioridade:** 🟢 Baixa
- **Categoria:** Ecossistema / build
- **Depende de:** — (independente; porém ASan ajuda a validar a tarefa 06)

## Problema

### 8.1 `CMakePresets.json` não é portável

O único preset (`msys2-mingw`) tem **caminhos absolutos hardcoded** da sua
máquina:

```json
"CMAKE_CXX_COMPILER": "C:/msys64/ucrt64/bin/g++.exe",
"CMAKE_MAKE_PROGRAM": "C:/msys64/ucrt64/bin/mingw32-make.exe"
```

Ninguém além de você consegue usar. E o CI nem usa presets — roda `cmake` puro.
(Compare com o 8Puzzle, que tem presets genéricos `x64-debug`/`x64-release`.)

### 8.2 CI não cobre Debug nem sanitizers

O workflow builda Release no Windows e default no Linux, sem AddressSanitizer /
UBSanitizer. Para uma engine cheia de ponteiros e referências (ver tarefa 06),
ASan/UBSan é a ferramenta que pega dangling/use-after-free automaticamente.

## Objetivo

Presets utilizáveis por qualquer pessoa e um CI que exercite Debug + sanitizers.

## Passos

1. Adicionar presets genéricos que não dependam de caminhos absolutos
   (ex.: `debug`, `release`, deixando o CMake detectar o compilador; manter o
   `msys2-mingw` como preset local opcional, talvez em `CMakeUserPresets.json`
   que fica fora do versionamento).
2. Adicionar um preset/flags de sanitizers para builds Debug
   (`-fsanitize=address,undefined` em GCC/Clang).
3. Atualizar o CI:
   - Adicionar job Debug no Linux com ASan/UBSan rodando `ctest`.
   - Opcional: usar os presets no CI para evitar duplicar configuração.

## Critérios de aceite

- [ ] `cmake --preset debug` funciona numa máquina limpa (sem paths hardcoded).
- [ ] CI tem um job Debug com ASan/UBSan verde.
- [ ] Preset específico de máquina (msys2) não atrapalha outros devs.

## Riscos

Baixo. Pode revelar bugs de memória latentes ao ligar ASan — o que é o objetivo.
