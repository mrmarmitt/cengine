# Configuração dos testes

# -------------------------------------------------------------
# 1. Configurar o download e build do GoogleTest (GTest)
# -------------------------------------------------------------
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.17.0
)

FetchContent_MakeAvailable(googletest)

# -------------------------------------------------------------
# 2. Executável de testes — sempre inclui a camada core
# -------------------------------------------------------------
add_executable(cengine_tests
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/EngineManagerTest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/integration/EngineManagerIntegrationTest.cpp"
)

target_link_libraries(cengine_tests
  PRIVATE
  cengine::core
  gtest_main
  gmock_main
)

# -------------------------------------------------------------
# 3. Testes do módulo routing — só quando o módulo está habilitado
#    (evita linkar cengine::routing quando -DCENGINE_BUILD_ROUTING=OFF)
# -------------------------------------------------------------
if(CENGINE_BUILD_ROUTING)
  target_sources(cengine_tests
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/routing/GameManagerTest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/routing/RouterInMemoryTest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/routing/SceneRepositoryTest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/routing/integration/SceneLifetimeTest.cpp"
  )
  target_link_libraries(cengine_tests PRIVATE cengine::routing)
endif()

# -------------------------------------------------------------
# 4. Includes e propriedades
# -------------------------------------------------------------
# Permite incluir os mocks como <mock/MockXxx.hpp>
target_include_directories(cengine_tests
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(cengine_tests PROPERTIES
  CXX_STANDARD 23
  CXX_STANDARD_REQUIRED ON
)

# -------------------------------------------------------------
# 5. Registrar no CTest
# -------------------------------------------------------------
include(GoogleTest)

gtest_discover_tests(cengine_tests)
