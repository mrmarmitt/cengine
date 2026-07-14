# -------------------------------------------------------------
# cengine::collision2d — módulo opcional de DETECÇÃO de colisão 2D
#
# Não depende de core nem de routing: é geometria pura. O jogo escolhe linkar
# (ou não) — ADR 0001, core mínimo + módulos opt-in.
# -------------------------------------------------------------
add_library(cengine_collision2d STATIC
    src/Intersects.cpp
)
add_library(cengine::collision2d ALIAS cengine_collision2d)

target_include_directories(cengine_collision2d PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Propaga o requisito de padrão C++ para quem linkar o módulo (ver tarefa 07).
target_compile_features(cengine_collision2d PUBLIC cxx_std_23)

set_target_properties(cengine_collision2d PROPERTIES OUTPUT_NAME "cengine_collision2d")
