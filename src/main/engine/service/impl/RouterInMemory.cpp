#include "RouterInMemory.hpp"

#include <utility>

RouterInMemory::RouterInMemory(std::shared_ptr<ISceneRepository> sceneRepository) : m_sceneRepository(std::move(sceneRepository)) {}

void RouterInMemory::setNextState(std::unique_ptr<IState> state) const {
    m_sceneRepository->persistNextState(std::move(state));
}

IState &RouterInMemory::getCurrentStateGame() const {
    return m_sceneRepository->getCurrentStateGame();
}

std::string RouterInMemory::getCurrentStateGameName() const {
    return m_sceneRepository->getCurrentStateGame().getName();
}

std::string RouterInMemory::getCurrentStateGameCode() const {
    return m_sceneRepository->getCurrentStateGame().getCode();
}

IScene &RouterInMemory::getCurrentCachedScreen() const {
    return m_sceneRepository->getScene(getCurrentStateGameCode());
}

IState &RouterInMemory::getNextStateGame() const {
    return m_sceneRepository->getNextStateGame();
}

std::string RouterInMemory::getNextStateGameName() const {
    return m_sceneRepository->getNextStateGame().getName();
}

std::string RouterInMemory::getNextStateGameCode() const {
    return m_sceneRepository->getNextStateGame().getCode();
}

IScene &RouterInMemory::getNextCachedScreen() const {
    return m_sceneRepository->getScene(getNextStateGameCode());
}

bool RouterInMemory::hasNextScreen() const {
    return m_sceneRepository->isNextStateEqualsToCurrentScene();
}

void RouterInMemory::goToNextScreen() const {
    m_sceneRepository->unloadScene(getCurrentStateGameCode());
    m_sceneRepository->persisteCurrentState();
}




