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
#include <string>
#include <utility>

#include <cengine/core/IScene.hpp>
#include <cengine/routing/IState.hpp>
#include <cengine/routing/SceneRepository.hpp>
#include <cengine/routing/RouterInMemory.hpp>
#include <cengine/routing/GameManager.hpp>

using namespace cengine::core;
using namespace cengine::routing;

namespace {

class FakeState final : public IState {
    std::string m_code;

public:
    explicit FakeState(std::string code) : m_code(std::move(code)) {}

    [[nodiscard]] std::string getCode() const override { return m_code; }
    [[nodiscard]] std::string getName() const override { return m_code; }
    [[nodiscard]] std::unique_ptr<IState> clone() const override {
        return std::make_unique<FakeState>(m_code);
    }
};

// Cena que rastreia seu tempo de vida (contador de instâncias vivas) e se seu
// onExit() foi chamado. Permite provar que a navegação destrói a cena antiga.
class TrackedScene final : public IScene {
    int* m_liveCount;
    bool* m_onExitCalled;
    bool m_onEnterExecuted{false};

public:
    TrackedScene(int* liveCount, bool* onExitCalled)
        : m_liveCount(liveCount), m_onExitCalled(onExitCalled) {
        ++(*m_liveCount);
    }
    ~TrackedScene() override { --(*m_liveCount); }

    void onEnter() override {}
    void onEnterExecuted() override { m_onEnterExecuted = true; }
    [[nodiscard]] bool isOnEnterExecuted() const override { return m_onEnterExecuted; }
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

    std::shared_ptr<SceneRepository> repository;
    std::shared_ptr<RouterInMemory> router;
    std::unique_ptr<GameManager> gameManager;

    void SetUp() override {
        repository = std::make_shared<SceneRepository>(std::make_unique<FakeState>("A"));
        repository->registerFactory("A", [this]() {
            return std::make_unique<TrackedScene>(&liveA, &onExitCalledA);
        });
        repository->registerFactory("B", [this]() {
            return std::make_unique<TrackedScene>(&liveB, &onExitCalledB);
        });

        router = std::make_shared<RouterInMemory>(repository);
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
