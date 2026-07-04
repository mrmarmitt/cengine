#pragma once
#include <memory>

#include <cengine/core/IScene.hpp>
#include <cengine/routing/IState.hpp>
#include <cengine/routing/IRouter.hpp>
#include <cengine/routing/ISceneRepository.hpp>

namespace cengine::routing {

class RouterInMemory final : public IRouter {
    std::shared_ptr<ISceneRepository> m_sceneRepository;

public:
    explicit RouterInMemory(std::shared_ptr<ISceneRepository> sceneRepository);
    ~RouterInMemory() override = default;

    void requestState(std::unique_ptr<IState> state) override;
    [[nodiscard]] bool hasPendingStateChange() const override;
    void commitStateChange() override;

    [[nodiscard]] const IState& currentState() const override;
    [[nodiscard]] core::IScene& currentScene() override;
};

} // namespace cengine::routing
