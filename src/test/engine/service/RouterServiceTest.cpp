#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <memory>
#include <string>

// Inclua os arquivos de cabeçalho da sua implementação
#include "../src/main/engine/service/RouterService.hpp"
#include <engine/IState.hpp>
#include <engine/IScene.hpp>

// Usando o namespace de testes do gmock para simplificar
using ::testing::_;
using ::testing::Return;
using ::testing::ByRef;
using ::testing::InSequence;

// Mocks das dependências
class MockState : public IState {
public:
    MOCK_METHOD(std::string, getCode, (), (const, override));
    MOCK_METHOD(std::string, getName, (), (const, override));
    MOCK_METHOD(std::unique_ptr<IState>, clone, (), (const, override));
};

class MockScene : public IScene {
public:
    MOCK_METHOD(void, onEnter, (), (override));
    MOCK_METHOD(void, draw, (), (override));
    MOCK_METHOD(void, input, (), (override));
    MOCK_METHOD(void, onExit, (), (override));
};

class MockSceneRepository {
public:
    MOCK_METHOD(void, persistNextState, (std::unique_ptr<IState>), (const));
    MOCK_METHOD(IState&, getCurrentStateGame, (), (const));
    MOCK_METHOD(IScene&, getScene, (const std::string&), (const));
    MOCK_METHOD(IState&, getNextStateGame, (), (const));
    MOCK_METHOD(bool, isNextStateEqualsToCurrentScene, (), (const));
    MOCK_METHOD(void, unloadScene, (const std::string&), (const));
    MOCK_METHOD(void, persisteCurrentState, (), (const));
};

// Fixture para a classe RouterServiceTest
class RouterServiceTest : public ::testing::Test {
protected:
    std::shared_ptr<MockSceneRepository> mockRepo;
    std::unique_ptr<RouterService> routerService;
    MockState mockCurrentState;
    MockState mockNextState;
    MockScene mockScene;

    void SetUp() override {
        // Inicializa o mock do repositório
        mockRepo = std::make_shared<MockSceneRepository>();
        // Inicializa o serviço com o repositório mockado
        routerService = std::make_unique<RouterService>(mockRepo);
    }
};

// Teste: setNextState deve delegar a chamada para SceneRepository::persistNextState.
TEST_F(RouterServiceTest, SetNextStateDelegatesToRepository) {
    // 1. Cria um unique_ptr para o estado a ser passado
    auto newState = std::make_unique<MockState>();
    // 2. Define a expectativa de que o método persistNextState do mock será chamado.
    // O ByRef(newState) não pode ser usado com std::move. O _ indica que qualquer unique_ptr é aceito.
    EXPECT_CALL(*mockRepo, persistNextState(_)).Times(1);

    // 3. Chama o método do serviço
    routerService->setNextState(std::move(newState));
}

// // Teste: getCurrentStateGame deve delegar a chamada para SceneRepository::getCurrentStateGame.
// TEST_F(RouterServiceTest, GetCurrentStateGameDelegatesCorrectly) {
//     // 1. Define a expectativa de que o método será chamado e retornará uma referência para mockCurrentState.
//     EXPECT_CALL(*mockRepo, getCurrentStateGame()).WillOnce(Return(ByRef(mockCurrentState)));
//     // 2. Chama o método e verifica se o retorno é o esperado.
//     IState& returnedState = routerService->getCurrentStateGame();
//     ASSERT_EQ(&returnedState, &mockCurrentState);
// }

// // Teste: getCurrentStateGameName deve delegar a chamada e retornar o nome correto.
// TEST_F(RouterServiceTest, GetCurrentStateGameNameDelegatesCorrectly) {
//     const std::string expectedName = "MenuState";
//     // 1. Define a expectativa que getCurrentStateGame() será chamado no repo.
//     EXPECT_CALL(*mockRepo, getCurrentStateGame()).WillOnce(Return(ByRef(mockCurrentState)));
//     // 2. Define a expectativa que getName() será chamado no mockCurrentState.
//     EXPECT_CALL(mockCurrentState, getName()).WillOnce(Return(expectedName));

