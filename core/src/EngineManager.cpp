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

bool EngineManager::frame(Seconds frameTime) {
    // Teto anti-espiral: um quadro muito lento não pode gerar passos de
    // simulação suficientes para deixar o próximo quadro ainda mais lento.
    // (Se o host clampa o dt dele antes, os dois clamps se compõem.)
    if (frameTime > m_maxFrameTime) {
        frameTime = m_maxFrameTime;
    }

    m_gameManager->onEnter();
    m_gameManager->input();

    // Fixed timestep: consome o tempo acumulado em passos de dt constante
    // (0..N chamadas por quadro; o resto fica para o próximo quadro).
    for (m_accumulator += frameTime; m_accumulator >= m_fixedDt; m_accumulator -= m_fixedDt) {
        m_gameManager->update(m_fixedDt);
    }

    // O render acontece mesmo num quadro sem passos de update.
    m_gameManager->render();
    m_gameManager->onExit();

    return !m_gameManager->shouldExit();
}

void EngineManager::run() {
    // O modo próprio é um consumidor de frame(): mede o tempo, atualiza a
    // janela e delega o quadro — mesma lógica nos dois modos (ver task 15).
    TimePoint previous = m_clockNow();

    while (m_isRunning) {
        const TimePoint now = m_clockNow();
        const Seconds frameTime = now - previous;
        previous = now;

        // A janela é da engine apenas no modo próprio; no hospedado ela é do
        // host — por isso update()/present() ficam fora de frame(). present()
        // roda mesmo no último quadro (shouldExit): o que foi desenhado é
        // apresentado antes do cleanup.
        if (m_windowManager) {
            m_windowManager->update();
        }

        m_isRunning = frame(frameTime);

        if (m_windowManager) {
            m_windowManager->present();
        }
    }
    cleanup();
}

} // namespace cengine::core
