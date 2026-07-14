#include <gtest/gtest.h>

#include <cengine/collision2d/Intersects.hpp>

// A deteccao 2D da engine. Geometria pura: nenhum jogo, nenhuma plataforma.
//
// A ultima secao ("consumidores reais") e o PEDAGIO da Emenda 1 da ADR 0002:
// como uma das duas evidencias que trouxeram este modulo vem de um jogo
// ESTACIONADO (o Space Invaders, que nunca vai linka-lo), a suite da engine tem
// de encarnar o caso de uso dele — provando que o mecanismo expressa a situacao
// real daquele jogo sem descongela-lo. Se um dia o modulo deixar de servir a
// esses testes, ele deixou de servir ao aprendizado que o justificou.

using cengine::collision2d::Aabb;
using cengine::collision2d::Circle;
using cengine::collision2d::intersects;

// =============================================================================
// Retangulo x retangulo
// =============================================================================

TEST(Collision2dTest, OverlappingBoxesIntersect)
{
    EXPECT_TRUE(intersects(Aabb{ 0.0f, 0.0f, 10.0f, 10.0f }, Aabb{ 5.0f, 5.0f, 10.0f, 10.0f }));
}

TEST(Collision2dTest, SeparateBoxesDoNotIntersect)
{
    EXPECT_FALSE(intersects(Aabb{ 0.0f, 0.0f, 10.0f, 10.0f }, Aabb{ 20.0f, 0.0f, 10.0f, 10.0f }));

    // Separados so no eixo Y (alinhados em X): ainda assim, separados.
    EXPECT_FALSE(intersects(Aabb{ 0.0f, 0.0f, 10.0f, 10.0f }, Aabb{ 0.0f, 20.0f, 10.0f, 10.0f }));
}

TEST(Collision2dTest, TouchingBoxEdgesDoNotIntersect)
{
    // Borda direita de `a` exatamente na esquerda de `b`: area de sobreposicao
    // zero. Encostar nao conta (contrato documentado no header).
    EXPECT_FALSE(intersects(Aabb{ 0.0f, 0.0f, 10.0f, 10.0f }, Aabb{ 10.0f, 0.0f, 10.0f, 10.0f }));
}

TEST(Collision2dTest, BoxContainedInAnotherIntersects)
{
    EXPECT_TRUE(intersects(Aabb{ 0.0f, 0.0f, 100.0f, 100.0f }, Aabb{ 40.0f, 40.0f, 5.0f, 5.0f }));
}

TEST(Collision2dTest, BoxIntersectionIsSymmetric)
{
    const Aabb a{ 0.0f, 0.0f, 10.0f, 10.0f };
    const Aabb b{ 5.0f, 5.0f, 10.0f, 10.0f };

    EXPECT_EQ(intersects(a, b), intersects(b, a));
}

// =============================================================================
// Circulo x circulo
// =============================================================================

TEST(Collision2dTest, OverlappingCirclesIntersect)
{
    EXPECT_TRUE(intersects(Circle{ { 0.0f, 0.0f }, 10.0f }, Circle{ { 15.0f, 0.0f }, 10.0f }));
}

TEST(Collision2dTest, SeparateCirclesDoNotIntersect)
{
    EXPECT_FALSE(intersects(Circle{ { 0.0f, 0.0f }, 10.0f }, Circle{ { 21.0f, 0.0f }, 10.0f }));
}

TEST(Collision2dTest, TangentCirclesIntersect)
{
    // Distancia == soma dos raios: tangentes. CONTA (contrato do header) — senao
    // um tiro passaria raspando sem acertar.
    EXPECT_TRUE(intersects(Circle{ { 0.0f, 0.0f }, 10.0f }, Circle{ { 20.0f, 0.0f }, 10.0f }));
}

TEST(Collision2dTest, CircleContainedInAnotherIntersects)
{
    EXPECT_TRUE(intersects(Circle{ { 0.0f, 0.0f }, 50.0f }, Circle{ { 5.0f, 5.0f }, 2.0f }));
}

TEST(Collision2dTest, CircleDistanceIsMeasuredOnBothAxes)
{
    // Perto em X, longe em Y: nao se tocam. (Guarda contra a distancia ser
    // calculada so num eixo.)
    EXPECT_FALSE(intersects(Circle{ { 0.0f, 0.0f }, 5.0f }, Circle{ { 1.0f, 40.0f }, 5.0f }));
}

// =============================================================================
// Circulo x retangulo
// =============================================================================

TEST(Collision2dTest, CircleOverlappingBoxIntersects)
{
    EXPECT_TRUE(intersects(Circle{ { 12.0f, 5.0f }, 5.0f }, Aabb{ 0.0f, 0.0f, 10.0f, 10.0f }));
}

TEST(Collision2dTest, CircleAwayFromBoxDoesNotIntersect)
{
    EXPECT_FALSE(intersects(Circle{ { 30.0f, 5.0f }, 5.0f }, Aabb{ 0.0f, 0.0f, 10.0f, 10.0f }));
}

