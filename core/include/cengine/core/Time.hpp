#pragma once

#include <chrono>

namespace cengine::core {

/// Duração em segundos (ponto flutuante) usada no contrato de tempo do loop.
/// `std::chrono` de ponta a ponta: nada de `double` cru com unidade ambígua.
using Seconds = std::chrono::duration<double>;

} // namespace cengine::core
