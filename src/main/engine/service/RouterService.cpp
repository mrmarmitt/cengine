#include "RouterService.hpp"

#include <utility>

RouterService::RouterService(std::shared_ptr<SceneRepository> sceneRepository) : m_sceneRepository(std::move(sceneRepository)) {}

void RouterService::setNextState(std::unique_ptr<IState> state) const {
    m_sceneRepository->persistNextState(std::move(state));
}

IState &RouterService::getCurrentStateGame() const {
    return m_sceneRepository->getCurrentStateGame();
}

std::string RouterService::getCurrentStateGameName() const {
    return m_sceneRepository->getCurrentStateGame().getName();
}

std::string RouterService::getCurrentStateGameCode() const {
    return m_sceneRepository->getCurrentStateGame().getCode();
}

IScene &RouterService::getCurrentCachedScreen() const {
    return m_sceneRepository->getScene(getCurrentStateGameCode());
}

IState &RouterService::getNextStateGame() const {
    return m_sceneRepository->getNextStateGame();
}

std::string RouterService::getNextStateGameName() const {
    return m_sceneRepository->getNextStateGame().getName();
}

std::string RouterService::getNextStateGameCode() const {
    return m_sceneRepository->getNextStateGame().getCode();
}

IScene &RouterService::getNextCachedScreen() const {
    return m_sceneRepository->getScene(getNextStateGameCode());
}

bool RouterService::hasNextScreen() const {
    return m_sceneRepository->isNextStateEqualsToCurrentScene();
}

void RouterService::goToNextScreen() const {
    m_sceneRepository->unloadScene(getCurrentStateGameCode());
    m_sceneRepository->persisteCurrentState();
}




