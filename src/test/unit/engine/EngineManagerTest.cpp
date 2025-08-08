#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <test/unit/mock/MockWindowManager.hpp>
#include <test/unit/mock/MockGameManager.hpp>
#include <test/unit/mock/MockRouter.hpp>

#include <engine/EngineManager.hpp>

// Fixture para os testes de EngineManager
class EngineManagerTest : public ::testing::Test {
protected:
    // A classe EngineManager aceita unique_ptr. Os mocks devem ser tratados
    // como unique_ptr para garantir a semântica de propriedade correta.
    std::unique_ptr<MockWindowManager> m_mockWindowManager;
    std::unique_ptr<MockGameManager> m_mockGameManager;

    // Pointers crus para configurar as expectativas após transferir a propriedade.
    MockWindowManager* m_capturedWindowManager;
    MockGameManager* m_capturedGameManager;

    // A classe a ser testada
    std::unique_ptr<EngineManager> m_engineManager;

    void SetUp() override {
        // Inicializa os mocks como unique_ptr
        m_mockWindowManager = std::make_unique<MockWindowManager>();
        m_mockGameManager = std::make_unique<MockGameManager>();

        // Captura os ponteiros crus para configurar as expectativas mais tarde
        m_capturedWindowManager = m_mockWindowManager.get();
        m_capturedGameManager = m_mockGameManager.get();

        // Inicializa a classe EngineManager, transferindo a propriedade dos mocks
        m_engineManager = std::make_unique<EngineManager>(
            std::move(m_mockWindowManager), 
            std::move(m_mockGameManager));
    }
};

// Teste para o método start() quando a tela ainda já foi inicializada
TEST_F(EngineManagerTest, Start_InitializesWindowAndRunsLoop) {
    testing::InSequence s;

    // 1. O método start() chama windowManager->init().
    EXPECT_CALL(*m_capturedWindowManager, init()).Times(1);

    // 2. O loop 'run' começa.
    EXPECT_CALL(*m_capturedWindowManager, update()).Times(1);
    EXPECT_CALL(*m_capturedGameManager, onEnter()).Times(1);
    EXPECT_CALL(*m_capturedGameManager, input()).Times(1);
    EXPECT_CALL(*m_capturedGameManager, render()).Times(1);
    EXPECT_CALL(*m_capturedGameManager, onExit()).Times(1);

    // 3. shouldExist() é chamado para verificar se o loop deve continuar.
    EXPECT_CALL(*m_capturedGameManager, shouldExist()).WillOnce(testing::Return(true));

    // 4. O loop termina e o método cleanup() é chamado.
    EXPECT_CALL(*m_capturedGameManager, cleanup()).Times(1);
    EXPECT_CALL(*m_capturedWindowManager, cleanup()).Times(1);

    // Chamar o método start() da classe que estamos testando
    m_engineManager->start();
}

// Teste para o método input()
TEST_F(EngineManagerTest, Input_CallsGameManagerInput) {
    EXPECT_CALL(*m_capturedGameManager, input()).Times(1);

    m_engineManager->input();
}

// Teste para o método cleanup()
TEST_F(EngineManagerTest, Cleanup_CallsDependenciesCleanup) {
    testing::InSequence s;

    // Esperamos a chamada para o cleanup do windowManager
    EXPECT_CALL(*m_capturedGameManager, cleanup()).Times(1);
    EXPECT_CALL(*m_capturedWindowManager, cleanup()).Times(1);

    m_engineManager->cleanup();
}