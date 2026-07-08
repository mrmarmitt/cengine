#include <cengine/core/EngineManager.hpp>

#include <stdexcept>
#include <utility>

namespace cengine::core {

EngineManager::EngineManager(std::unique_ptr<IWindowManager> windowManager,
                             std::unique_ptr<IGameManager> gameManager,
                             const Seconds fixedDt,
                             const Seconds maxFrameTime,
                             ClockNowFn clockNow) : m_windowManager(std::move(windowManager)),
                                                    m_gameManager(std::move(gameManager)),
                                                    m_fixedDt(fixedDt),
                                                    m_maxFrameTime(maxFrameTime),
                                                    m_clockNow(std::move(clockNow)),
                                                    m_isRunning(false) {
    if (m_fixedDt <= Seconds{0}) {
        throw std::invalid_argument("EngineManager: fixedDt must be positive");
    }
    if (m_maxFrameTime <= Seconds{0}) {
        throw std::invalid_argument("EngineManager: maxFrameTime must be positive");
    }
}

void EngineManager::start() {
    if (m_windowManager) {
        m_windowManager->init();
    }

    m_isRunning = true;
    run();
}

void EngineManager::cleanup() const {
    if (m_gameManager) {
        m_gameManager->cleanup();
    }

    if (m_windowManager) {
        m_windowManager->cleanup();
    }
}

void EngineManager::run() {
    TimePoint previous = m_clockNow();
    Seconds accumulator{0};

    while (m_isRunning) {
        const TimePoint now = m_clockNow();
        Seconds frameTime = now - previous;
        previous = now;

        // Teto anti-espiral: um quadro muito lento não pode gerar passos de
        // simulação suficientes para deixar o próximo quadro ainda mais lento.
        if (frameTime > m_maxFrameTime) {
            frameTime = m_maxFrameTime;
        }

        m_windowManager->update();

        m_gameManager->onEnter();
        m_gameManager->input();

        // Fixed timestep: consome o tempo acumulado em passos de dt constante
        // (0..N chamadas por quadro; o resto fica para o próximo quadro).
        for (accumulator += frameTime; accumulator >= m_fixedDt; accumulator -= m_fixedDt) {
            m_gameManager->update(m_fixedDt);
        }

        m_gameManager->render();
        m_gameManager->onExit();

        m_isRunning = !m_gameManager->shouldExit();
    }
    cleanup();
}

} // namespace cengine::core
