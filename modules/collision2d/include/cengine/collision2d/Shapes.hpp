#pragma once

namespace cengine::collision2d {

/// Ponto/vetor 2D. Sem operadores: este modulo nao e uma biblioteca de algebra
/// linear — e so o vocabulario minimo para descrever as formas.
struct Vec2
{
    float x = 0.0f;
    float y = 0.0f;
};

/// Retangulo alinhado aos eixos, pelo canto superior esquerdo + tamanho.
///
/// Convencao de eixos: nenhuma. O modulo nao sabe se Y cresce para cima ou para
/// baixo — a intersecao e a mesma nos dois casos. Quem tem convencao e o jogo.
struct Aabb
{
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
};

struct Circle
{
    Vec2  center = {};
    float radius = 0.0f;
};

} // namespace cengine::collision2d
