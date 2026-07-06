#pragma once
#include <memory>
#include <string>
#include <functional>

#include <cengine/core/IScene.hpp>
#include <cengine/routing/IState.hpp>

namespace cengine::routing {

class ISceneRepository {
public:
    virtual ~ISceneRepository() = default;

    virtual void registerFactory(const std::string& name, std::function<std::unique_ptr<core::IScene>()> factory) = 0;
    virtual IState& getCurrentStateGame() const = 0;
    virtual IState& getNextStateGame() const = 0;
    virtual void persisteCurrentState() = 0;
    virtual void persistNextState(std::unique_ptr<IState> state) = 0;
    // Contrato de tempo de vida: a referência retornada aponta para dentro do
    // mapa interno de cenas e é invalidada por unloadScene(name)/unloadAll().
    // NÃO retenha a referência através de um desses unloads. Ver
    // .ai/task/06-scene-lifetime.md.
    virtual core::IScene& getScene(const std::string& name) = 0;
    virtual void unloadScene(const std::string& name) = 0;
    virtual void unloadAll() = 0;
    virtual bool hasPendingStateChange() const = 0;
};

} // namespace cengine::routing
