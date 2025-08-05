# CMakeList.txt: projeto de CMake para cengine, incluir origem e definir
# lógica específica a um projeto aqui.
#
cmake_minimum_required (VERSION 3.8)

include(FetchContent)

project(CEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Inclui todos os diretórios de headers dentro de src/
include_directories(src)

# -----------------------------
# Código do projeto (Biblioteca)
# -----------------------------

# Coleta os arquivos fonte do projeto
file(GLOB_RECURSE PROJECT_SOURCES
        "src/*.cpp" "src/*.h"
)

# Cria a biblioteca.
# O "core_lib" é o nome da sua biblioteca. Você pode mudar para algo como "MyEngine".
# O tipo STATIC significa que ela será uma biblioteca estática.
add_library(cengine_lib STATIC ${PROJECT_SOURCES})
target_include_directories(cengine_lib PUBLIC src)
