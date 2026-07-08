#include <cengine/routing/RouterInMemory.hpp>

#include <utility>

namespace cengine::routing {

RouterInMemory::RouterInMemory(std::unique_ptr<ISceneRepository> sceneRepository,
                               std::unique_ptr<IState> initialState)
    : m_sceneRepository(std::move(sceneRepository)),
      m_currentState(std::move(initialState)) {}

void RouterInMemory::requestState(std::unique_ptr<IState> state) {
    m_nextState = std::move(state);
}

bool RouterInMemory::hasPendingStateChange() const {
    return m_nextState != nullptr;
}

void RouterInMemory::commitStateChange() {
    if (!m_nextState) {
        return;
    }

    m_sceneRepository->unloadScene(m_currentState->getCode());
    m_currentState = std::move(m_nextState);
}

const IState& RouterInMemory::currentState() const {
    return *m_currentState;
}

core::IScene& RouterInMemory::currentScene() {
    return m_sceneRepository->getScene(m_currentState->getCode());
}

} // namespace cengine::routing
