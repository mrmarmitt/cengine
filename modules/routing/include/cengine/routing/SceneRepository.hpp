#pragma once

#include <functional>
#include <memory>
#include <stdexcept>
#include <unordered_map>

#include <cengine/core/IScene.hpp>
#include <cengine/routing/IState.hpp>
#include <cengine/routing/ISceneRepository.hpp>

namespace cengine::routing {

class SceneRepository final : public ISceneRepository {
    std::unordered_map<std::string, std::unique_ptr<core::IScene>> m_scenes;
    std::unordered_map<std::string, std::function<std::unique_ptr<core::IScene>()>> m_factories;

    std::unique_ptr<IState> m_nextState;
    std::unique_ptr<IState> m_currentState;

public:
    explicit SceneRepository(std::unique_ptr<IState> initialState);
    ~SceneRepository() override = default;

    void registerFactory(const std::string& name, std::function<std::unique_ptr<core::IScene>()> factory) override;

    [[nodiscard]] IState& getCurrentStateGame() const override;

    [[nodiscard]] IState& getNextStateGame() const override;

    void persisteCurrentState() override;

    void persistNextState(std::unique_ptr<IState> state) override;

    core::IScene& getScene(const std::string& name) override;

    void unloadScene(const std::string& name) override;

    void unloadAll() override;

    bool hasPendingStateChange() const override;
};

} // namespace cengine::routing
