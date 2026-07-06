#include <cengine/routing/GameManager.hpp>

#include <cengine/core/IScene.hpp>

#include <utility>

#include <cengine/routing/IState.hpp>
#include <cengine/routing/IRouter.hpp>
#include <cengine/routing/StateCodes.hpp>

namespace cengine::routing {

GameManager::GameManager(std::shared_ptr<IRouter> routerService) : m_routerService(std::move(routerService)){}

void GameManager::onEnter() {
    if (core::IScene& scene = m_routerService->currentScene(); !scene.isOnEnterExecuted()) {
        scene.onEnter();
        scene.onEnterExecuted();
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
}

bool GameManager::shouldExit() const {
    return m_routerService->currentState().getCode() == kExitStateCode;
}

void GameManager::cleanup() {
}

} // namespace cengine::routing
