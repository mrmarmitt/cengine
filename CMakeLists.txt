cmake_minimum_required(VERSION 3.16)

project(cengine VERSION 0.1.0 LANGUAGES CXX)

# Define padrão do projeto
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Habilita testes
enable_testing()

# Define os diretórios principais
set(CENGINE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(CENGINE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Define os arquivos fonte (.cpp), excluindo os arquivos de teste
file(GLOB_RECURSE CENGINE_SOURCES
        ${CENGINE_SRC_DIR}/main/*.cpp
)

# Cria a biblioteca estática
add_library(cengine_lib STATIC ${CENGINE_SOURCES})

# Define os diretórios de include públicos
target_include_directories(cengine_lib PUBLIC ${CENGINE_INCLUDE_DIR}
)

# Define as propriedades da biblioteca
set_target_properties(cengine_lib PROPERTIES OUTPUT_NAME "cengine" CXX_STANDARD 20
)

# Opcional: instalação da lib (se for distribuir)
# install(TARGETS cengine_lib DESTINATION lib)
# install(DIRECTORY ${CENGINE_INCLUDE_DIR}/ DESTINATION include)

# Adiciona o diretório de testes
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/test)
