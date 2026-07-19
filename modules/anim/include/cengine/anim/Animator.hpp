#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

// cengine::anim (task 25): a maquina de CLIP DE ANIMACAO dirigida pelo tempo —
// clip atual + frame + acumulador; troca de clip zera; so ciclos multiframe
// avancam. Extraida das copias identicas do mario-bros (@ 8dfbb90,
// `src/mario/anim/PlayerAnimator.cpp`) e do zelda (@ 3a3abda,
// `src/zelda/anim/HeroAnimator.cpp`): o nucleo dos dois era o MESMO codigo,
// ate na cadencia (2 frames a 0.12s).
//
// O que NAO subiu (o corte da task 25): a SELECAO do clip (Idle/Walk/Jump no
// mario por estado fisico; Idle/Walk/Attack no zelda por acao — a maquina de
// estados que ESCOLHE e vocabulario de cada jogo), o facing/espelho e a tabela
// de regioes do atlas (do jogo, como decidido no forgesprite). E o
// contra-exemplo do spaceinvaders continua valendo: quem anima por regra de
// dominio (a pose derivada do passo da marcha) simplesmente nao linka este
// modulo — opt-in, ADR 0001.

namespace cengine::anim {

/// O id de um clip no CATALOGO DO JOGO. Como o SoundId da porta de audio: a
/// engine carrega o numero, nao o significado — qual clip existe (andar?
/// golpear?) e vocabulario de cada jogo.
using ClipId = std::uint32_t;

/// A forma de um clip: quantos frames e a cadencia. `frameTime` so importa com
/// mais de um frame (poses unicas nao avancam).
struct ClipDesc
{
    int    frameCount = 1;   // >= 1
    double frameTime = 0.0;  // segundos por frame (ciclos multiframe)
};

/// O cursor: recebe o clip DESEJADO (que o jogo escolheu pelos fatos dele) e o
/// dt, e responde "que frame e agora". Troca de clip comeca no frame 0 sem
/// herdar o tempo do anterior — a regra que os dois consumidores exigiam.
class Animator
{
public:
    /// A tabela de clips do jogo (indice = ClipId do catalogo dele).
    explicit Animator(std::vector<ClipDesc> clips, ClipId initial = 0);

    /// Avanca um passo. `desired` fora da tabela e ignorado (mesmo contrato do
    /// `play(id)` da porta de audio: chamar e sempre valido).
    void update(double dt, ClipId desired);

    /// Acucar para o enum do JOGO: `anim.update(dt, Clip::Walk)`.
    template <typename Clip>
        requires std::is_enum_v<Clip>
    void update(const double dt, const Clip desired)
    {
        update(dt, static_cast<ClipId>(desired));
    }

    [[nodiscard]] ClipId clip() const { return m_clip; }

    /// Frame ATUAL dentro do clip (0..frameCount-1).
    [[nodiscard]] int frame() const { return m_frame; }

    /// Acucar de leitura para o enum do jogo: `anim.clipAs<Clip>()`.
    template <typename Clip>
        requires std::is_enum_v<Clip>
    [[nodiscard]] Clip clipAs() const
    {
        return static_cast<Clip>(m_clip);
    }

private:
    std::vector<ClipDesc> m_clips;

    ClipId m_clip = 0;
    int    m_frame = 0;
    double m_elapsed = 0.0;
};

} // namespace cengine::anim
