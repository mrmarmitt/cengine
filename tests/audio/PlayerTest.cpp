#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include <cengine/audio/Player.hpp>

// O contrato da porta de audio. Sem backend nenhum: o teste implementa a porta
// como a plataforma implementaria (gravando os pedidos) e chama como uma cena
// chamaria.
//
// A ultima secao e a regra de proveniencia da ADR 0002: a forma `play(id)` veio
// de DUAS copias quase identicas de AudioPlayer — breakout @ 31dc850 e
// mario-bros @ 0fab493 — e a suite encarna o uso REAL de cada um, com os
// valores transcritos do jogo e a origem citada, para serem conferiveis e nao
// lembrancas de quem promoveu.

using cengine::audio::Player;
using cengine::audio::SoundId;

namespace {

/// A porta implementada como a plataforma implementaria — menos o device: em
/// vez de tocar, grava. E o suficiente para provar o contrato.
class RecordingPlayer final: public Player
{
public:
    std::vector<SoundId> played;

    void play(const SoundId id) override { played.push_back(id); }
    using Player::play; // o acucar de enum continua visivel no tipo concreto
};

/// O backend SEM device: todo play() e um no-op seguro. E o "mudo" que os dois
/// consumidores reais ja praticavam (init() falho => jogo roda calado).
class MutePlayer final: public Player
{
public:
    void play(SoundId) override {} // nada — e isso e o contrato, nao um bug
};

} // namespace

// =============================================================================
// O contrato
// =============================================================================

TEST(AudioPlayerTest, PlayCarriesTheGameSoundId)
{
    RecordingPlayer player;

    player.play(SoundId{ 3 });
    player.play(SoundId{ 0 });

    ASSERT_EQ(player.played.size(), 2u);
    EXPECT_EQ(player.played[0], 3u);
    EXPECT_EQ(player.played[1], 0u) << "id 0 e um som como outro qualquer, nao um 'nenhum'";
}

TEST(AudioPlayerTest, EnumSugarTranslatesToTheUnderlyingId)
{
    // O enum e do JOGO — a engine nunca o conhece. O acucar so evita o cast nas
    // cenas; o que atravessa a porta e o numero.
    enum class Sound : uint8_t
    {
        Jump = 0,
        Coin = 1,
    };

    RecordingPlayer player;
    Player&         port = player; // como a cena ve: so a porta

    port.play(Sound::Coin);
    port.play(Sound::Jump);

    ASSERT_EQ(player.played.size(), 2u);
    EXPECT_EQ(player.played[0], 1u);
    EXPECT_EQ(player.played[1], 0u);
}

TEST(AudioPlayerTest, MuteIsNormalDegradationNotAnError)
{
    MutePlayer player;
    Player&    port = player;

    // Sem device, chamar continua VALIDO — o jogo nao pode quebrar por falta
    // de alto-falante. Se isto nao lancar nem travar, o contrato esta de pe.
    for (SoundId id = 0; id < 100; ++id)
    {
        port.play(id);
    }
    SUCCEED();
}

// =============================================================================
// Consumidores reais — a regra de proveniencia (ADR 0002)
// =============================================================================

// --- breakout @ 31dc850 (BreakoutForge/audio/AudioPlayer.h + scene/ForgeGameScene.cpp:35-60) ---
//
// O 1o consumidor. O catalogo de la: PaddleHit=0, WallBounce=1, BrickBreak=2,
// LifeLost=3, LevelUp=4. A cena traduz os Events CONTADOS do dominio em um
// play() POR UNIDADE: "dois tijolos no mesmo quadro tocam dois blips — e o
// jogo soa como o que aconteceu, nao como uma media" (comentario de la).

TEST(AudioPlayerTest, BreakoutCountedEventsPlayOncePerUnit)
{
    enum class Sound : uint8_t
    {
        PaddleHit = 0,
        WallBounce = 1,
        BrickBreak = 2,
        LifeLost = 3,
        LevelUp = 4,
    };

    // Um quadro real possivel: a bola quica na raquete e leva DOIS tijolos.
    const uint32_t brickBreaks = 2;
    const uint32_t paddleHits = 1;
    const bool     lifeLost = false;

    RecordingPlayer player;
    Player&         port = player;

    for (uint32_t i = 0; i < brickBreaks; ++i)
    {
        port.play(Sound::BrickBreak);
    }
    for (uint32_t i = 0; i < paddleHits; ++i)
    {
        port.play(Sound::PaddleHit);
    }
    if (lifeLost)
    {
        port.play(Sound::LifeLost);
    }

    ASSERT_EQ(player.played.size(), 3u) << "dois tijolos soam como dois, mais a raquete";
    EXPECT_EQ(player.played[0], 2u);
    EXPECT_EQ(player.played[1], 2u);
    EXPECT_EQ(player.played[2], 0u);
}

// --- mario-bros @ 0fab493 (MarioForge/audio/AudioPlayer.h + scene/ForgeGameScene.cpp, playSounds) ---
//
// O 2o consumidor — a copia que disparou o gate. O catalogo de la: Jump=0,
// Coin=1, Stomp=2, Hurt=3, Flag=4, GameOver=5. Detalhe de politica que a porta
// NAO conhece e o jogo decide na traducao: gameOver e playerHit sao um
// else-if (a descida do game over ja conta a dor; tocar Hurt junto seria eco).

TEST(AudioPlayerTest, MarioTranslatesTheFrameEventsIntoPlays)
{
    enum class Sound : uint8_t
    {
        Jump = 0,
        Coin = 1,
        Stomp = 2,
        Hurt = 3,
        Flag = 4,
        GameOver = 5,
    };

    // O ultimo quadro de uma partida perdida: o jogador encosta no goomba com a
    // ultima vida (playerHit E gameOver no mesmo quadro — hitPlayer marca os
    // dois; valores do World real).
    const bool playerHit = true;
    const bool gameOver = true;

    RecordingPlayer player;
    Player&         port = player;

    if (gameOver)
    {
        port.play(Sound::GameOver);
    }
    else if (playerHit)
    {
        port.play(Sound::Hurt);
    }

    ASSERT_EQ(player.played.size(), 1u) << "o else-if da cena: game over engole o som de dano";
    EXPECT_EQ(player.played[0], 5u);
}

TEST(AudioPlayerTest, MarioTwoCoinsInAFrameSoundLikeTwo)
{
    enum class Sound : uint8_t
    {
        Jump = 0,
        Coin = 1,
    };

    // Pulo que atravessa duas moedas vizinhas no mesmo quadro: o evento contado
    // (Events::coinsCollected = 2) vira dois dings, mais o pulo que o lancou.
    const bool     jumped = true;
    const uint32_t coinsCollected = 2;

    RecordingPlayer player;
    Player&         port = player;

    if (jumped)
    {
        port.play(Sound::Jump);
    }
    for (uint32_t i = 0; i < coinsCollected; ++i)
    {
        port.play(Sound::Coin);
    }

    ASSERT_EQ(player.played.size(), 3u);
    EXPECT_EQ(player.played[0], 0u);
    EXPECT_EQ(player.played[1], 1u);
    EXPECT_EQ(player.played[2], 1u);
}
