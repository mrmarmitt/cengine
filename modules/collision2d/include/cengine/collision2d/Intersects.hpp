#pragma once

#include <cengine/collision2d/Shapes.hpp>

// cengine::collision2d — DETECCAO de colisao 2D, nada alem disso.
//
// A engine responde uma unica pergunta: "estas duas formas se tocam?". Possuir
// entidades, mover, decidir o que a colisao significa, pontuar, tirar vida ou
// encerrar a partida continua sendo do jogo (ADR 0001: core minimo, jogo dono
// das regras).
//
// O que este modulo deliberadamente NAO sabe:
//
// - **o formato do mundo.** Arena que da a volta (o toro do Asteroids) e
//   politica do jogo: o consumidor calcula o menor delta na topologia dele e
//   pergunta a engine com a posicao ja corrigida. O Space Invaders, cuja arena
//   nao da a volta, chama direto. Ver a Emenda 1 da ADR 0002.
// - **a convencao de eixos.** Y para cima ou para baixo da no mesmo.
// - **unidades.** Pixels, celulas ou unidades de arena: problema do jogo.
//
// Evidencias que trouxeram o modulo (ADR 0002, criterio 2): AABB do Space
// Invaders (tiro x invasor, bomba x jogador) e circulo do Asteroids (tiro x
// rocha, nave x rocha). Os dois casos estao encarnados na suite deste modulo.

namespace cengine::collision2d {

/// Retangulo x retangulo. Encostar NAO conta: bordas coincidentes (o direito de
/// `a` no esquerdo de `b`) sao consideradas separadas — a area de sobreposicao e
/// zero.
[[nodiscard]] bool intersects(const Aabb& a, const Aabb& b);

/// Circulo x circulo. Tangenciar CONTA (distancia == soma dos raios): e o limiar
/// natural do "se tocam?", e evita que um tiro passe raspando sem acertar.
[[nodiscard]] bool intersects(const Circle& a, const Circle& b);

/// Circulo x retangulo: o circulo alcanca o ponto do retangulo mais proximo do
/// centro dele? Tangenciar CONTA, pelo mesmo motivo.
[[nodiscard]] bool intersects(const Circle& circle, const Aabb& box);

/// Simetria — a ordem dos argumentos nao muda a resposta.
[[nodiscard]] bool intersects(const Aabb& box, const Circle& circle);

} // namespace cengine::collision2d
