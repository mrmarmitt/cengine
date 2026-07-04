# -------------------------------------------------------------
# cengine::core — o game loop + portas (agnóstico de gráfico)
# -------------------------------------------------------------
add_library(cengine_core STATIC
    src/EngineManager.cpp
)
add_library(cengine::core ALIAS cengine_core)

target_include_directories(cengine_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(cengine_core PUBLIC cxx_std_23)

set_target_properties(cengine_core PROPERTIES OUTPUT_NAME "cengine_core")