TEST(Collision2dTest, CircleNearBoxCornerUsesTrueDistanceNotTheBoundingBox)
{
    // O circulo esta na diagonal do canto (10,10), a ~7.07 de distancia, com
    // raio 5: NAO alcanca. Uma implementacao preguicosa (que so testasse o
    // AABB do circulo contra a caixa) diria que sim — este teste e o guarda.
    EXPECT_FALSE(intersects(Circle{ { 15.0f, 15.0f }, 5.0f }, Aabb{ 0.0f, 0.0f, 10.0f, 10.0f }));

    // Mais perto do mesmo canto, agora dentro do alcance.
    EXPECT_TRUE(intersects(Circle{ { 13.0f, 13.0f }, 5.0f }, Aabb{ 0.0f, 0.0f, 10.0f, 10.0f }));
}

TEST(Collision2dTest, CircleInsideBoxIntersects)
{
    EXPECT_TRUE(intersects(Circle{ { 5.0f, 5.0f }, 1.0f }, Aabb{ 0.0f, 0.0f, 10.0f, 10.0f }));
}

TEST(Collision2dTest, CircleBoxIntersectionIsSymmetric)
{
    const Circle circle{ { 12.0f, 5.0f }, 5.0f };
    const Aabb   box{ 0.0f, 0.0f, 10.0f, 10.0f };

    EXPECT_EQ(intersects(circle, box), intersects(box, circle));
}

// =============================================================================
// Consumidores reais — o pedagio da Emenda 1 (ADR 0002)
// =============================================================================

// --- Space Invaders (ESTACIONADO na 0.5.0): a evidencia AABB ---
//
// Arena 224x256, Y para baixo, SEM wrap. Sprites do arcade: invasor 12x8, tiro
// do jogador 1x4, bomba 3x7, canhao 13x8. Se o modulo nao consegue expressar
// isto, ele nao esta pronto para subir.

TEST(Collision2dTest, SpaceInvadersPlayerShotHitsInvader)
{
    const Aabb invader{ 100.0f, 60.0f, 12.0f, 8.0f };

    // Tiro subindo, entrando na base do invasor.
    const Aabb shotHitting{ 104.0f, 64.0f, 1.0f, 4.0f };
    EXPECT_TRUE(intersects(shotHitting, invader));

    // Mesma coluna, ainda abaixo dele: nao acertou (ainda).
    const Aabb shotBelow{ 104.0f, 80.0f, 1.0f, 4.0f };
    EXPECT_FALSE(intersects(shotBelow, invader));

    // Na altura certa, mas na coluna do vizinho: passa raspando.
    const Aabb shotBeside{ 120.0f, 64.0f, 1.0f, 4.0f };
    EXPECT_FALSE(intersects(shotBeside, invader));
}

TEST(Collision2dTest, SpaceInvadersBombHitsPlayerCannon)
{
    const Aabb cannon{ 50.0f, 216.0f, 13.0f, 8.0f };

    const Aabb bombHitting{ 55.0f, 214.0f, 3.0f, 7.0f };
    EXPECT_TRUE(intersects(bombHitting, cannon));

    const Aabb bombFalling{ 55.0f, 180.0f, 3.0f, 7.0f };
    EXPECT_FALSE(intersects(bombFalling, cannon));
}

// --- Asteroids (consumidor VIVO): a evidencia circulo ---
//
// Arena 800x600 que DA A VOLTA. O wrap fica no jogo: ele calcula o menor delta
// no toro e pergunta a engine com a posicao ja corrigida — a engine nao sabe (e
// nao deve saber) o formato do mundo. Estes testes mostram os dois lados desse
// contrato.

TEST(Collision2dTest, AsteroidsShotHitsRock)
{
    const Circle rock{ { 400.0f, 180.0f }, 42.0f };

    EXPECT_TRUE(intersects(Circle{ { 400.0f, 220.0f }, 2.0f }, rock));
    EXPECT_FALSE(intersects(Circle{ { 400.0f, 300.0f }, 2.0f }, rock));
}

TEST(Collision2dTest, AsteroidsShipHitsRock)
{
    const Circle ship{ { 400.0f, 300.0f }, 9.0f };
    const Circle rock{ { 430.0f, 300.0f }, 22.0f };

    EXPECT_TRUE(intersects(ship, rock));
}

TEST(Collision2dTest, AsteroidsWrapIsTheGamesJobNotTheEngines)
{
    // Nave colada na borda esquerda, rocha colada na direita: na arena-toro do
    // Asteroids elas se tocam (10 de distancia PELA BORDA).
    const Circle ship{ { 5.0f, 300.0f }, 9.0f };
    const Circle rockFarSide{ { 795.0f, 300.0f }, 22.0f };

    // A engine, que nao sabe do toro, responde o que a geometria plana diz:
    // 790 de distancia, nao se tocam. E o certo — ela nao tem opiniao sobre o
    // formato do mundo.
    EXPECT_FALSE(intersects(ship, rockFarSide));

    // O JOGO corrige a posicao pelo menor delta no toro e pergunta de novo.
    // (Aqui, o mesmo que o ast::World::toroidalDelta faz: 795 - 800 = -5.)
    constexpr float kArenaW = 800.0f;
    const Circle    rockWrapped{ { rockFarSide.center.x - kArenaW, rockFarSide.center.y }, rockFarSide.radius };

    EXPECT_TRUE(intersects(ship, rockWrapped)) << "com a posicao corrigida pelo jogo, a engine acerta";
}
