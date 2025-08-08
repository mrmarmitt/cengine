#pragma once

class IScene {
    // bool m_isEnterExecuted = false;
public:
    IScene() = default;
    virtual ~IScene() = default;

    virtual void onEnter() = 0;
    // virtual void onEnterExecuted() { m_isEnterExecuted = true; }
    // [[nodiscard]] virtual bool isOnEnterExecuted() const { return m_isEnterExecuted; }
    virtual void onEnterExecuted() = 0;
    [[nodiscard]] virtual bool isOnEnterExecuted() const = 0;

    virtual void draw() = 0;
    virtual void input() = 0;
    virtual void onExit() = 0;
};
