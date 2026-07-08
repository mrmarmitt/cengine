#pragma once
#include <memory>

namespace cengine::core {
class IScene;
}

namespace cengine::routing {

class IState;

/**
 * @brief Orquestra a navegação entre estados/telas do jogo.
 *
 * O roteamento é um **módulo opcional** (`cengine::routing`), fora do núcleo da
 * engine: um jogo pode usá-lo ou fornecer sua própria navegação. Ver
 * ADR 0001 em `.ai/decisions/`.
 *
 * A troca de estado é em duas fases (para não trocar de cena no meio de uma
 * iteração do loop): `requestState()` agenda o próximo estado e
 * `commitStateChange()` o efetiva — normalmente ao final de `onExit()`.
 */
class IRouter {
public:
    virtual ~IRouter() = default;

    /// Agenda a transição para @p state (efetivada depois, em commitStateChange).
    /// Solicitar o código do estado ATUAL é válido e efetiva um reload: a cena
    /// é descarregada, recriada e reativada (`onEnter` roda de novo).
    virtual void requestState(std::unique_ptr<IState> state) = 0;

    /// @return true se há uma troca de estado agendada e ainda não efetivada.
    [[nodiscard]] virtual bool hasPendingStateChange() const = 0;

    /// Efetiva a troca agendada: descarrega a cena atual e promove a próxima.
    virtual void commitStateChange() = 0;

    /// @return o estado atualmente ativo.
    [[nodiscard]] virtual const IState& currentState() const = 0;

    // Contrato de tempo de vida: a referência retornada é válida apenas até a
    // próxima navegação (commitStateChange), que pode descarregar a cena atual.
    // NÃO retenha esta referência através de uma troca de estado — obtenha-a
    // novamente após navegar. Ver .ai/task/06-scene-lifetime.md.
    [[nodiscard]] virtual core::IScene& currentScene() = 0;
};

} // namespace cengine::routing
