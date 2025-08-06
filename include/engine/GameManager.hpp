#pragma once
#include <memory>

class RouterService;

class GameManager final {
    std::shared_ptr<RouterService> m_routerService;
public:
    explicit GameManager(std::shared_ptr<RouterService> routerService);
    ~GameManager();

    void onEnter() const;
    void render() const;
    void input() const;
    void onExit() const;

    [[nodiscard]] bool shouldExist() const;
    static void cleanup();
};