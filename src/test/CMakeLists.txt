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
# 2. Definir os arquivos-fonte do executável de testes
# -------------------------------------------------------------
# file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

set(UNIT_TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/unit/engine/service/repository/SceneRepositoryTest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/unit/engine/service/impl/RouterInMemoryTest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/unit/engine/GameManagerTest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/unit/engine/EngineManagerTest.cpp"
)

set(INTEGRATION_TEST_SOURCE
  "${CMAKE_CURRENT_SOURCE_DIR}/integration/EngineManagerIntegrationTest.cpp"
)

# -------------------------------------------------------------
# 3. Criar o executável de testes
# -------------------------------------------------------------
add_executable(cengine_tests ${UNIT_TEST_SOURCES} ${INTEGRATION_TEST_SOURCE})

# --------------------------------------------------------ö-----
# 4. Vincular as bibliotecas e diretórios de include
# -------------------------------------------------------------
target_link_libraries(cengine_tests
  PRIVATE
  cengine_lib         # Sua biblioteca principal
  gtest_main          # A biblioteca principal do GoogleTest
  gmock_main          # A biblioteca principal do Google Mock
)

target_include_directories(cengine_tests
  PRIVATE
  ${CMAKE_SOURCE_DIR}/include # Headers públicos da sua engine
  ${CMAKE_SOURCE_DIR}/src     # Para acessar os headers internos, se necessário
)

# -------------------------------------------------------------
# 5. Configurar as propriedades do executável de testes
# -------------------------------------------------------------
set_target_properties(cengine_tests PROPERTIES
  CXX_STANDARD 23
  CXX_STANDARD_REQUIRED ON
)

# -------------------------------------------------------------
# 6. Adicionar o teste ao CTest
# -------------------------------------------------------------
include(GoogleTest)

gtest_discover_tests(cengine_tests)