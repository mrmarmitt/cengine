# -------------------------------------------------------------
# cengine::anim — módulo opcional de clip de animação (frames sobre tempo)
#
# A máquina clip + frame + acumulador (task 25). Não depende de nada: é
# aritmética pura. Quem anima por regra de domínio (spaceinvaders) simplesmente
# não linka — ADR 0001, core mínimo + módulos opt-in.
# -------------------------------------------------------------
add_library(cengine_anim STATIC
    src/Animator.cpp
)
add_library(cengine::anim ALIAS cengine_anim)

target_include_directories(cengine_anim PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Propaga o requisito de padrão C++ para quem linkar o módulo (ver tarefa 07).
target_compile_features(cengine_anim PUBLIC cxx_std_23)

set_target_properties(cengine_anim PROPERTIES OUTPUT_NAME "cengine_anim")
