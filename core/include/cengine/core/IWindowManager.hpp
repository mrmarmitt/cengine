#pragma once

namespace cengine::core {

/**
 * @brief Abstrai a janela/plataforma gráfica sobre a qual o jogo roda.
 *
 * É o ponto de extensão que mantém a engine **agnóstica de biblioteca gráfica**:
 * o jogo fornece uma implementação (SDL, Raylib, GLFW, terminal...) e a engine
 * só a aciona pelo contrato abaixo.
 *
 * Chamadas pelo `EngineManager` (modo próprio): `init()` uma vez em `start()`;
 * a cada iteração do loop, `update()` ANTES das fases do jogo e `present()`
 * DEPOIS delas; `cleanup()` uma vez ao encerrar. O quadro tem simetria
 * início/fim:
 *
 * @code
 * window.update()   // eventos de SO, preparo do quadro (acquire, beginCmd...)
 * fases do jogo     // onEnter -> input -> update(fixedDt) 0..N -> render -> onExit
 * window.present()  // fecha e apresenta o quadro (endCmd, submit, queuePresent)
 * @endcode
 */
class IWindowManager {
public:
    virtual ~IWindowManager() = default;

    /// Cria/inicializa a janela e o contexto gráfico. Chamado uma vez.
    virtual void init() = 0;

    /// Início do quadro: eventos de SO e preparo do desenho (em GPU: adquirir
    /// a imagem do swapchain, abrir o command buffer). Chamado toda iteração,
    /// antes das fases do jogo.
    virtual void update() = 0;

    /// Fim do quadro: fecha e apresenta o que as fases desenharam (em GPU:
    /// endCmd, submit, queuePresent; no terminal: imprimir a tela — ou vazio).
    /// Chamado toda iteração, depois de `render()`/`onExit()` — inclusive no
    /// último quadro, quando o jogo já pediu saída: o quadro desenhado é
    /// apresentado antes do `cleanup()`.
    ///
    /// @note Roda DEPOIS do commit de troca de cena (`onExit()` pode destruir
    ///       a cena que rendeu o quadro). Por isso o contrato exige que todo
    ///       recurso referenciado pelo quadro gravado (fontes, buffers,
    ///       swapchain...) pertença à plataforma — cenas são lógica pura e
    ///       desenham via ponte da plataforma, nunca possuindo recurso de
    ///       GPU. Mesma ordem do modo hospedado, em que o host apresenta
    ///       depois de `frame()` retornar.
    virtual void present() = 0;

    /// Destrói a janela e libera recursos gráficos. Chamado uma vez, ao sair.
    virtual void cleanup() = 0;
};

} // namespace cengine::core
