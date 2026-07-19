#include <gtest/gtest.h>

#include <cengine/camera2d/Viewport.hpp>

// A metade MECANISMO da camera (task 23): projecao mundo -> viewport por
// subtracao e culling contra a janela inflada pela margem. O seguimento (para
// onde a camera olha) NUNCA sobe — os cenarios de follow/clamp continuam nos
// testes dos jogos.
//
// A ultima secao e a regra de proveniencia da ADR 0002: a forma veio de DOIS
// consumidores reais com o `visible()` identico linha a linha, e os valores
// abaixo sao transcritos deles.

using cengine::camera2d::Viewport;
using cengine::camera2d::visible;
using cengine::camera2d::worldToView;

// =============================================================================
// worldToView — a projecao e uma subtracao, e so
// =============================================================================

TEST(Camera2dViewportTest, WorldToViewSubtractsTheOrigin)
{
    const Viewport vp{ .origin = { 100.0f, 40.0f }, .size = { 320.0f, 180.0f } };

    const auto v = worldToView(vp, { 130.0f, 40.0f });
    EXPECT_FLOAT_EQ(v.x, 30.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);

    // Atras da origem: coordenada de view negativa (o culling decide se
    // desenha; a projecao nao opina).
    const auto behind = worldToView(vp, { 90.0f, 30.0f });
    EXPECT_FLOAT_EQ(behind.x, -10.0f);
    EXPECT_FLOAT_EQ(behind.y, -10.0f);
}

TEST(Camera2dViewportTest, OriginAtZeroIsTheIdentity)
{
    const Viewport vp{ .origin = {}, .size = { 320.0f, 180.0f } };

    const auto v = worldToView(vp, { 55.0f, 77.0f });
    EXPECT_FLOAT_EQ(v.x, 55.0f);
    EXPECT_FLOAT_EQ(v.y, 77.0f);
}

// =============================================================================
// visible — o culling contra a janela inflada
// =============================================================================

TEST(Camera2dViewportTest, RectsInsideTheViewAreVisible)
{
    const Viewport vp{ .origin = { 0.0f, 0.0f }, .size = { 320.0f, 180.0f }, .cullMargin = 0.0f };

    EXPECT_TRUE(visible(vp, { 10.0f, 10.0f, 16.0f, 16.0f }));
    EXPECT_TRUE(visible(vp, { 310.0f, 170.0f, 16.0f, 16.0f })) << "atravessando a borda ainda aparece";
}

TEST(Camera2dViewportTest, RectsBeyondTheMarginAreCulled)
{
    const Viewport vp{ .origin = { 0.0f, 0.0f }, .size = { 320.0f, 180.0f }, .cullMargin = 16.0f };

    // Dentro da margem (a folga que impede a borda de piscar): visivel.
    EXPECT_TRUE(visible(vp, { 328.0f, 10.0f, 16.0f, 16.0f }));
    EXPECT_TRUE(visible(vp, { 10.0f, -20.0f, 16.0f, 16.0f }));

    // Alem dela: nem projeta.
    EXPECT_FALSE(visible(vp, { 400.0f, 10.0f, 16.0f, 16.0f }));
    EXPECT_FALSE(visible(vp, { 10.0f, 300.0f, 16.0f, 16.0f }));
}

TEST(Camera2dViewportTest, TheViewportScrollsWithItsOrigin)
{
    // A mesma janela, rolada: o que era visivel deixa de ser, e vice-versa.
    const Viewport at0{ .origin = { 0.0f, 0.0f }, .size = { 320.0f, 180.0f }, .cullMargin = 16.0f };
    const Viewport scrolled{ .origin = { 320.0f, 204.0f }, .size = { 320.0f, 180.0f }, .cullMargin = 16.0f };

    const cengine::collision2d::Aabb tile{ 480.0f, 288.0f, 16.0f, 16.0f };
    EXPECT_FALSE(visible(at0, tile));
    EXPECT_TRUE(visible(scrolled, tile));
}

// =============================================================================
// Consumidores reais — a regra de proveniencia (ADR 0002)
// =============================================================================

// --- mario-bros @ 4a8f825 (src/mario/camera/Camera.cpp:27-32) ---
//
// O 1o consumidor: rolagem HORIZONTAL (nivel 64 colunas x 16 = 1024 de
// largura; a altura cabe na janela). kCullMargin = 16 ("um tile: a borda nao
// pode piscar sumindo"). A formula de la: view = {x-16, y-16, w+32, h+32},
// intersects(worldRect, view).

TEST(Camera2dViewportTest, MarioSideScrollCulling)
{
    // A camera do mario no meio do nivel: origem (400, 0).
    const Viewport vp{ .origin = { 400.0f, 0.0f }, .size = { 400.0f, 224.0f }, .cullMargin = 16.0f };

    EXPECT_TRUE(visible(vp, { 416.0f, 192.0f, 16.0f, 16.0f })) << "chao sob os pes do jogador";
    EXPECT_TRUE(visible(vp, { 384.0f, 192.0f, 16.0f, 16.0f })) << "um tile atras, dentro da margem";
    EXPECT_FALSE(visible(vp, { 100.0f, 192.0f, 16.0f, 16.0f })) << "o comeco do nivel ja rolou para fora";
    EXPECT_FALSE(visible(vp, { 1008.0f, 192.0f, 16.0f, 16.0f })) << "a bandeira ainda esta longe";
}

// --- zelda @ 9658ae0 (src/zelda/camera/Camera.cpp:31-36 + task 03) ---
//
// O 2o consumidor: a MESMA formula, agora rolando nos DOIS eixos. View fixa
// 320x180 (16:9), masmorra 40x24 tiles (640x384), kCullMargin = 16. A cena so
// desenha ~20x11 dos 40x24 tiles por quadro — este culling e o que poupa o
// resto.

TEST(Camera2dViewportTest, ZeldaTwoAxisCulling)
{
    // A camera centrada no heroi no meio da masmorra: origem (160, 102).
    const Viewport vp{ .origin = { 160.0f, 102.0f }, .size = { 320.0f, 180.0f }, .cullMargin = 16.0f };

    EXPECT_TRUE(visible(vp, { 320.0f, 176.0f, 16.0f, 16.0f })) << "um pilar perto do heroi";
    EXPECT_TRUE(visible(vp, { 144.0f, 96.0f, 16.0f, 16.0f })) << "na borda, dentro da margem";
    EXPECT_FALSE(visible(vp, { 0.0f, 0.0f, 16.0f, 16.0f })) << "o canto da masmorra ficou atras";
    EXPECT_FALSE(visible(vp, { 624.0f, 368.0f, 16.0f, 16.0f })) << "o canto oposto ainda nao rolou";

    // E a projecao da cena de la: tela = (mundo - camera) * escala + centro.
    // A parte que subiu e a subtracao; escala/centro seguem na cena.
    const auto v = worldToView(vp, { 320.0f, 176.0f });
    EXPECT_FLOAT_EQ(v.x, 160.0f);
    EXPECT_FLOAT_EQ(v.y, 74.0f);
}
