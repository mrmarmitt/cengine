#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <memory>
#include <string>

#include <engine/IState.hpp>
#include <engine/IScene.hpp>

#include <test/unit/mock/MockScene.hpp>
#include <test/unit/mock/MockState.hpp>
#include <test/unit/mock/MockSceneRepository.hpp>

// Inclua os arquivos de cabeçalho da sua implementação
#include "../src/main/engine/service/impl/RouterInMemory.hpp"

// Fixture para a classe RouterServiceTest
class RouterInMemoryTest : public ::testing::Test {
protected:
    std::shared_ptr<MockSceneRepository> mockSceneRepository;
    std::unique_ptr<RouterInMemory> routerService;

    // Mocks de dependências que os métodos de teste usarão
    MockState mockCurrentState;
    MockScene mockScene;

    void SetUp() override {
        // Inicializa o mock do repositório
        mockSceneRepository = std::make_shared<MockSceneRepository>();
        // Inicializa o serviço com o repositório mockado
        routerService = std::make_unique<RouterInMemory>(mockSceneRepository);
    }
};

// Teste: setNextState deve delegar a chamada para SceneRepository::persistNextState.
TEST_F(RouterInMemoryTest, SetNextStateDelegatesToRepository) {
    // 1. Cria um unique_ptr para o estado a ser passado
    auto newState = std::make_unique<MockState>();
    // 2. Define a expectativa de que o método persistNextState do mock será chamado.
    EXPECT_CALL(*mockSceneRepository, persistNextState(testing::A<std::unique_ptr<IState>>())).Times(1);

    // 3. Chama o método do serviço
    routerService->setNextState(std::move(newState));
}

// Teste: getCurrentStateGame deve delegar a chamada para SceneRepository::getCurrentStateGame.
TEST_F(RouterInMemoryTest, GetCurrentStateGameDelegatesCorrectly) {
    // 1. Define a expectativa de que o método será chamado e retornará uma referência para mockCurrentState.
    EXPECT_CALL(*mockSceneRepository, getCurrentStateGame()).WillOnce(testing::ReturnRef(mockCurrentState));
    // 2. Chama o método e verifica se o retorno é o esperado.
    IState& returnedState = routerService->getCurrentStateGame();
    ASSERT_EQ(&returnedState, &mockCurrentState);
}

// Teste: getCurrentStateGameName deve delegar a chamada e retornar o nome correto.
TEST_F(RouterInMemoryTest, GetCurrentStateGameNameDelegatesCorrectly) {
    const std::string expectedName = "MenuState";
    // 1. Define a expectativa que getCurrentStateGame() será chamado no repo.
    EXPECT_CALL(*mockSceneRepository, getCurrentStateGame()).WillOnce(testing::ReturnRef(mockCurrentState));
    // 2. Define a expectativa que getName() será chamado no mockCurrentState.
    EXPECT_CALL(mockCurrentState, getName()).WillOnce(testing::Return(expectedName));

    // 3. Chama o método e verifica se o nome retornado é o esperado.
    std::string returnedName = routerService->getCurrentStateGameName();
    ASSERT_EQ(returnedName, expectedName);
}

// Teste: getCurrentStateGameCode deve delegar a chamada e retornar o código correto.
TEST_F(RouterInMemoryTest, GetCurrentStateGameCodeDelegatesCorrectly) {
    const std::string expectedCode = "MENU";
    // 1. Define a expectativa que getCurrentStateGame() será chamado no repo.
    EXPECT_CALL(*mockSceneRepository, getCurrentStateGame()).WillOnce(testing::ReturnRef(mockCurrentState));
    // 2. Define a expectativa que getCode() será chamado no mockCurrentState.
    EXPECT_CALL(mockCurrentState, getCode()).WillOnce(testing::Return(expectedCode));

    // 3. Chama o método e verifica se o código retornado é o esperado.
    std::string returnedCode = routerService->getCurrentStateGameCode();
    ASSERT_EQ(returnedCode, expectedCode);
}

// Teste: getCurrentCachedScreen deve delegar a chamada e retornar a scene correta.
TEST_F(RouterInMemoryTest, GetCurrentCachedScreenDelegatesCorrectly) {
    const std::string expectedCode = "MENU";
    // 1. Define a expectativa que getCurrentStateGame() será chamado e retornará um mock de estado.
    EXPECT_CALL(*mockSceneRepository, getCurrentStateGame()).WillOnce(testing::ReturnRef(mockCurrentState));
    // 2. Define a expectativa que getCode() será chamado no mock do estado.
    EXPECT_CALL(mockCurrentState, getCode()).WillOnce(testing::Return(expectedCode));
    // 3. Define a expectativa que getScene() será chamado no mock do repo com o código correto.
    EXPECT_CALL(*mockSceneRepository, getScene(expectedCode)).WillOnce(testing::ReturnRef(mockScene));

    // 4. Chama o método e verifica se a cena retornada é a esperada.
    IScene& returnedScene = routerService->getCurrentCachedScreen();
    ASSERT_EQ(&returnedScene, &mockScene);
}

// Teste: goToNextScreen deve chamar unloadScene e persisteCurrentState na ordem correta.
TEST_F(RouterInMemoryTest, GoToNextScreenCallsMethodsInCorrectOrder) {
    const std::string expectedCode = "OLD_SCREEN";

    // Usa InSequence para garantir a ordem das chamadas
    testing::InSequence s;
    
    // 1. Define a expectativa de que getCurrentStateGame() será chamado.
    EXPECT_CALL(*mockSceneRepository, getCurrentStateGame()).WillOnce(testing::ReturnRef(mockCurrentState));
    // 2. Define a expectativa de que getCode() será chamado para obter o código da cena a ser descarregada.
    EXPECT_CALL(mockCurrentState, getCode()).WillOnce(testing::Return(expectedCode));
    // 3. Define a expectativa de que unloadScene() será chamado com o código correto.
    EXPECT_CALL(*mockSceneRepository, unloadScene(expectedCode)).Times(1);
    // 4. Define a expectativa de que persisteCurrentState() será chamado em seguida.
    EXPECT_CALL(*mockSceneRepository, persisteCurrentState()).Times(1);

    // 5. Chama o método que orquestra as chamadas.
    routerService->goToNextScreen();
}

// Teste: hasNextScreen deve delegar a chamada para isNextStateEqualsToCurrentScene e retornar o valor.
TEST_F(RouterInMemoryTest, HasNextScreenDelegatesCorrectly) {
    // 1. Define a expectativa de que isNextStateEqualsToCurrentScene() será chamado e retornará true.
    EXPECT_CALL(*mockSceneRepository, isNextStateEqualsToCurrentScene()).WillOnce(testing::Return(true));
    ASSERT_TRUE(routerService->hasNextScreen());

    // 2. Define a expectativa para a próxima chamada, retornando false.
    EXPECT_CALL(*mockSceneRepository, isNextStateEqualsToCurrentScene()).WillOnce(testing::Return(false));
    ASSERT_FALSE(routerService->hasNextScreen());
}