# -------------------------------------------------------------
# cengine::camera2d — módulo opcional de câmera 2D (mecanismo)
#
# Projeção mundo -> viewport + culling. Depende do collision2d (Vec2/Aabb e o
# intersects do culling); NÃO contém seguimento — para onde a câmera olha é
# política de cada jogo (task 23). O jogo escolhe linkar (ou não) — ADR 0001.
# -------------------------------------------------------------
add_library(cengine_camera2d STATIC
    src/Viewport.cpp
)
add_library(cengine::camera2d ALIAS cengine_camera2d)

target_include_directories(cengine_camera2d PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(cengine_camera2d PUBLIC cengine::collision2d)

# Propaga o requisito de padrão C++ para quem linkar o módulo (ver tarefa 07).
target_compile_features(cengine_camera2d PUBLIC cxx_std_23)

set_target_properties(cengine_camera2d PROPERTIES OUTPUT_NAME "cengine_camera2d")
