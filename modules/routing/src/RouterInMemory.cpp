#include <cengine/routing/RouterInMemory.hpp>

#include <string>
#include <utility>

namespace cengine::routing {

RouterInMemory::RouterInMemory(std::shared_ptr<ISceneRepository> sceneRepository) : m_sceneRepository(std::move(sceneRepository)) {}

void RouterInMemory::requestState(std::unique_ptr<IState> state) {
    m_sceneRepository->persistNextState(std::move(state));
}

bool RouterInMemory::hasPendingStateChange() const {
    return m_sceneRepository->hasPendingStateChange();
}

void RouterInMemory::commitStateChange() {
    const std::string currentStateCode = m_sceneRepository->getCurrentStateGame().getCode();
    m_sceneRepository->unloadScene(currentStateCode);
    m_sceneRepository->persisteCurrentState();
}

const IState& RouterInMemory::currentState() const {
    return m_sceneRepository->getCurrentStateGame();
}

core::IScene& RouterInMemory::currentScene() {
    return m_sceneRepository->getScene(currentState().getCode());
}

} // namespace cengine::routing
