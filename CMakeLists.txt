cmake_minimum_required(VERSION 3.23)

project(cengine VERSION 0.5.0 LANGUAGES CXX)

# Define padrÃ£o do projeto
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Habilita testes
enable_testing()

# -------------------------------------------------------------
# OpÃ§Ãµes de build (mÃ³dulos opt-in â€” ver ADR 0001)
# -------------------------------------------------------------
option(CENGINE_BUILD_ROUTING "Build the optional routing module (cengine::routing)" ON)
option(CENGINE_BUILD_TESTS   "Build the test suite" ON)

# -------------------------------------------------------------
# Core (sempre) + mÃ³dulos opcionais
# -------------------------------------------------------------
add_subdirectory(core)

if(CENGINE_BUILD_ROUTING)
    add_subdirectory(modules/routing)
endif()

if(CENGINE_BUILD_TESTS)
    add_subdirectory(tests)
endif()
