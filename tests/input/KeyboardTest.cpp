#include <gtest/gtest.h>

#include <cengine/input/Keyboard.hpp>

// O contrato de teclado. Sem plataforma nenhuma: o teste empurra eventos como a
// WndProc empurraria, e le como uma cena leria.
//
// A ultima secao e o PEDAGIO da Emenda 1 da ADR 0002: das quatro copias que
// justificaram esta promocao, duas vivem em jogos ESTACIONADOS (8puzzle e
// spaceinvaders) que nunca vao linkar o modulo. A suite tem de encarnar o uso
// deles — com a origem citada, para que os valores sejam conferiveis e nao
// lembrancas minhas.

using cengine::input::Key;
using cengine::input::KeyEvent;
using cengine::input::Keyboard;

// =============================================================================
// A fila de edges
// =============================================================================

TEST(KeyboardTest, EmptyQueueReadsNone)
{
    Keyboard keyboard;

    EXPECT_EQ(keyboard.readKey().key, Key::None);
}

TEST(KeyboardTest, ReadsAtMostOneEventPerCall)
{
    Keyboard keyboard;

    keyboard.pushKey({ Key::Down });
    keyboard.pushKey({ Key::Down });
    keyboard.pushKey({ Key::Enter });

    // A semantica que importa: uma cena que chama input() uma vez por quadro
    // consome UM evento. Sem isso, segurar a seta atravessaria o menu inteiro
    // num quadro so.
    EXPECT_EQ(keyboard.readKey().key, Key::Down);
    EXPECT_EQ(keyboard.readKey().key, Key::Down);
    EXPECT_EQ(keyboard.readKey().key, Key::Enter);
    EXPECT_EQ(keyboard.readKey().key, Key::None);
}

TEST(KeyboardTest, EventsComeOutInTheOrderTheyWereTyped)
{
    Keyboard keyboard;

    keyboard.pushKey({ Key::Char, 'A' });
    keyboard.pushKey({ Key::Char, 'B' });
    keyboard.pushKey({ Key::Backspace });

    EXPECT_EQ(keyboard.readKey().character, 'A');
    EXPECT_EQ(keyboard.readKey().character, 'B');
    EXPECT_EQ(keyboard.readKey().key, Key::Backspace);
}

TEST(KeyboardTest, CharEventCarriesTheCharacter)
{
    Keyboard keyboard;

    keyboard.pushKey({ Key::Char, 'x' });

    const KeyEvent event = keyboard.readKey();
    EXPECT_EQ(event.key, Key::Char);
    EXPECT_EQ(event.character, 'x');
}

TEST(KeyboardTest, FullQueueDropsTheNewEventNotTheOldOnes)
{
    Keyboard keyboard;

    for (size_t i = 0; i < Keyboard::kQueueMax; ++i)
    {
        keyboard.pushKey({ Key::Char, 'a' });
    }
    keyboard.pushKey({ Key::Escape }); // nao cabe

    // O que ja estava na fila foi digitado ANTES e sai primeiro; descartar o
    // velho embaralharia a ordem do jogador.
    for (size_t i = 0; i < Keyboard::kQueueMax; ++i)
    {
        EXPECT_EQ(keyboard.readKey().character, 'a') << "posicao " << i;
    }
    EXPECT_EQ(keyboard.readKey().key, Key::None) << "o evento que estourou a fila nao entrou";
}

// =============================================================================
// O estado segurado
// =============================================================================

TEST(KeyboardTest, KeysAreNotHeldByDefault)
{
    const Keyboard keyboard;

    EXPECT_FALSE(keyboard.isHeld(Key::Left));
    EXPECT_FALSE(keyboard.isHeld(Key::Space));
}

