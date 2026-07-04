#pragma once
#include <memory>

namespace cengine::core {
class IScene;
}

namespace cengine::routing {

class IState;

class IRouter {
public:
    virtual ~IRouter() = default;

    virtual void requestState(std::unique_ptr<IState> state) = 0;
    [[nodiscard]] virtual bool hasPendingStateChange() const = 0;
    virtual void commitStateChange() = 0;

    [[nodiscard]] virtual const IState& currentState() const = 0;
    [[nodiscard]] virtual core::IScene& currentScene() = 0;
};

} // namespace cengine::routing
