#pragma once

#include <cstdint>
#include <type_traits>

// A PORTA de audio (task 24): o jogo pede "toque este som"; a plataforma toca.
//
// A cengine nao sabe o que e uma onda quadrada, um device, uma voz ou um mixer —
// so o CONTRATO. E o mesmo corte do input (task 20): la o vocabulario (Key +
// fila) mora na engine e a CAPTURA (WndProc) na plataforma; aqui o vocabulario
// (`play(id)`) mora na engine e a SINTESE/reproducao (XAudio2, pool de vozes)
// na plataforma.
//
// A forma veio de DOIS consumidores reais com copias quase identicas —
// breakout @ 31dc850 e mario-bros @ 0fab493 (`audio/AudioPlayer.{h,cpp}` em
// ambos) — e e deliberadamente MINIMA: `play(id)` e mais nada. Nenhum dos dois
// precisou de volume, prioridade, stop ou musica; quando um consumidor real
// precisar, a porta cresce com a evidencia (ADR 0002), nao antes.

namespace cengine::audio {

/// O id de um som no CATALOGO DO JOGO. A engine carrega o pedido, nao o
/// significado: qual som existe (pulo? tijolo?), como ele soa e quando tocar
/// sao decisoes do jogo — por isso um numero, e nao um enum da engine (ao
/// contrario do `Key` do input, os vocabularios de som DIFEREM entre jogos).
using SoundId = std::uint32_t;

/// O contrato que as CENAS falam. A plataforma implementa (backend XAudio2,
/// sintese, pool de vozes — nada disso entra aqui); o composition root injeta.
///
/// "Mudo" e degradacao NORMAL, nao erro: sem device de som, `play()` e um
/// no-op seguro — o contrato exige que chamar sempre seja valido. Placar,
/// fisica e fluxo nunca dependem de um alto-falante.
class Player
{
public:
    Player() = default;
    virtual ~Player() = default;

    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;

    /// Toca o som `id` do catalogo do jogo. Disparo imediato ("fire and
    /// forget"): nao ha handle, stop nem status — os dois consumidores reais
    /// confirmaram que o arcade nao precisa de mais.
    virtual void play(SoundId id) = 0;

    /// Acucar para o enum do JOGO: `player.play(Sound::Jump)` em vez do cast a
    /// cada chamada. So traduz o enum para o id — o despacho e o virtual acima.
    template <typename Sound>
        requires std::is_enum_v<Sound>
    void play(const Sound sound)
    {
        play(static_cast<SoundId>(sound));
    }
};

} // namespace cengine::audio
