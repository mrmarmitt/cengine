#include <cengine/routing/SceneRepository.hpp>

namespace cengine::routing {

void SceneRepository::registerFactory(const std::string &name, std::function<std::unique_ptr<core::IScene>()> factory)
{
    m_factories[name] = std::move(factory);
}

core::IScene &SceneRepository::getScene(const std::string &name) {
    // Se já estiver instanciada, retorna
    if (const auto it = m_scenes.find(name); it != m_scenes.end()) {
        return *(it->second);
    }

    // Caso contrário, instancia sob demanda via factory
    if (const auto factoryIt = m_factories.find(name); factoryIt != m_factories.end()) {
        const auto [it, inserted] = m_scenes.emplace(name, factoryIt->second());
        return *(it->second);
    }

    throw std::runtime_error("Scene not found: " + name);
}

void SceneRepository::unloadScene(const std::string &name) {
    m_scenes.erase(name);
}

void SceneRepository::unloadAll() {
    m_scenes.clear();
}

} // namespace cengine::routing
