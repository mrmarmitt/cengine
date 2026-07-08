#pragma once

#include <cengine/core/Time.hpp>

namespace cengine::core {

/**
 * @brief Ponte entre o game loop da engine e as regras/telas do jogo.
 *
 * O `EngineManager` conhece apenas esta interface; toda a lógica de "qual cena
 * está ativa e o que ela faz" fica do lado do jogo (por exemplo, na
 * implementação `cengine::routing::GameManager`, que delega para o roteador).
 *
 * Os métodos abaixo são chamados a cada iteração do loop, na ordem:
 * `onEnter()` → `input()` → `update(dt)` (0..N vezes, passo fixo) →
 * `render()` → `onExit()`, e por fim `shouldExit()` decide se o loop
 * continua. `cleanup()` roda uma vez, ao encerrar.
 */
class IGameManager {
public:
    virtual ~IGameManager() = default;

    /// Ativa/entra na cena atual. Deve ser idempotente por ativação: chamado
    /// toda iteração, mas só efetiva a entrada uma vez por cena ativada (a
    /// contabilidade é do implementador, não da cena — ver IScene).
    virtual void onEnter() = 0;

    /// Renderiza a cena atual.
    virtual void render() = 0;

    /// Processa a entrada do usuário na cena atual.
    virtual void input() = 0;

    /// Avança a simulação em @p dt. Contrato de *fixed timestep*: o loop chama
    /// 0..N vezes por iteração, sempre com o MESMO dt (ver EngineManager).
    virtual void update(Seconds dt) = 0;

    /// Trata a saída da cena atual e efetiva uma eventual troca de estado.
    virtual void onExit() = 0;

    /// Libera recursos do jogo. Chamado uma vez, no encerramento da engine.
    virtual void cleanup() = 0;

    /// @return true quando o jogo pediu para encerrar; para o game loop.
    [[nodiscard]] virtual bool shouldExit() const = 0;
};

} // namespace cengine::core
