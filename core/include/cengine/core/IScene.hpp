#pragma once

namespace cengine::core {

class IScene {
public:
    IScene() = default;
    virtual ~IScene() = default;

    virtual void onEnter() = 0;
    virtual void onEnterExecuted() = 0;
    [[nodiscard]] virtual bool isOnEnterExecuted() const = 0;

    virtual void draw() = 0;
    virtual void input() = 0;
    virtual void onExit() = 0;
};

} // namespace cengine::core
