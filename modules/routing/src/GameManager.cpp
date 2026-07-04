#include <cengine/routing/GameManager.hpp>

#include <cengine/core/IScene.hpp>

#include <iostream>
#include <utility>

#include <cengine/routing/IState.hpp>
#include <cengine/routing/IRouter.hpp>

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
    if (m_routerService->hasPendingStateChange()) {
        core::IScene& scene = m_routerService->currentScene();
        scene.onExit();
        m_routerService->commitStateChange();
    }
}

bool GameManager::shouldExist() const {
    return m_routerService->currentState().getCode() == "exit";
}

void GameManager::cleanup() {
    // std::println("");
    std::cout << "TerminalGameManager: cleanup" << std::endl;
}

} // namespace cengine::routing
