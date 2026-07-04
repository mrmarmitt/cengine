#include <cengine/routing/GameManager.hpp>

#include <cengine/core/IScene.hpp>

#include <iostream>
#include <utility>

#include <cengine/routing/IRouter.hpp>

namespace cengine::routing {

GameManager::GameManager(std::shared_ptr<IRouter> routerService) : m_routerService(std::move(routerService)){}

void GameManager::onEnter() {
    if (core::IScene& screen = m_routerService->getCurrentCachedScreen(); !screen.isOnEnterExecuted()) {
        screen.onEnter();
        screen.onEnterExecuted();
    }
}

void GameManager::render() {
    core::IScene& screen = m_routerService->getCurrentCachedScreen();

    screen.draw();
}

void GameManager::input() {
    core::IScene& screen = m_routerService->getCurrentCachedScreen();

    screen.input();
}

void GameManager::onExit() {
    if (m_routerService->hasNextScreen()) {
        core::IScene& screen = m_routerService->getCurrentCachedScreen();
        screen.onExit();
        m_routerService->goToNextScreen();
    }
}

bool GameManager::shouldExist() const {
    return m_routerService->getCurrentStateGameCode() == "exit";
}

void GameManager::cleanup() {
    // std::println("");
    std::cout << "TerminalGameManager: cleanup" << std::endl;
}

} // namespace cengine::routing
