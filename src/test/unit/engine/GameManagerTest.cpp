#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>
#include <string>

#include <engine/IScene.hpp>

#include <test/unit/mock/MockScene.hpp>
#include <test/unit/mock/MockRouter.hpp>

#include <main/engine/GameManager.hpp>

// Fixture para os testes de GameManager
class GameManagerTest : public ::testing::Test {
protected:
    std::shared_ptr<MockRouter> m_mockRouter;
    std::unique_ptr<GameManager> m_gameManager;
    MockScene m_mockScene;

    void SetUp() override {
        // Inicializa os mocks e a classe a ser testada
        m_mockRouter = std::make_shared<MockRouter>();
        m_gameManager = std::make_unique<GameManager>(m_mockRouter);
    }
};

// Teste para o método onEnter() quando a tela ainda não foi inicializada
TEST_F(GameManagerTest, OnEnter_WhenScreenNotInitialized_CallsOnEnter) {
    // Expectativas para o mock do roteador
    EXPECT_CALL(*m_mockRouter, getCurrentCachedScreen()).WillOnce(testing::ReturnRef(m_mockScene));
    // Expectativas para o mock da cena
    EXPECT_CALL(m_mockScene, isOnEnterExecuted()).WillOnce(testing::Return(false));
    EXPECT_CALL(m_mockScene, onEnter()).Times(1);
    EXPECT_CALL(m_mockScene, onEnterExecuted()).Times(1);
    
    // Chama o método a ser testado
    m_gameManager->onEnter();
}

// Teste para o método onEnter() quando a tela já foi inicializada
TEST_F(GameManagerTest, OnEnter_WhenScreenAlreadyInitialized_DoesNothing) {
    // Expectativas para o mock do roteador
    EXPECT_CALL(*m_mockRouter, getCurrentCachedScreen()).WillOnce(testing::ReturnRef(m_mockScene));

    // Expectativas para o mock da cena
    EXPECT_CALL(m_mockScene, isOnEnterExecuted()).WillOnce(testing::Return(true));
    EXPECT_CALL(m_mockScene, onEnter()).Times(0);
    EXPECT_CALL(m_mockScene, onEnterExecuted()).Times(0);
    
    // Chama o método a ser testado
    m_gameManager->onEnter();
}

// Teste para o método render()
TEST_F(GameManagerTest, Render_CallsDrawOnCurrentScreen) {
    // Expectativa para o mock do roteador
    EXPECT_CALL(*m_mockRouter, getCurrentCachedScreen()).WillOnce(testing::ReturnRef(m_mockScene));

    // Expectativa para o mock da cena
    EXPECT_CALL(m_mockScene, draw()).Times(1);

    // Chama o método a ser testado
    m_gameManager->render();
}

// Teste para o método input()
TEST_F(GameManagerTest, Input_CallsInputOnCurrentScreen) {
    // Expectativa para o mock do roteador
    EXPECT_CALL(*m_mockRouter, getCurrentCachedScreen()).WillOnce(testing::ReturnRef(m_mockScene));

    // Expectativa para o mock da cena
    EXPECT_CALL(m_mockScene, input()).Times(1);

    // Chama o método a ser testado
    m_gameManager->input();
}

// Teste para o método onExit() quando há uma próxima tela
TEST_F(GameManagerTest, OnExit_WhenNextScreenExists_CallsOnExitAndGoToNext) {
    // Expectativas para o mock do roteador
    EXPECT_CALL(*m_mockRouter, hasNextScreen()).WillOnce(testing::Return(true));
    EXPECT_CALL(*m_mockRouter, getCurrentCachedScreen()).WillOnce(testing::ReturnRef(m_mockScene));
    EXPECT_CALL(*m_mockRouter, goToNextScreen()).Times(1);

    // Expectativa para o mock da cena
    EXPECT_CALL(m_mockScene, onExit()).Times(1);
    
    // Chama o método a ser testado
    m_gameManager->onExit();
}

// Teste para o método onExit() quando não há uma próxima tela
TEST_F(GameManagerTest, OnExit_WhenNextScreenDoesNotExist_DoesNothing) {
    // Expectativa para o mock do roteador
    EXPECT_CALL(*m_mockRouter, hasNextScreen()).WillOnce(testing::Return(false));
    EXPECT_CALL(*m_mockRouter, getCurrentCachedScreen()).Times(0);
    EXPECT_CALL(*m_mockRouter, goToNextScreen()).Times(0);

    // Expectativa para o mock da cena
    EXPECT_CALL(m_mockScene, onExit()).Times(0);
    
    // Chama o método a ser testado
    m_gameManager->onExit();
}

// Teste para o método shouldExist() quando o código de estado é "exit"
TEST_F(GameManagerTest, ShouldExist_WhenStateIsExit_ReturnsTrue) {
    // Expectativa para o mock do roteador
    EXPECT_CALL(*m_mockRouter, getCurrentStateGameCode()).WillOnce(testing::Return("exit"));

    // Chama o método e verifica o retorno
    ASSERT_TRUE(m_gameManager->shouldExist());
}

// Teste para o método shouldExist() quando o código de estado não é "exit"
TEST_F(GameManagerTest, ShouldExist_WhenStateIsNotExit_ReturnsFalse) {
    // Expectativa para o mock do roteador
    EXPECT_CALL(*m_mockRouter, getCurrentStateGameCode()).WillOnce(testing::Return("main_menu"));

    // Chama o método e verifica o retorno
    ASSERT_FALSE(m_gameManager->shouldExist());
}
