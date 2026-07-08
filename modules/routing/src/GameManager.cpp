#include <cengine/routing/GameManager.hpp>

#include <cengine/core/IScene.hpp>

#include <utility>

#include <cengine/routing/IState.hpp>
#include <cengine/routing/IRouter.hpp>
#include <cengine/routing/StateCodes.hpp>

namespace cengine::routing {

GameManager::GameManager(std::shared_ptr<IRouter> routerService) : m_routerService(std::move(routerService)){}

void GameManager::onEnter() {
    // A ativação é rastreada aqui (não na cena): compara o código do estado
    // atual com o último ativado; o registro é limpo no commit da navegação.
    const std::string code = m_routerService->currentState().getCode();
    if (code != m_enteredStateCode) {
        m_routerService->currentScene().onEnter();
        m_enteredStateCode = code;
    }
}

void GameManager::render() {
    core::IScene& scene = m_routerService->currentScene();

    scene.draw();
}

void GameManager::input() {
    core::IScene& scene = m_routerService->currentScene();

    scene.input();
}

void GameManager::update(const core::Seconds dt) {
    core::IScene& scene = m_routerService->currentScene();

    scene.update(dt);
}

void GameManager::onExit() {
    if (!m_routerService->hasPendingStateChange()) {
        return;
    }

    // A referência à cena atual NÃO pode sobreviver ao commit: commitStateChange()
    // descarrega a cena (unloadScene -> erase no mapa), invalidando qualquer
    // IScene& obtida aqui. Isolamos a referência em um escopo próprio para
    // garantir, por construção, que ela morre antes do commit — evitando
    // use-after-free se alguém acrescentar código nesta função no futuro.
    // Ver .ai/task/06-scene-lifetime.md.
    {
        core::IScene& scene = m_routerService->currentScene();
        scene.onExit();
    }

    m_routerService->commitStateChange();

    // A navegação foi efetivada: a próxima iteração deve ativar a nova cena,
    // mesmo que o código do estado se repita (ex.: A -> B -> A).
    m_enteredStateCode.clear();
}

bool GameManager::shouldExit() const {
    return m_routerService->currentState().getCode() == kExitStateCode;
}

void GameManager::cleanup() {
}

} // namespace cengine::routing
