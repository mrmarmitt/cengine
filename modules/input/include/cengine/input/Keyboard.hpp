#pragma once

#include <cstddef>
#include <vector>

#include <cengine/input/Key.hpp>

namespace cengine::input {

/// O CONTRATO de teclado entre a plataforma (que captura) e as cenas (que
/// consomem) — o mecanismo que os jogos vinham copiando em cada ponte.
///
/// Duas leituras, porque um jogo precisa das duas e elas nao se substituem:
///
/// - **fila de EDGES** (`pushKey`/`readKey`): "o jogador APERTOU". Serve para
///   navegar menu, confirmar, digitar. A cena consome **no maximo um evento por
///   `input()`** — e esse limite e semantica, nao detalhe: e ele que impede uma
///   tecla repetida de atravessar tres itens de menu num quadro.
/// - **estado SEGURADO** (`pushHeldKey`/`isHeld`): "a tecla esta pressionada
///   AGORA". Serve para mover: uma nave nao anda com edges, anda enquanto a
///   seta estiver apertada, todo quadro.
///
/// O que este objeto NAO sabe: teclado fisico, janela, foco, GPU. A plataforma
/// empurra; a cena le. E so.
class Keyboard
{
public:
    /// Teto da fila. Cheia, o evento NOVO e descartado (e nao o antigo): quem
    /// esta na fila chegou primeiro e sera consumido primeiro — descartar o
    /// velho embaralharia a ordem que o jogador digitou.
    static constexpr size_t kQueueMax = 32;

    // --- lado da PLATAFORMA (quem captura) ---

    void pushKey(KeyEvent event);

    /// Publica o estado segurado de UMA tecla. O estado persiste ate o proximo
    /// push da mesma tecla — a plataforma nao precisa republicar a cada quadro.
    void pushHeldKey(Key key, bool held);

    /// Solta todas as teclas seguradas. A janela perdeu o foco: o KEYUP nunca
    /// vai chegar, e sem isto a nave sairia voando sozinha ao voltar.
    void clearHeldKeys();

    // --- lado das CENAS (quem consome) ---

    /// Consome no maximo um evento. Fila vazia devolve `Key::None`.
    [[nodiscard]] KeyEvent readKey();

    [[nodiscard]] bool isHeld(Key key) const;

    /// Conveniencia para eixos: -1 com so `negative` segurada, +1 com so
    /// `positive`, 0 nos demais casos (nenhuma, ou as duas).
    [[nodiscard]] float heldAxis(Key negative, Key positive) const;

private:
    static constexpr size_t kKeyCount = static_cast<size_t>(Key::Other) + 1;

    std::vector<KeyEvent> m_queue;
    bool                  m_held[kKeyCount] = {};
};

} // namespace cengine::input
