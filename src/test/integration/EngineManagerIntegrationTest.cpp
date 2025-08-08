#include "gtest/gtest.h"
#include <memory>
#include <string>

#include <engine/EngineManager.hpp>
#include "FakeImplementations.hpp"


// Fixture para os testes de Integração
class EngineManagerIntegrationTest : public ::testing::Test {
protected:
    // Implementações reais (ou "fakes") das dependências
    std::unique_ptr<FakeWindowManager> m_fakeWindowManager;
    std::unique_ptr<FakeGameManager> m_fakeGameManager;

    // Pointers crus para verificar o estado após a transferência de propriedade
    FakeWindowManager* m_rawWindowManager;
    FakeGameManager* m_rawGameManager;

    // A classe a ser testada
    std::unique_ptr<EngineManager> m_engineManager;

    void SetUp() override {
        m_fakeWindowManager = std::make_unique<FakeWindowManager>();
        m_fakeGameManager = std::make_unique<FakeGameManager>();

        // Captura os ponteiros crus para verificação
        m_rawWindowManager = m_fakeWindowManager.get();
        m_rawGameManager = m_fakeGameManager.get();
        
        // Inicializa o EngineManager com as implementações "fake"
        m_engineManager = std::make_unique<EngineManager>(
            std::move(m_fakeWindowManager),
            std::move(m_fakeGameManager)
        );
    }
};


TEST_F(EngineManagerIntegrationTest, Start_ExecutesFullLifecycle) {
    // Definimos que o loop deve rodar apenas uma vez.
    m_rawGameManager->maxRuns = 1;

    // Executa o método start() do EngineManager, que contém o loop de execução.
    m_engineManager->start();

    // Verificamos o log de chamadas para garantir que a sequência foi a correta.
    ASSERT_EQ(m_rawWindowManager->callLog.size(), 3);
    ASSERT_EQ(m_rawWindowManager->callLog[0], "init");
    ASSERT_EQ(m_rawWindowManager->callLog[1], "update");
    ASSERT_EQ(m_rawWindowManager->callLog[2], "cleanup");

    ASSERT_EQ(m_rawGameManager->callLog.size(), 5);
    ASSERT_EQ(m_rawGameManager->callLog[0], "onEnter");
    ASSERT_EQ(m_rawGameManager->callLog[1], "input");
    ASSERT_EQ(m_rawGameManager->callLog[2], "render");
    ASSERT_EQ(m_rawGameManager->callLog[3], "onExit");
    ASSERT_EQ(m_rawGameManager->callLog[4], "cleanup");
}

TEST_F(EngineManagerIntegrationTest, Start_ExecutesLoopCorrectlyForMultipleIterations) {
    // Definimos que o loop deve rodar duas vezes.
    m_rawGameManager->maxRuns = 2;
    // Precisamos ajustar o log de chamadas para o EngineManager
    // chamará o shouldExist() no final de cada iteração do loop.
    m_rawGameManager->callLog.clear(); // Limpa o log para este teste

    // Executa o método start() do EngineManager, que contém o loop de execução.
    m_engineManager->start();

    // O log do WindowManager terá 'init' uma vez e 'update' duas vezes.
    ASSERT_EQ(m_rawWindowManager->callLog.size(), 4);
    ASSERT_EQ(m_rawWindowManager->callLog[0], "init");
    ASSERT_EQ(m_rawWindowManager->callLog[1], "update");
    ASSERT_EQ(m_rawWindowManager->callLog[2], "update");
    ASSERT_EQ(m_rawWindowManager->callLog[3], "cleanup");

    // O log do GameManager terá o ciclo completo de onEnter, input, render, onExit duas vezes,
    // e o cleanup no final.
    ASSERT_EQ(m_rawGameManager->callLog.size(), 9); // 4 chamadas * 2 iterações + 1 cleanup
    ASSERT_EQ(m_rawGameManager->callLog[0], "onEnter");
    ASSERT_EQ(m_rawGameManager->callLog[1], "input");
    ASSERT_EQ(m_rawGameManager->callLog[2], "render");
    ASSERT_EQ(m_rawGameManager->callLog[3], "onExit");
    ASSERT_EQ(m_rawGameManager->callLog[4], "onEnter");
    ASSERT_EQ(m_rawGameManager->callLog[5], "input");
    ASSERT_EQ(m_rawGameManager->callLog[6], "render");
    ASSERT_EQ(m_rawGameManager->callLog[7], "onExit");
    ASSERT_EQ(m_rawGameManager->callLog[8], "cleanup");
}
