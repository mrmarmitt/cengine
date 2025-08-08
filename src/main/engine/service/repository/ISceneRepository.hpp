#pragma once
#include <memory>
#include <string>
#include <functional>

class IState;
class IScene;

class ISceneRepository {
public:
    virtual ~ISceneRepository() = default;

    virtual void registerFactory(const std::string& name, std::function<std::unique_ptr<IScene>()> factory) = 0;
    virtual IState& getCurrentStateGame() const = 0;
    virtual IState& getNextStateGame() const = 0;
    virtual void persisteCurrentState() = 0;
    virtual void persistNextState(std::unique_ptr<IState> state) = 0;
    virtual IScene& getScene(const std::string& name) = 0;
    virtual void unloadScene(const std::string& name) = 0;
    virtual void unloadAll() = 0;
    virtual bool isNextStateEqualsToCurrentScene() const = 0;
};
