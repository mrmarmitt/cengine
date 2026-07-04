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

    void setNextState(std::unique_ptr<IState> state) const override;

    [[nodiscard]] IState& getCurrentStateGame() const override;
    [[nodiscard]] std::string getCurrentStateGameName() const override;
    [[nodiscard]] std::string getCurrentStateGameCode() const override;
    [[nodiscard]] IScene& getCurrentCachedScreen() const override;

    [[nodiscard]] IState& getNextStateGame() const override;
    [[nodiscard]] std::string getNextStateGameName() const override;
    [[nodiscard]] std::string getNextStateGameCode() const override;
    [[nodiscard]] IScene& getNextCachedScreen() const override;

    [[nodiscard]] bool hasNextScreen() const override;

    void goToNextScreen() const override;
};
