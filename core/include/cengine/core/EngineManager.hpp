#pragma once

#include <memory>

#include <cengine/core/IGameManager.hpp>
#include <cengine/core/IWindowManager.hpp>

namespace cengine::core {

class EngineManager {
    void run();
    std::unique_ptr<IWindowManager> m_windowManager;
    std::unique_ptr<IGameManager> m_gameManager;
    bool m_isRunning;

public:
    EngineManager(
        std::unique_ptr<IWindowManager> windowManager,
        std::unique_ptr<IGameManager> gameManager);

    ~EngineManager() = default;

    void start();
    void cleanup() const;
};

} // namespace cengine::core
