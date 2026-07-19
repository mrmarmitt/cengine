#pragma once

#include <cengine/collision2d/Shapes.hpp>

// cengine::camera2d (task 23): a metade MECANISMO da camera 2D — a projecao
// mundo -> viewport por subtracao da origem, e o culling de retangulos contra a
// area visivel com margem. Extraida das copias identicas do mario-bros
// (@ 4a8f825, `src/mario/camera/Camera.cpp`) e do zelda (@ 9658ae0,
// `src/zelda/camera/Camera.cpp`): o `visible()` dos dois era o MESMO codigo,
// linha a linha, so mudando o namespace.
//
// O que NAO subiu — e nunca sobe (o corte da task 23): o SEGUIMENTO. Para onde
// a camera olha (ancorar o foco, look-ahead, deadzone, travar nos limites do
// nivel, saltar por sala) e FEEL de cada jogo: o mario segue so em X, o zelda
// nos dois eixos, um metroidvania saltaria por sala. A engine com opiniao sobre
// seguimento seria a engine com opiniao sobre o genero.
//
// Escala, letterbox e centralizacao em PIXELS tambem ficam nas cenas: este
// modulo trabalha em unidades de MUNDO e nao sabe o tamanho da janela real.

namespace cengine::camera2d {

using Aabb = collision2d::Aabb;
using Vec2 = collision2d::Vec2;

/// A janela sobre o mundo, em unidades de MUNDO. A ORIGEM e calculada pelo
/// jogo (follow/deadzone/limites — a politica); este tipo so projeta e
/// consulta visibilidade.
struct Viewport
{
    Vec2  origin = {};      // canto superior esquerdo da janela, no mundo
    Vec2  size = {};        // largura/altura visiveis
    float cullMargin = 0.0f; // folga do culling (a borda nao pode piscar sumindo)
};

/// Projeta um ponto do mundo para o espaco da viewport: subtrai a origem. A
/// cena dai escala/centraliza em pixels (isso e dela).
[[nodiscard]] Vec2 worldToView(const Viewport& viewport, Vec2 world);

/// O retangulo do mundo aparece na janela (inflada pela margem)? Falso = pode
/// nem projetar (o culling que poupa o desenho do que esta fora).
[[nodiscard]] bool visible(const Viewport& viewport, const Aabb& worldRect);

} // namespace cengine::camera2d
