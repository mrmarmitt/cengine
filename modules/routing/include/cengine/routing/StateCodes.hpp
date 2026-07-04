#pragma once

#include <string_view>

namespace cengine::routing {

// Código de estado especial que sinaliza o encerramento do jogo.
// Usado no lugar da string literal "exit" espalhada pela lógica de controle.
inline constexpr std::string_view kExitStateCode{"exit"};

} // namespace cengine::routing
