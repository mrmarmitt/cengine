#pragma once
#include <memory>
#include <string>

#include <engine/IState.hpp>
#include "repository/ISceneRepository.hpp"

class IScene;
class IState;

class IRouter {
public:
    virtual ~IRouter() = default;

    virtual void setNextState(std::unique_ptr<IState> state) const = 0;

    [[nodiscard]] virtual IState& getCurrentStateGame() const = 0;
    [[nodiscard]] virtual std::string getCurrentStateGameName() const = 0;
    [[nodiscard]] virtual std::string getCurrentStateGameCode() const = 0;
    [[nodiscard]] virtual IScene& getCurrentCachedScreen() const = 0;

    [[nodiscard]] virtual IState& getNextStateGame() const = 0;
    [[nodiscard]] virtual std::string getNextStateGameName() const = 0;
    [[nodiscard]] virtual std::string getNextStateGameCode() const = 0;
    [[nodiscard]] virtual IScene& getNextCachedScreen() const = 0;

    [[nodiscard]] virtual bool hasNextScreen() const = 0;

    virtual void goToNextScreen() const = 0;
};
