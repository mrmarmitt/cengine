#pragma once
#include <memory>
#include <string>

#include <engine/IState.hpp>
#include "../IRouter.hpp"

class IScene;
class IState;

class RouterInMemory final : public IRouter {
    std::shared_ptr<ISceneRepository> m_sceneRepository;

public:
    explicit RouterInMemory(std::shared_ptr<ISceneRepository> sceneRepository);
    ~RouterInMemory() override = default;

    void setNextState(std::unique_ptr<IState> state) const;

    [[nodiscard]] IState& getCurrentStateGame() const;
    [[nodiscard]] std::string getCurrentStateGameName() const;
    [[nodiscard]] std::string getCurrentStateGameCode() const;
    [[nodiscard]] IScene& getCurrentCachedScreen() const;

    [[nodiscard]] IState& getNextStateGame() const;
    [[nodiscard]] std::string getNextStateGameName() const;
    [[nodiscard]] std::string getNextStateGameCode() const;
    [[nodiscard]] IScene& getNextCachedScreen() const;

    [[nodiscard]] bool hasNextScreen() const;

    void goToNextScreen() const;
};