//     // 3. Chama o método e verifica se o nome retornado é o esperado.
//     std::string returnedName = routerService->getCurrentStateGameName();
//     ASSERT_EQ(returnedName, expectedName);
// }

// // Teste: getCurrentStateGameCode deve delegar a chamada e retornar o código correto.
// TEST_F(RouterServiceTest, GetCurrentStateGameCodeDelegatesCorrectly) {
//     const std::string expectedCode = "MENU";
//     // 1. Define a expectativa que getCurrentStateGame() será chamado no repo.
//     EXPECT_CALL(*mockRepo, getCurrentStateGame()).WillOnce(Return(ByRef(mockCurrentState)));
//     // 2. Define a expectativa que getCode() será chamado no mockCurrentState.
//     EXPECT_CALL(mockCurrentState, getCode()).WillOnce(Return(expectedCode));

//     // 3. Chama o método e verifica se o código retornado é o esperado.
//     std::string returnedCode = routerService->getCurrentStateGameCode();
//     ASSERT_EQ(returnedCode, expectedCode);
// }

// // Teste: getCurrentCachedScreen deve delegar a chamada e retornar a scene correta.
// TEST_F(RouterServiceTest, GetCurrentCachedScreenDelegatesCorrectly) {
//     const std::string expectedCode = "MENU";
//     // 1. Define a expectativa que getCurrentStateGame() será chamado e retornará um mock de estado.
//     EXPECT_CALL(*mockRepo, getCurrentStateGame()).WillOnce(Return(ByRef(mockCurrentState)));
//     // 2. Define a expectativa que getCode() será chamado no mock do estado.
//     EXPECT_CALL(mockCurrentState, getCode()).WillOnce(Return(expectedCode));
//     // 3. Define a expectativa que getScene() será chamado no mock do repo com o código correto.
//     EXPECT_CALL(*mockRepo, getScene(expectedCode)).WillOnce(Return(ByRef(mockScene)));

//     // 4. Chama o método e verifica se a cena retornada é a esperada.
//     IScene& returnedScene = routerService->getCurrentCachedScreen();
//     ASSERT_EQ(&returnedScene, &mockScene);
// }

// // Teste: goToNextScreen deve chamar unloadScene e persisteCurrentState na ordem correta.
// TEST_F(RouterServiceTest, GoToNextScreenCallsMethodsInCorrectOrder) {
//     const std::string expectedCode = "OLD_SCREEN";

//     // Usa InSequence para garantir a ordem das chamadas
//     InSequence s;
    
//     // 1. Define a expectativa de que getCurrentStateGame() será chamado.
//     EXPECT_CALL(*mockRepo, getCurrentStateGame()).WillOnce(Return(ByRef(mockCurrentState)));
//     // 2. Define a expectativa de que getCode() será chamado para obter o código da cena a ser descarregada.
//     EXPECT_CALL(mockCurrentState, getCode()).WillOnce(Return(expectedCode));
//     // 3. Define a expectativa de que unloadScene() será chamado com o código correto.
//     EXPECT_CALL(*mockRepo, unloadScene(expectedCode)).Times(1);
//     // 4. Define a expectativa de que persisteCurrentState() será chamado em seguida.
//     EXPECT_CALL(*mockRepo, persisteCurrentState()).Times(1);

//     // 5. Chama o método que orquestra as chamadas.
//     routerService->goToNextScreen();
// }

// // Teste: hasNextScreen deve delegar a chamada para isNextStateEqualsToCurrentScene e retornar o valor.
// TEST_F(RouterServiceTest, HasNextScreenDelegatesCorrectly) {
//     // 1. Define a expectativa de que isNextStateEqualsToCurrentScene() será chamado e retornará true.
//     EXPECT_CALL(*mockRepo, isNextStateEqualsToCurrentScene()).WillOnce(Return(true));
//     ASSERT_TRUE(routerService->hasNextScreen());

//     // 2. Define a expectativa para a próxima chamada, retornando false.
//     EXPECT_CALL(*mockRepo, isNextStateEqualsToCurrentScene()).WillOnce(Return(false));
//     ASSERT_FALSE(routerService->hasNextScreen());
// }