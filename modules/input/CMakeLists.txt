# -------------------------------------------------------------
# cengine::input — módulo opcional: o VOCABULÁRIO de teclado (o contrato entre
# plataforma e cenas). A captura (WndProc, bindings, terminal) nunca entra aqui.
#
# Não depende de core nem de routing. O jogo escolhe linkar (ou não) — ADR 0001.
# -------------------------------------------------------------
add_library(cengine_input STATIC
    src/Keyboard.cpp
)
add_library(cengine::input ALIAS cengine_input)

target_include_directories(cengine_input PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Propaga o requisito de padrão C++ para quem linkar o módulo (ver tarefa 07).
target_compile_features(cengine_input PUBLIC cxx_std_23)

set_target_properties(cengine_input PROPERTIES OUTPUT_NAME "cengine_input")
