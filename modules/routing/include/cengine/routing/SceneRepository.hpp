#pragma once

#include <functional>
#include <memory>
#include <stdexcept>
#include <unordered_map>

#include <cengine/core/IScene.hpp>
#include <cengine/routing/ISceneRepository.hpp>

namespace cengine::routing {

/**
 * @brief `ISceneRepository` em memória: mapa de factories + cache de cenas
 *        instanciadas.
 *
 * `getScene()` instancia a cena via factory na primeira vez e a mantém em cache
 * até um `unloadScene`/`unloadAll`. Puro provisionamento — nenhuma noção de
 * estado/navegação (essa é do `RouterInMemory`, dono deste repositório).
 */
class SceneRepository final : public ISceneRepository {
    std::unordered_map<std::string, std::unique_ptr<core::IScene>> m_scenes;
    std::unordered_map<std::string, std::function<std::unique_ptr<core::IScene>()>> m_factories;

public:
    SceneRepository() = default;
    ~SceneRepository() override = default;

    void registerFactory(const std::string& name, std::function<std::unique_ptr<core::IScene>()> factory) override;

    core::IScene& getScene(const std::string& name) override;

    void unloadScene(const std::string& name) override;

    void unloadAll() override;
};

} // namespace cengine::routing
