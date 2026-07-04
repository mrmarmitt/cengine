#pragma once
#include <memory>
#include <string>

#include <cengine/core/IScene.hpp>
#include <cengine/routing/IState.hpp>
#include <cengine/routing/ISceneRepository.hpp>

namespace cengine::routing {

class IRouter {
public:
    virtual ~IRouter() = default;

    virtual void setNextState(std::unique_ptr<IState> state) const = 0;

    [[nodiscard]] virtual IState& getCurrentStateGame() const = 0;
    [[nodiscard]] virtual std::string getCurrentStateGameName() const = 0;
    [[nodiscard]] virtual std::string getCurrentStateGameCode() const = 0;
    [[nodiscard]] virtual core::IScene& getCurrentCachedScreen() const = 0;

    [[nodiscard]] virtual IState& getNextStateGame() const = 0;
    [[nodiscard]] virtual std::string getNextStateGameName() const = 0;
    [[nodiscard]] virtual std::string getNextStateGameCode() const = 0;
    [[nodiscard]] virtual core::IScene& getNextCachedScreen() const = 0;

    [[nodiscard]] virtual bool hasNextScreen() const = 0;

    virtual void goToNextScreen() const = 0;
};

} // namespace cengine::routing
