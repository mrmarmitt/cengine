#pragma once

#include <chrono>
#include <functional>
#include <memory>

#include <cengine/core/IGameManager.hpp>
#include <cengine/core/IWindowManager.hpp>
#include <cengine/core/Time.hpp>

namespace cengine::core {

/**
 * @brief O coração da engine: o game loop, com *fixed timestep*.
 *
 * É a única classe concreta do `core`. Recebe por injeção de dependência um
 * `IWindowManager` (a plataforma gráfica) e um `IGameManager` (as regras/telas
 * do jogo), assumindo posse de ambos, e roda o loop principal até o jogo pedir
 * para sair.
 *
 * Cada iteração executa: `window.update()` → `game.onEnter()` → `game.input()`
 * → `game.update(dt)` (0..N vezes) → `game.render()` → `game.onExit()`, e para
 * quando `game.shouldExit()` retorna true — encerrando com `cleanup()`.
 *
 * ## Tempo (padrão "fix your timestep")
 * O tempo do quadro é medido com relógio monotônico e acumulado; a simulação
 * avança em passos FIXOS de `fixedDt` (default 1/60 s): num quadro curto,
 * `update` pode não rodar; num quadro longo, roda várias vezes — sempre com o
 * mesmo dt, o que mantém a simulação determinística e estável (pré-requisito
 * de física). O tempo de quadro é limitado a `maxFrameTime` (default 250 ms)
 * para evitar a espiral da morte (quadro lento → mais passos → quadro mais
 * lento). Render não interpola entre passos (fora do escopo — ver task 14).
 *
 * A fonte de tempo é injetável (`clockNow`) para permitir testes
 * determinísticos do loop, sem tempo real; o default é o relógio monotônico
 * `std::chrono::steady_clock`.
 *
 * @code
 * cengine::core::EngineManager engine{
 *     std::make_unique<MyWindowManager>(),
 *     std::make_unique<MyGameManager>()
 * };
 * engine.start(); // bloqueia até o jogo pedir para sair
 * @endcode
 */
class EngineManager {
public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using ClockNowFn = std::function<TimePoint()>;

    static constexpr Seconds kDefaultFixedDt{1.0 / 60.0};
    static constexpr Seconds kDefaultMaxFrameTime{0.25};

    /**
     * @param windowManager plataforma gráfica (posse transferida à engine).
     * @param gameManager   regras/telas do jogo (posse transferida à engine).
     * @param fixedDt       passo fixo da simulação (> 0; default 1/60 s).
     * @param maxFrameTime  teto do tempo de quadro acumulável (> 0; default
     *                      250 ms) — proteção contra a espiral da morte.
     * @param clockNow      fonte de tempo (default: relógio monotônico).
     * @throws std::invalid_argument se @p fixedDt ou @p maxFrameTime não for
     *         positivo.
     */
    EngineManager(
        std::unique_ptr<IWindowManager> windowManager,
        std::unique_ptr<IGameManager> gameManager,
        Seconds fixedDt = kDefaultFixedDt,
        Seconds maxFrameTime = kDefaultMaxFrameTime,
        ClockNowFn clockNow = Clock::now);

    ~EngineManager() = default;

    /// Inicializa a janela e entra no game loop. Bloqueia até `shouldExit()`.
    void start();

    /// Libera recursos do jogo e da janela. Invocado ao final de `start()`.
    void cleanup() const;

private:
    void run();

    std::unique_ptr<IWindowManager> m_windowManager;
    std::unique_ptr<IGameManager> m_gameManager;
    Seconds m_fixedDt;
    Seconds m_maxFrameTime;
    ClockNowFn m_clockNow;
    bool m_isRunning;
};

} // namespace cengine::core
