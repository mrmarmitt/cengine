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
# 2. Arquivos-fonte de teste, organizados por camada
# -------------------------------------------------------------
set(CORE_TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/core/EngineManagerTest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/integration/EngineManagerIntegrationTest.cpp"
)

set(ROUTING_TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/routing/GameManagerTest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/routing/RouterInMemoryTest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/routing/SceneRepositoryTest.cpp"
)

# -------------------------------------------------------------
# 3. Executável de testes
# -------------------------------------------------------------
add_executable(cengine_tests
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
    ${CORE_TEST_SOURCES}
    ${ROUTING_TEST_SOURCES}
)

# -------------------------------------------------------------
# 4. Bibliotecas e includes
# -------------------------------------------------------------
target_link_libraries(cengine_tests
  PRIVATE
  cengine::core
  cengine::routing
  gtest_main
  gmock_main
)

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
