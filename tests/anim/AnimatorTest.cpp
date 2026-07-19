#include <gtest/gtest.h>

#include <cstdint>

#include <cengine/anim/Animator.hpp>

// A maquina de clip (task 25): clip atual + frame + acumulador; troca zera; so
// ciclos multiframe avancam. A SELECAO do clip (que estado toca o que) e dos
// jogos — aqui entra so o clip DESEJADO ja escolhido.
//
// A ultima secao e a regra de proveniencia da ADR 0002: o nucleo veio de DOIS
// consumidores reais identicos ate na cadencia, e os cenarios abaixo sao
// transcritos dos testes deles.

using cengine::anim::Animator;
using cengine::anim::ClipDesc;
using cengine::anim::ClipId;

namespace {

constexpr double kStep = 1.0 / 60.0; // o passo fixo dos consumidores

void advance(Animator& anim, const double seconds, const ClipId desired)
{
    for (double elapsed = 0.0; elapsed < seconds; elapsed += kStep)
    {
        anim.update(kStep, desired);
    }
}

} // namespace

// =============================================================================
// O mecanismo
// =============================================================================

TEST(AnimatorTest, StartsOnTheInitialClipAtFrameZero)
{
    const Animator anim{ { { 1, 0.0 }, { 2, 0.12 } }, 1 };
    EXPECT_EQ(anim.clip(), 1u);
    EXPECT_EQ(anim.frame(), 0);
}

TEST(AnimatorTest, OutOfRangeInitialFallsBackToZero)
{
    const Animator anim{ { { 1, 0.0 } }, 7 };
    EXPECT_EQ(anim.clip(), 0u);
}

TEST(AnimatorTest, MultiframeClipsCycleAtTheirCadence)
{
    Animator anim{ { { 1, 0.0 }, { 3, 0.10 } } };

    anim.update(kStep, 1); // entra no ciclo (frame 0)
    ASSERT_EQ(anim.frame(), 0);

    advance(anim, 0.10, 1);
    EXPECT_EQ(anim.frame(), 1);
    advance(anim, 0.10, 1);
    EXPECT_EQ(anim.frame(), 2);
    advance(anim, 0.10, 1);
    EXPECT_EQ(anim.frame(), 0) << "ciclo de 3 da a volta";
}

TEST(AnimatorTest, SwitchingClipsResetsFrameAndTime)
{
    Animator anim{ { { 1, 0.0 }, { 2, 0.12 } } };

    advance(anim, 0.15, 1); // no meio do ciclo, frame 1
    ASSERT_EQ(anim.frame(), 1);

    anim.update(kStep, 0);
    EXPECT_EQ(anim.clip(), 0u);
    EXPECT_EQ(anim.frame(), 0);

    // Voltando ao ciclo: comeca do zero, sem herdar o acumulador antigo.
    anim.update(kStep, 1);
    EXPECT_EQ(anim.frame(), 0);
    advance(anim, 0.12, 1);
    EXPECT_EQ(anim.frame(), 1) << "a cadencia conta a partir da troca";
}

TEST(AnimatorTest, SingleFrameClipsNeverAdvance)
{
    Animator anim{ { { 1, 0.0 } } };
    advance(anim, 1.0, 0);
    EXPECT_EQ(anim.frame(), 0);
}

TEST(AnimatorTest, DegenerateClipsAreSafe)
{
    // desired fora da tabela: no-op (mesmo contrato do play(id) do audio).
    Animator anim{ { { 2, 0.10 } } };
    anim.update(kStep, 9);
    EXPECT_EQ(anim.clip(), 0u);

    // Ciclo multiframe com frameTime 0 seria um laco infinito — vira pose.
    Animator broken{ { { 4, 0.0 } } };
    advance(broken, 1.0, 0);
    EXPECT_EQ(broken.frame(), 0);
}

// =============================================================================
// Consumidores reais — a regra de proveniencia (ADR 0002)
// =============================================================================

// --- mario-bros @ 8dfbb90 (src/mario/anim/PlayerAnimator.{h,cpp}, task 03) ---
//
// O 1o consumidor: Idle(0) e Jump(2) sao poses unicas; Walk(1) cicla 2 frames
// a kWalkFrameTime = 0.12s. A selecao de la (ar manda, chao decide andar/
// parar) fica no jogo — aqui chega o clip ja escolhido.

TEST(AnimatorTest, MarioWalkCycleAtPointTwelveSeconds)
{
    enum class Clip : uint8_t
    {
        Idle = 0,
        Walk = 1,
        Jump = 2,
    };

    Animator anim{ { { 1, 0.0 }, { 2, 0.12 }, { 1, 0.0 } } };

    anim.update(kStep, Clip::Walk);
    ASSERT_EQ(anim.clipAs<Clip>(), Clip::Walk);
    ASSERT_EQ(anim.frame(), 0);

    advance(anim, 0.12, static_cast<ClipId>(Clip::Walk));
    EXPECT_EQ(anim.frame(), 1);
    advance(anim, 0.12, static_cast<ClipId>(Clip::Walk));
    EXPECT_EQ(anim.frame(), 0) << "WalkA/WalkB alternando, como no jogo";

    // Pulou: pose unica, e o retorno ao chao recomeca o andar do zero.
    anim.update(kStep, Clip::Jump);
    EXPECT_EQ(anim.frame(), 0);
    anim.update(kStep, Clip::Walk);
    EXPECT_EQ(anim.frame(), 0);
}

// --- zelda @ 3a3abda (src/zelda/anim/HeroAnimator.{h,cpp}, task 05) ---
//
// O 2o consumidor: a MESMA maquina, ate na cadencia (0.12s, 2 frames) — o
// vocabulario e que muda (Attack em vez de Jump; o golpe MANDA enquanto
// swordActive()). O facing nao passa por aqui: no zelda ele e fato de dominio
// (World::facing()), e a tabela de regioes do atlas o combina com o clip.

TEST(AnimatorTest, ZeldaAttackOverridesWalkAndRestartsIt)
{
    enum class Clip : uint8_t
    {
        Idle = 0,
        Walk = 1,
        Attack = 2,
    };

    Animator anim{ { { 1, 0.0 }, { 2, 0.12 }, { 1, 0.0 } } };

    advance(anim, 0.12 * 1.2, static_cast<ClipId>(Clip::Walk));
    ASSERT_EQ(anim.frame(), 1) << "no meio do ciclo de andar";

    // O golpe (0.22s de swordActive) manda mesmo andando; pose unica.
    advance(anim, 0.22, static_cast<ClipId>(Clip::Attack));
    EXPECT_EQ(anim.clipAs<Clip>(), Clip::Attack);
    EXPECT_EQ(anim.frame(), 0);

    // Passado o golpe, o andar recomeca no frame 0 — troca nao herda tempo.
    anim.update(kStep, Clip::Walk);
    EXPECT_EQ(anim.clipAs<Clip>(), Clip::Walk);
    EXPECT_EQ(anim.frame(), 0);
}
