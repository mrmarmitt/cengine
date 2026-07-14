#pragma once

namespace cengine::input {

/// O vocabulario de teclado que as CENAS falam.
///
/// Nao existe tecla fisica aqui: nem scancode, nem virtual key do Windows, nem
/// evento do ftxui. Traduzir o teclado real para este vocabulario e trabalho da
/// PLATAFORMA (WndProc, bindings do The-Forge, eventos de terminal) — e nunca
/// entra na engine.
///
/// O conjunto e o que os jogos do ecossistema realmente usaram: setas + Enter/
/// Escape/Backspace (8puzzle), Space (asteroids), e Char para texto imprimivel
/// (digitar o nome no recorde).
enum class Key
{
    None,      ///< nenhuma tecla pendente
    Up,
    Down,
    Left,
    Right,
    Space,
    Enter,
    Escape,
    Backspace,
    Char,      ///< caractere imprimivel — ver KeyEvent::character
    Other,     ///< tecla que a plataforma reconheceu mas o jogo nao nomeia
};

struct KeyEvent
{
    Key  key = Key::None;
    char character = '\0'; ///< valido quando key == Key::Char
};

} // namespace cengine::input
