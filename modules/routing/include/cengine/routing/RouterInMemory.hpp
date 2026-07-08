#pragma once
#include <memory>

#include <cengine/core/IScene.hpp>
#include <cengine/routing/IState.hpp>
#include <cengine/routing/IRouter.hpp>
#include <cengine/routing/ISceneRepository.hpp>

namespace cengine::routing {

/**
 * @brief `IRouter` em memória: dono da máquina de estados (atual/próximo),
 *        apoiado em um `ISceneRepository` que apenas provê cenas.
 *
 * O router assume a posse exclusiva do repositório: o jogo registra as
 * factories e transfere o `unique_ptr`, de modo que nenhum componente externo
 * pode descarregar cenas por fora do ciclo de navegação (garante por
 * construção o contrato de ativação do `GameManager` — ver .ai/task/13).
 *
 * `requestState()` agenda a troca; `commitStateChange()` descarrega a cena do
 * estado que sai e promove o próximo. Solicitar o código do estado ATUAL
 * também é uma troca válida: o commit descarrega e recria a cena (reload
 * deliberado).
 */
class RouterInMemory final : public IRouter {
    std::unique_ptr<ISceneRepository> m_sceneRepository;
    std::unique_ptr<IState> m_currentState;
    std::unique_ptr<IState> m_nextState; // nullptr = nenhuma troca pendente

public:
    /// @param sceneRepository provedor de cenas (posse transferida ao router).
    /// @param initialState    estado ativo no início do loop.
    RouterInMemory(std::unique_ptr<ISceneRepository> sceneRepository,
                   std::unique_ptr<IState> initialState);
    ~RouterInMemory() override = default;

    void requestState(std::unique_ptr<IState> state) override;
    [[nodiscard]] bool hasPendingStateChange() const override;
    void commitStateChange() override;

    [[nodiscard]] const IState& currentState() const override;
    [[nodiscard]] core::IScene& currentScene() override;
};

} // namespace cengine::routing
