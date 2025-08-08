#pragma once

#include <engine/IGameManager.hpp>
#include "service/IRouter.hpp"

class GameManager : public IGameManager {
    std::shared_ptr<IRouter> m_routerService;
public:
    explicit GameManager(std::shared_ptr<IRouter> routerService);

    void onEnter() override;
    void render() override;
    void input() override;
    void onExit() override;

    [[nodiscard]] bool shouldExist() const override;
    void cleanup();
};
