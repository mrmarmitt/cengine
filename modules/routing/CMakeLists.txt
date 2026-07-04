# -------------------------------------------------------------
# cengine::routing — módulo opcional de roteamento de cenas/estados
# Depende PUBLIC do core; o jogo escolhe linkar (ou não).
# -------------------------------------------------------------
add_library(cengine_routing STATIC
    src/GameManager.cpp
    src/RouterInMemory.cpp
    src/SceneRepository.cpp
)
add_library(cengine::routing ALIAS cengine_routing)

target_include_directories(cengine_routing PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(cengine_routing PUBLIC cengine::core)

set_target_properties(cengine_routing PROPERTIES OUTPUT_NAME "cengine_routing")
