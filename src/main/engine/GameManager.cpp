#include "GameManager.hpp"

#include <engine/IScene.hpp>

#include <iostream>
#include <utility>

#include "service/IRouter.hpp"

GameManager::GameManager(std::shared_ptr<IRouter> routerService) : m_routerService(std::move(routerService)){}

void GameManager::onEnter() {
    if (IScene& screen = m_routerService->getCurrentCachedScreen(); !screen.isOnEnterExecuted()) {
        screen.onEnter();
        screen.onEnterExecuted();
    }
}

void GameManager::render() {
    IScene& screen = m_routerService->getCurrentCachedScreen();

    screen.draw();
}

void GameManager::input() {
    IScene& screen = m_routerService->getCurrentCachedScreen();

    screen.input();
}

void GameManager::onExit() {
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
    // std::println("");
    std::cout << "TerminalGameManager: cleanup" << std::endl;
}


