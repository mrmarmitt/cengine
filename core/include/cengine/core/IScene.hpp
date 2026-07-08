#pragma once

#include <cengine/core/Time.hpp>

namespace cengine::core {

/**
 * @brief Uma tela/estado renderizável do jogo (menu, gameplay, game over...).
 *
 * O jogo implementa esta interface; a engine apenas orquestra o ciclo de vida.
 * A engine é agnóstica de biblioteca gráfica: o que `draw()`/`input()` fazem por
 * baixo (SDL, Raylib, terminal...) é decisão do jogo.
 *
 * ## Ciclo de vida (por iteração do game loop)
 * A cada volta do loop o gerenciador de jogo chama, nesta ordem:
 * `onEnter()` (só na primeira vez em que a cena é ativada) → `input()` →
 * `update(dt)` (0..N vezes, passo fixo) → `draw()`. Ao trocar de cena, a cena
 * que sai recebe `onExit()`.
 *
 * A garantia de que `onEnter()` roda **uma única vez** por ativação é do
 * orquestrador (ex.: `cengine::routing::GameManager`), não da cena — a
 * implementação não precisa (nem deve) manter flag de "já ativada".
 *
 * @note Contrato de tempo de vida: referências a uma cena obtidas via
 *       `cengine::routing::IRouter::currentScene()` só valem até a próxima
 *       navegação. Nunca retenha uma `IScene&` através de uma troca de estado.
 */
class IScene {
public:
    virtual ~IScene() = default;

    /// Inicialização única da cena (carregar recursos, montar objetos).
    /// Chamado uma vez por ativação, antes do primeiro `input()`/`draw()`.
    virtual void onEnter() = 0;

    /// Avança a simulação da cena em @p dt (animações, timers, física...).
    /// Contrato de *fixed timestep*: chamado **0..N vezes por iteração** do
    /// loop, sempre com o MESMO dt — a cena não deve assumir 1 chamada por
    /// quadro nem medir tempo por conta própria.
    virtual void update(Seconds dt) = 0;

    /// Desenha o quadro atual. Chamado toda iteração enquanto a cena está ativa.
    virtual void draw() = 0;

    /// Processa entrada do usuário. Chamado toda iteração, antes de `update()`.
    virtual void input() = 0;

    /// Finalização da cena (liberar recursos). Chamado ao sair/trocar de cena.
    virtual void onExit() = 0;
};

} // namespace cengine::core
