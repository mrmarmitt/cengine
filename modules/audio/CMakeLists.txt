# -------------------------------------------------------------
# cengine::audio — módulo opcional: a PORTA de áudio (o contrato entre cenas e
# plataforma: `play(id)`). O backend (síntese, XAudio2, pool de vozes) nunca
# entra aqui — mora na plataforma de cada jogo, como a captura no input.
#
# Header-only (o contrato é uma interface): biblioteca INTERFACE.
# Não depende de core nem de routing. O jogo escolhe linkar (ou não) — ADR 0001.
# -------------------------------------------------------------
add_library(cengine_audio INTERFACE)
add_library(cengine::audio ALIAS cengine_audio)

target_include_directories(cengine_audio INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Propaga o requisito de padrão C++ para quem linkar o módulo (ver tarefa 07).
target_compile_features(cengine_audio INTERFACE cxx_std_23)
