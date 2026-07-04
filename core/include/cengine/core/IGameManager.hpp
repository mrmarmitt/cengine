#pragma once

namespace cengine::core {

class IGameManager {
public:
    virtual ~IGameManager() = default;

    virtual void onEnter() = 0;
    virtual void render() = 0;
    virtual void input() = 0;
    virtual void onExit() = 0;
    virtual void cleanup() = 0;

    [[nodiscard]] virtual bool shouldExit() const = 0;
};

} // namespace cengine::core
