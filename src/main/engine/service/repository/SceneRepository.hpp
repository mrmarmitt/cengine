#pragma once

#include <functional>
#include <memory>
#include <stdexcept>
#include <unordered_map>

#include "../../../../../include/engine/IScene.hpp"
#include "ISceneRepository.hpp" 

class IState;

class SceneRepository final : public ISceneRepository {
    std::unordered_map<std::string, std::unique_ptr<IScene>> m_scenes;
    std::unordered_map<std::string, std::function<std::unique_ptr<IScene>()>> m_factories;

    std::unique_ptr<IState> m_nextState;
    std::unique_ptr<IState> m_currentState;

public:
    explicit SceneRepository(std::unique_ptr<IState> initialState);
    ~SceneRepository() override = default;

    void registerFactory(const std::string& name, std::function<std::unique_ptr<IScene>()> factory) override;

    [[nodiscard]] IState& getCurrentStateGame() const override;

    [[nodiscard]] IState& getNextStateGame() const override;

    void persisteCurrentState() override;

    void persistNextState(std::unique_ptr<IState> state) override;

    IScene& getScene(const std::string& name) override;

    void unloadScene(const std::string& name) override;

    void unloadAll() override;

    bool isNextStateEqualsToCurrentScene() const override;
};
