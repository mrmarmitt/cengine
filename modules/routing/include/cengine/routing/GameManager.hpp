#pragma once

#include <memory>
#include <string>

#include <cengine/core/IGameManager.hpp>
#include <cengine/routing/IRouter.hpp>

namespace cengine::routing {

/**
 * @brief Implementação de `core::IGameManager` baseada em roteamento de cenas.
 *
 * Liga o game loop da engine (que só conhece `IGameManager`) ao `IRouter`:
 * cada callback do loop é delegado à cena atual obtida do roteador, e `onExit()`
 * efetiva uma eventual troca de estado pendente. `shouldExit()` compara o estado
 * atual com `kExitStateCode`.
 *
 * A contabilidade "a cena atual já recebeu `onEnter()`?" é interna a esta
 * classe (código do estado ativado, resetado no commit da navegação) — a
 * `IScene` não carrega flag de ativação. Ver .ai/task/12.
 */
class GameManager : public core::IGameManager {
    std::shared_ptr<IRouter> m_routerService;
    std::string m_enteredStateCode; // vazio = nenhuma cena ativada nesta rota
public:
    explicit GameManager(std::shared_ptr<IRouter> routerService);

    void onEnter() override;
    void render() override;
    void input() override;
    void update(core::Seconds dt) override;
    void onExit() override;

    [[nodiscard]] bool shouldExit() const override;
    void cleanup() override;
};

} // namespace cengine::routing
