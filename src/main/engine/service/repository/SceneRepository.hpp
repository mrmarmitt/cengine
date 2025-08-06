#pragma once

#include <functional>
#include <memory>
#include <stdexcept>
#include <unordered_map>

#include "../../../../../include/engine/IScene.hpp"

class IState;

class SceneRepository {
    std::unordered_map<std::string, std::unique_ptr<IScene>> m_scenes;
    std::unordered_map<std::string, std::function<std::unique_ptr<IScene>()>> m_factories;

    std::unique_ptr<IState> m_nextState;
    std::unique_ptr<IState> m_currentState;

public:
    explicit SceneRepository(const std::unique_ptr<IState> &initialState);
    ~SceneRepository() = default;

    void registerFactory(const std::string& name, std::function<std::unique_ptr<IScene>()> factory);

    [[nodiscard]] IState& getCurrentStateGame() const;

    [[nodiscard]] IState& getNextStateGame() const;

    void persisteCurrentState();

    void persistNextState(std::unique_ptr<IState> state);

    IScene& getScene(const std::string& name);

    void unloadScene(const std::string& name);

    void unloadAll();

    bool isNextStateEqualsToCurrentScene() const;
};