TEST(KeyboardTest, HeldStatePersistsUntilTheNextPush)
{
    Keyboard keyboard;

    keyboard.pushHeldKey(Key::Right, true);
    EXPECT_TRUE(keyboard.isHeld(Key::Right));

    // A plataforma NAO republica a cada quadro: o estado tem de persistir.
    EXPECT_TRUE(keyboard.isHeld(Key::Right));

    keyboard.pushHeldKey(Key::Right, false);
    EXPECT_FALSE(keyboard.isHeld(Key::Right));
}

TEST(KeyboardTest, HeldStateIsIndependentPerKey)
{
    Keyboard keyboard;

    keyboard.pushHeldKey(Key::Left, true);

    EXPECT_TRUE(keyboard.isHeld(Key::Left));
    EXPECT_FALSE(keyboard.isHeld(Key::Right));
}

TEST(KeyboardTest, ClearHeldKeysReleasesEverything)
{
    Keyboard keyboard;

    keyboard.pushHeldKey(Key::Up, true);
    keyboard.pushHeldKey(Key::Space, true);

    keyboard.clearHeldKeys(); // a janela perdeu o foco: o KEYUP nunca vem

    EXPECT_FALSE(keyboard.isHeld(Key::Up));
    EXPECT_FALSE(keyboard.isHeld(Key::Space)) << "sem isto a nave sai voando sozinha ao voltar o foco";
}

TEST(KeyboardTest, HeldKeysAndTheQueueAreIndependent)
{
    Keyboard keyboard;

    keyboard.pushHeldKey(Key::Left, true);

    // Segurar nao enfileira: as duas leituras respondem perguntas diferentes.
    EXPECT_EQ(keyboard.readKey().key, Key::None);
    EXPECT_TRUE(keyboard.isHeld(Key::Left));

    // E consumir a fila nao solta o que esta segurado.
    keyboard.pushKey({ Key::Escape });
    EXPECT_EQ(keyboard.readKey().key, Key::Escape);
    EXPECT_TRUE(keyboard.isHeld(Key::Left));
}

TEST(KeyboardTest, HeldAxisAnswersMinusOneZeroOrOne)
{
    Keyboard keyboard;

    EXPECT_FLOAT_EQ(keyboard.heldAxis(Key::Left, Key::Right), 0.0f) << "nenhuma segurada";

    keyboard.pushHeldKey(Key::Left, true);
    EXPECT_FLOAT_EQ(keyboard.heldAxis(Key::Left, Key::Right), -1.0f);

    keyboard.pushHeldKey(Key::Right, true);
    EXPECT_FLOAT_EQ(keyboard.heldAxis(Key::Left, Key::Right), 0.0f) << "as duas: parado, nao esquizofrenico";

    keyboard.pushHeldKey(Key::Left, false);
    EXPECT_FLOAT_EQ(keyboard.heldAxis(Key::Left, Key::Right), 1.0f);
}

// =============================================================================
// Consumidores reais — o pedagio da Emenda 1 (ADR 0002)
// =============================================================================
//
// Cada cena abaixo e transcrita do codigo do jogo que a originou, com a fonte
// citada. Codigo congelado nao se mexe, entao a citacao nao apodrece.

// --- 8puzzle @ ESTACIONADO (src/platform/ftxui/Keyboard.h) ---
//
// A origem do vocabulario: setas + Enter/Escape/Backspace/Char, e o contrato
// "consome no maximo uma tecla por chamada" ja escrito no comentario de la.

TEST(KeyboardTest, EightPuzzleMenuNavigation)
{
    Keyboard keyboard;

    // O jogador aperta baixo, baixo, ENTER. A cena de menu de la roda um
    // input() por quadro — entao SAO tres quadros, e nao um.
    keyboard.pushKey({ Key::Down });
    keyboard.pushKey({ Key::Down });
    keyboard.pushKey({ Key::Enter });

    int selected = 0;
    int frames = 0;
    bool confirmed = false;

    for (; frames < 10 && !confirmed; ++frames)
    {
        switch (keyboard.readKey().key) // um por quadro
        {
        case Key::Down:
            ++selected;
            break;
        case Key::Enter:
            confirmed = true;
            break;
        default:
            break;
        }
    }

    EXPECT_TRUE(confirmed);
    EXPECT_EQ(selected, 2);
    EXPECT_EQ(frames, 3) << "tres teclas, tres quadros: e o contrato da fila";
}

