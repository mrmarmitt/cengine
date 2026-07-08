#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <chrono>
#include <memory>
#include <stdexcept>

#include <mock/MockWindowManager.hpp>
#include <mock/MockGameManager.hpp>

#include <cengine/core/EngineManager.hpp>

using namespace cengine::core;

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

        // Inicializa a classe EngineManager, transferindo a propriedade dos mocks.
        // Relógio CONGELADO: frameTime = 0 em todo quadro -> nenhum passo de
        // update; os testes de contagem de passos ficam nas suítes de fixed
        // timestep abaixo.
        m_engineManager = std::make_unique<EngineManager>(
            std::move(m_mockWindowManager),
            std::move(m_mockGameManager),
            EngineManager::kDefaultFixedDt,
            EngineManager::kDefaultMaxFrameTime,
            [] { return EngineManager::TimePoint{}; });
    }
};

// Teste para o método start() quando a tela ainda já foi inicializada
TEST_F(EngineManagerTest, Start_InitializesWindowAndRunsLoop) {
    testing::InSequence s;

    // 1. O método start() chama windowManager->init().
    EXPECT_CALL(*m_capturedWindowManager, init()).Times(1);

    // 2. O loop 'run' começa. Com o relógio congelado, nenhum tempo acumula:
    //    o quadro "curto" tem ZERO passos de update — e o render acontece
    //    mesmo assim.
    EXPECT_CALL(*m_capturedWindowManager, update()).Times(1);
    EXPECT_CALL(*m_capturedGameManager, onEnter()).Times(1);
    EXPECT_CALL(*m_capturedGameManager, input()).Times(1);
    EXPECT_CALL(*m_capturedGameManager, update(testing::_)).Times(0);
    EXPECT_CALL(*m_capturedGameManager, render()).Times(1);
    EXPECT_CALL(*m_capturedGameManager, onExit()).Times(1);

    // 3. shouldExit() é chamado para verificar se o loop deve continuar.
    EXPECT_CALL(*m_capturedGameManager, shouldExit()).WillOnce(testing::Return(true));

    // 4. O loop termina e o método cleanup() é chamado.
    EXPECT_CALL(*m_capturedGameManager, cleanup()).Times(1);
    EXPECT_CALL(*m_capturedWindowManager, cleanup()).Times(1);

    // Chamar o método start() da classe que estamos testando
    m_engineManager->start();
}

// Teste para o método cleanup()
TEST_F(EngineManagerTest, Cleanup_CallsDependenciesCleanup) {
    testing::InSequence s;

    // Esperamos a chamada para o cleanup do windowManager
    EXPECT_CALL(*m_capturedGameManager, cleanup()).Times(1);
    EXPECT_CALL(*m_capturedWindowManager, cleanup()).Times(1);

    m_engineManager->cleanup();
}

// ------------------------------------------------------------------
// Fixed timestep (task 14): contagem de passos de update por quadro,
// com relógio fake que avança um passo controlado a cada consulta.
// ------------------------------------------------------------------
class EngineManagerFixedTimestepTest : public ::testing::Test {
protected:
    MockWindowManager* m_windowManager{nullptr};
    MockGameManager* m_gameManager{nullptr};
    std::unique_ptr<EngineManager> m_engineManager;

    // Monta a engine com um relógio que avança @p clockStep a cada chamada
    // (uma consulta antes do loop + uma por quadro -> frameTime == clockStep).
    void makeEngine(const Seconds fixedDt, const Seconds maxFrameTime, const Seconds clockStep) {
        auto windowManager = std::make_unique<MockWindowManager>();
        auto gameManager = std::make_unique<MockGameManager>();
        m_windowManager = windowManager.get();
        m_gameManager = gameManager.get();

        auto now = std::make_shared<EngineManager::TimePoint>();
        m_engineManager = std::make_unique<EngineManager>(
            std::move(windowManager),
            std::move(gameManager),
            fixedDt,
            maxFrameTime,
            [now, clockStep] {
                *now += std::chrono::duration_cast<EngineManager::Clock::duration>(clockStep);
                return *now;
            });

        // Fora do foco destes testes: um quadro só, sem InSequence.
        EXPECT_CALL(*m_windowManager, init()).Times(1);
        EXPECT_CALL(*m_windowManager, update()).Times(1);
        EXPECT_CALL(*m_windowManager, cleanup()).Times(1);
        EXPECT_CALL(*m_gameManager, onEnter()).Times(1);
        EXPECT_CALL(*m_gameManager, input()).Times(1);
        EXPECT_CALL(*m_gameManager, render()).Times(1);
        EXPECT_CALL(*m_gameManager, onExit()).Times(1);
        EXPECT_CALL(*m_gameManager, cleanup()).Times(1);
        EXPECT_CALL(*m_gameManager, shouldExit()).WillOnce(testing::Return(true));
    }
};

// Quadro longo: 35 ms com passo de 10 ms -> exatamente 3 updates, todos com o
// MESMO dt fixo (os 5 ms restantes ficam acumulados para o próximo quadro).
TEST_F(EngineManagerFixedTimestepTest, LongFrameRunsMultipleUpdatesWithSameFixedDt) {
    constexpr Seconds fixedDt{0.010};
    makeEngine(fixedDt, EngineManager::kDefaultMaxFrameTime, Seconds{0.035});

    EXPECT_CALL(*m_gameManager, update(fixedDt)).Times(3);

    m_engineManager->start();
}

// Quadro mais curto que o passo fixo: nenhum update, mas o quadro (render)
// acontece normalmente.
TEST_F(EngineManagerFixedTimestepTest, ShortFrameRunsZeroUpdates) {
    makeEngine(Seconds{0.010}, EngineManager::kDefaultMaxFrameTime, Seconds{0.004});

    EXPECT_CALL(*m_gameManager, update(testing::_)).Times(0);

    m_engineManager->start();
}

// Teto anti-espiral: um quadro de 1 s com teto de 100 ms e passo de 50 ms
// gera apenas 2 updates (o excedente é descartado, não acumulado).
TEST_F(EngineManagerFixedTimestepTest, FrameTimeIsClampedToMaxFrameTime) {
    constexpr Seconds fixedDt{0.050};
    makeEngine(fixedDt, Seconds{0.100}, Seconds{1.0});

    EXPECT_CALL(*m_gameManager, update(fixedDt)).Times(2);

    m_engineManager->start();
}

// Configuração inválida deve falhar na construção, não travar o loop.
TEST(EngineManagerConstructionTest, RejectsNonPositiveFixedDt) {
    EXPECT_THROW(EngineManager(std::make_unique<MockWindowManager>(),
                               std::make_unique<MockGameManager>(),
                               Seconds{0}),
                 std::invalid_argument);
    EXPECT_THROW(EngineManager(std::make_unique<MockWindowManager>(),
                               std::make_unique<MockGameManager>(),
                               EngineManager::kDefaultFixedDt,
                               Seconds{-1.0}),
                 std::invalid_argument);
}
