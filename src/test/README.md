# Testes Unitários do CEngine

Este diretório contém os testes unitários para o CEngine, utilizando o framework Google Test (gtest) e Google Mock (gmock).

## Estrutura

- `main.cpp`: Ponto de entrada para os testes
- `engine/`: Testes para os componentes do motor
  - `RouterServiceTest.cpp`: Testes para a classe RouterService

## Requisitos

O sistema de build do projeto está configurado para baixar e compilar automaticamente o Google Test (gtest) e Google Mock (gmock) caso não estejam instalados no seu sistema. Não é necessário instalar manualmente essas bibliotecas.

Se você preferir usar uma instalação local do Google Test, o sistema tentará encontrá-la primeiro antes de fazer o download.

## Como executar os testes

### No Linux/macOS

1. Compile o projeto com suporte a testes:

```bash
mkdir -p build && cd build
cmake ..
make
```

2. Execute os testes:

```bash
ctest
```

Ou execute diretamente o binário de testes:

```bash
./cengine_tests
```

### No Windows

1. Compile o projeto com suporte a testes:

```powershell
md -Force build
cd build
cmake ..
cmake --build . --config Debug
```

2. Execute os testes:

```powershell
ctest -C Debug
```

Ou execute diretamente o binário de testes:

```powershell
.\Debug\cengine_tests.exe
```

Durante a primeira compilação, o CMake irá automaticamente baixar e compilar o Google Test se ele não for encontrado no seu sistema. Isso pode levar alguns minutos.

## Adicionando novos testes

Para adicionar novos testes:

1. Crie um novo arquivo de teste na pasta apropriada (ex: `engine/NovoComponenteTest.cpp`)
2. Implemente os testes usando o framework Google Test
3. O arquivo será automaticamente incluído na compilação dos testes