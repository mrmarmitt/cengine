#include <cengine/routing/SceneRepository.hpp>

namespace cengine::routing {

SceneRepository::SceneRepository(std::unique_ptr<IState> initialState){
    m_nextState = initialState->clone();
    m_currentState = initialState->clone();
}

void SceneRepository::registerFactory(const std::string &name, std::function<std::unique_ptr<core::IScene>()> factory)
{
    m_factories[name] = std::move(factory);
}

IState &SceneRepository::getCurrentStateGame() const {
    return *m_currentState;
}

IState &SceneRepository::getNextStateGame() const {
    return *m_nextState;
}

void SceneRepository::persisteCurrentState() {
    m_currentState = m_nextState->clone();
}

void SceneRepository::persistNextState(std::unique_ptr<IState> state) {
    m_nextState = std::move(state);
}

core::IScene &SceneRepository::getScene(const std::string &name) {
    // Se já estiver instanciada, retorna
    const auto it = m_scenes.find(name);
    if (it != m_scenes.end()) {
        return *(it->second);
    }

    // Caso contrário, instancia sob demanda via factory
    const auto factoryIt = m_factories.find(name);
    if (factoryIt != m_factories.end()) {
        m_scenes[name] = factoryIt->second();
        return *(m_scenes[name]);
    }

    throw std::runtime_error("Scene not found: " + name);
}

void SceneRepository::unloadScene(const std::string &name) {
    m_scenes.erase(name);
}

void SceneRepository::unloadAll() {
    m_scenes.clear();
}

bool SceneRepository::hasPendingStateChange() const {
    return m_nextState->getCode() != m_currentState->getCode();
}

} // namespace cengine::routing
