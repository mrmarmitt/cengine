#pragma once

#include <memory>

#include <cengine/core/IGameManager.hpp>
#include <cengine/routing/IRouter.hpp>

namespace cengine::routing {

class GameManager : public core::IGameManager {
    std::shared_ptr<IRouter> m_routerService;
public:
    explicit GameManager(std::shared_ptr<IRouter> routerService);

    void onEnter() override;
    void render() override;
    void input() override;
    void onExit() override;

    [[nodiscard]] bool shouldExit() const override;
    void cleanup() override;
};

} // namespace cengine::routing
