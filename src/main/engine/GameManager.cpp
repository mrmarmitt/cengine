#include <engine/GameManager.hpp>
#include <engine/IScene.hpp>

#include <iostream>
#include <utility>

#include "service/RouterService.hpp"

GameManager::GameManager(std::shared_ptr<RouterService> routerService) : m_routerService(std::move(routerService)){}
GameManager::~GameManager() = default;

void GameManager::onEnter() const {
    if (IScene& screen = m_routerService->getCurrentCachedScreen(); !screen.isOnEnterExecuted()) {
        screen.onEnter();
        screen.onEnterExecuted();
    }
}

void GameManager::render() const {
    IScene& screen = m_routerService->getCurrentCachedScreen();

    screen.draw();
}

void GameManager::input() const {
    IScene& screen = m_routerService->getCurrentCachedScreen();

    screen.input();
}

void GameManager::onExit() const {
    if (m_routerService->hasNextScreen()) {
        IScene& screen = m_routerService->getCurrentCachedScreen();
        screen.onExit();
        m_routerService->goToNextScreen();
    }
}

bool GameManager::shouldExist() const {
    return m_routerService->getCurrentStateGameCode() == "exit";
}

void GameManager::cleanup() {
    std::cout << "TerminalGameManager: cleanup" << std::endl;
}