// --- spaceinvaders @ bb4e9b1 (src/platform/theforge/.../ForgeUi.h:62-76) ---
//
// A origem do estado SEGURADO. La a ponte publicava um par ja mastigado —
// `pushHeldState(float moveAxis, bool fireHeld)` — porque o canhao so andava
// para os lados e so tinha um gatilho. Este teste prova que o mecanismo
// generico (por tecla) EXPRESSA aquele par: se nao expressasse, a promocao
// estaria errada.

TEST(KeyboardTest, SpaceInvadersCannonAxisAndTrigger)
{
    Keyboard keyboard;

    // WndProc: setas seguradas + espaco.
    keyboard.pushHeldKey(Key::Left, true);
    keyboard.pushHeldKey(Key::Space, true);

    EXPECT_FLOAT_EQ(keyboard.heldAxis(Key::Left, Key::Right), -1.0f) << "o moveAxis() de la";
    EXPECT_TRUE(keyboard.isHeld(Key::Space)) << "o fireHeld() de la";

    // Solta a esquerda, segura a direita: o canhao inverte.
    keyboard.pushHeldKey(Key::Left, false);
    keyboard.pushHeldKey(Key::Right, true);

    EXPECT_FLOAT_EQ(keyboard.heldAxis(Key::Left, Key::Right), 1.0f);
}

// --- asteroids @ main (platform-theforge-common, ForgeUi.h) ---
//
// O consumidor VIVO, e a prova de que o contrato EVOLUI: o asteroids precisou
// de DOIS eixos (girar e acelerar) e de Key::Space — o par mastigado do
// spaceinvaders nao daria conta, e foi isso que forcou o mecanismo por tecla.

TEST(KeyboardTest, AsteroidsShipUsesTwoAxesAndAHeldTrigger)
{
    Keyboard keyboard;

    keyboard.pushHeldKey(Key::Right, true); // girar
    keyboard.pushHeldKey(Key::Up, true);    // acelerar
    keyboard.pushHeldKey(Key::Space, true); // atirar

    EXPECT_FLOAT_EQ(keyboard.heldAxis(Key::Left, Key::Right), 1.0f);
    EXPECT_TRUE(keyboard.isHeld(Key::Up));
    EXPECT_TRUE(keyboard.isHeld(Key::Space));

    // E o ESC (edge) continua chegando pela fila, sem atrapalhar o que esta
    // segurado — as duas leituras convivem no mesmo quadro.
    keyboard.pushKey({ Key::Escape });
    EXPECT_EQ(keyboard.readKey().key, Key::Escape);
    EXPECT_TRUE(keyboard.isHeld(Key::Up));
}

TEST(KeyboardTest, AsteroidsRecordNameIsTypedThroughTheQueue)
{
    Keyboard keyboard;

    // Digitar o nome no recorde: Char + Backspace, um por quadro.
    keyboard.pushKey({ Key::Char, 'M' });
    keyboard.pushKey({ Key::Char, 'X' });
    keyboard.pushKey({ Key::Backspace });
    keyboard.pushKey({ Key::Char, 'R' });
    keyboard.pushKey({ Key::Enter });

    std::string name;
    bool        done = false;

    while (!done)
    {
        const KeyEvent event = keyboard.readKey();
        switch (event.key)
        {
        case Key::Char:
            name += event.character;
            break;
        case Key::Backspace:
            if (!name.empty())
            {
                name.pop_back();
            }
            break;
        case Key::Enter:
        case Key::None:
            done = true;
            break;
        default:
            break;
        }
    }

    EXPECT_EQ(name, "MR");
}
