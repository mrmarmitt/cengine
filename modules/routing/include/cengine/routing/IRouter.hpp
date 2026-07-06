#pragma once
#include <memory>

namespace cengine::core {
class IScene;
}

namespace cengine::routing {

class IState;

class IRouter {
public:
    virtual ~IRouter() = default;

    virtual void requestState(std::unique_ptr<IState> state) = 0;
    [[nodiscard]] virtual bool hasPendingStateChange() const = 0;
    virtual void commitStateChange() = 0;

    [[nodiscard]] virtual const IState& currentState() const = 0;

    // Contrato de tempo de vida: a referência retornada é válida apenas até a
    // próxima navegação (commitStateChange), que pode descarregar a cena atual.
    // NÃO retenha esta referência através de uma troca de estado — obtenha-a
    // novamente após navegar. Ver .ai/task/06-scene-lifetime.md.
    [[nodiscard]] virtual core::IScene& currentScene() = 0;
};

} // namespace cengine::routing
