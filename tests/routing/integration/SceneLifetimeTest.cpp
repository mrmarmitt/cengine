// Teste de regressão para .ai/task/06-scene-lifetime.md.
//
// Exercita o ciclo "obter cena -> navegar -> commit (unload)" através das
// implementações REAIS (SceneRepository + RouterInMemory + GameManager), em vez
// de mocks. Sob AddressSanitizer (preset `asan` / job de CI da tarefa 08),
// qualquer uso de uma IScene& após o unload da cena dispararia use-after-free e
// falharia o teste. Também valida, via contador de instâncias vivas, que a cena
// de saída é de fato destruída na navegação.

#include "gtest/gtest.h"

#include <memory>
#include <utility>

#include <cengine/core/IScene.hpp>
#include <cengine/routing/SceneRepository.hpp>
#include <cengine/routing/RouterInMemory.hpp>
#include <cengine/routing/GameManager.hpp>

#include <mock/FakeState.hpp>

using namespace cengine::core;
using namespace cengine::routing;

namespace {

// Cena que rastreia seu tempo de vida (contador de instâncias vivas), quantas
// vezes foi ativada (onEnter) e se seu onExit() foi chamado. Permite provar que
// a navegação destrói a cena antiga e que a reativação acontece via engine.
class TrackedScene final : public IScene {
    int* m_liveCount;
    bool* m_onExitCalled;
    int* m_onEnterCount;

public:
    TrackedScene(int* liveCount, bool* onExitCalled, int* onEnterCount)
        : m_liveCount(liveCount), m_onExitCalled(onExitCalled), m_onEnterCount(onEnterCount) {
        ++(*m_liveCount);
    }
    ~TrackedScene() override { --(*m_liveCount); }

    void onEnter() override { ++(*m_onEnterCount); }
    void update(Seconds) override {}
    void draw() override {}
    void input() override {}
    void onExit() override { *m_onExitCalled = true; }
};

} // namespace

class SceneLifetimeTest : public ::testing::Test {
protected:
    int liveA{0};
    int liveB{0};
    bool onExitCalledA{false};
    bool onExitCalledB{false};
    int onEnterCountA{0};
    int onEnterCountB{0};

    std::shared_ptr<RouterInMemory> router;
    std::unique_ptr<GameManager> gameManager;

    void SetUp() override {
        // Fiação recomendada pós-task 13: o jogo registra as factories e
        // TRANSFERE a posse do repositório ao router — nenhuma referência
        // externa sobra para descarregar cenas por fora da navegação.
        auto repository = std::make_unique<SceneRepository>();
        repository->registerFactory("A", [this]() {
            return std::make_unique<TrackedScene>(&liveA, &onExitCalledA, &onEnterCountA);
        });
        repository->registerFactory("B", [this]() {
            return std::make_unique<TrackedScene>(&liveB, &onExitCalledB, &onEnterCountB);
        });

        router = std::make_shared<RouterInMemory>(
            std::move(repository), std::make_unique<FakeState>("A"));
        gameManager = std::make_unique<GameManager>(router);
    }
};

// Navegar de A para B deve descarregar (destruir) a cena A sem nenhum acesso
// inválido à referência obtida durante onExit(). Sob ASan a ausência de
// use-after-free é verificada pelo próprio runtime.
TEST_F(SceneLifetimeTest, NavigateUnloadsPreviousSceneWithoutDanglingAccess) {
    // Entra na cena inicial "A".
    gameManager->onEnter();
    ASSERT_EQ(liveA, 1);
    ASSERT_EQ(liveB, 0);

    // Solicita transição para "B" -> passa a existir mudança de estado pendente.
    router->requestState(std::make_unique<FakeState>("B"));
    ASSERT_TRUE(router->hasPendingStateChange());

    // onExit() obtém a cena atual (A), chama seu onExit() e comita a troca, o que
    // descarrega A. A referência a A não pode sobreviver ao commit.
    gameManager->onExit();

    EXPECT_TRUE(onExitCalledA);      // A recebeu onExit antes de ser destruída
    EXPECT_EQ(liveA, 0);             // A foi de fato descarregada/destruída
    EXPECT_FALSE(router->hasPendingStateChange());  // estado atual agora é "B"

    // Entrar de novo instancia "B" a partir da factory — nenhum acesso reaproveita
    // memória da cena A já liberada.
    gameManager->onEnter();
    EXPECT_EQ(liveB, 1);
}

// Regressão para .ai/task/12: navegar A -> B -> A deve REATIVAR a cena A
// (onEnter roda de novo na volta). Hoje isso é sustentado pelo unload no
// commit + reset do tracking no GameManager; o teste protege o comportamento
// caso a política de unload mude (ex.: keep-alive de cenas).
TEST_F(SceneLifetimeTest, NavigatingBackReactivatesScene) {
    // Ativa A.
    gameManager->onEnter();
    EXPECT_EQ(onEnterCountA, 1);

    // A -> B.
    router->requestState(std::make_unique<FakeState>("B"));
    gameManager->onExit();
    gameManager->onEnter();
    EXPECT_EQ(onEnterCountB, 1);

    // B -> A: a volta deve reativar A (nova ativação, novo onEnter).
    router->requestState(std::make_unique<FakeState>("A"));
    gameManager->onExit();
    gameManager->onEnter();

    EXPECT_EQ(onEnterCountA, 2);
    EXPECT_EQ(liveA, 1);
    EXPECT_EQ(liveB, 0); // B foi descarregada na volta
}

// Critério de aceite da task 13 (achado do review do PR #11): o caminho
// SUPORTADO para descartar/recriar a cena do estado ativo é requestState com o
// mesmo código — o commit descarrega a instância antiga e a próxima iteração
// recria E reativa a cena (onEnter roda na instância nova). Eviction por fora
// da navegação deixou de ser possível por construção: o router é o único dono
// do repositório.
TEST_F(SceneLifetimeTest, RequestingCurrentStateReloadsAndReactivatesScene) {
    // Ativa A.
    gameManager->onEnter();
    ASSERT_EQ(liveA, 1);
    ASSERT_EQ(onEnterCountA, 1);

    // Reload deliberado: pedir o próprio estado "A".
    router->requestState(std::make_unique<FakeState>("A"));
    ASSERT_TRUE(router->hasPendingStateChange());

    gameManager->onExit();
    EXPECT_TRUE(onExitCalledA);  // a instância antiga recebeu onExit
    EXPECT_EQ(liveA, 0);         // e foi destruída no commit

    // A próxima iteração recria a cena via factory e a REATIVA.
    gameManager->onEnter();
    EXPECT_EQ(liveA, 1);
    EXPECT_EQ(onEnterCountA, 2);
}
