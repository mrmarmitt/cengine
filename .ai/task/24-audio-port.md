# 24 - Audio como porta (`play(id)`), backend na plataforma

- **Status:** done (0.9.0, 2026-07-16) - implementada apos o gate disparar com
  2/2 evidencias (breakout + mario, copias quase identicas). Modulo
  `cengine::audio` header-only (INTERFACE), suite 91/91 com os dois
  consumidores reais transcritos (proveniencia); o mario e o consumidor de
  validacao da 0.9.0.
- **Prioridade:** concluida.
- **Categoria:** Arquitetura / porta da cengine (vocabulario), backend fica na
  plataforma.
- **Depende de:** nada estrutural. Depende de EVIDENCIA (2o jogo com som).
- **Breaking:** nao. Nasceria como porta NOVA opt-in (`cengine::audio`), como o
  `cengine::input` (task 20), sem tocar o resto.

## O corte que importa: PORTA (cengine) x BACKEND (plataforma)

Audio nao e uma coisa so — sao duas camadas, e so UMA e candidata a cengine:

- **A porta / vocabulario** (`play(SoundId)`; o jogo mapeia os `Events` do
  dominio para sons) — mecanismo puro, agnostico de plataforma. E ESTA que
  subiria para a cengine, **ao lado de routing / collision2d / input**.
- **O backend** (sintetizar ondas, mixar, pool de vozes, XAudio2) — Win32 /
  The-Forge. Fica na PLATAFORMA, nunca na cengine.

E exatamente o corte do **input** (task 20): a porta (vocabulario `Key` + fila)
mora na cengine; a CAPTURA (WndProc traduzindo `VK_*`) mora na plataforma. E o
oposto do **desenho**, que NAO virou porta — ha uma ponte so (`forgeui`), sem
duplicacao, entao ficou no common (ver breakout task 01).

## Avaliacao do gate - o breakout trouxe som, e o manteve no jogo

O breakout (task 06) foi o **primeiro e unico** jogo do ecossistema com audio:
sons sintetizados (ondas quadradas, sem arquivos), backend XAudio2, `AudioPlayer`
dentro do jogo. Ele NAO destravou uma porta de audio, e a decisao esta escrita
la:

**1. Uma evidencia nao sao duas (ADR 0002, criterio 2).** Nenhum outro jogo tem
som — nem os congelados. Contagem: UM.

**2. Sem duplicacao, nao ha o que consolidar.** O input subiu porque havia
**quatro copias** do enum `Key` (spaceinvaders, asteroids, breakout e a ponte) —
duplicacao real doendo. O audio tem UMA implementacao, em um jogo. "A copia e o
custo aceito" (corolario da ADR 0002, citado na task 06 do breakout).

**3. Adivinhar a forma da porta com um consumidor so e especular.** `play(id)`?
`play(id, volume)`? um pool exposto? prioridade de voz? Sem um segundo jogo para
corrigir o desenho, qualquer assinatura e chute — o que a ADR 0002 proibe.

**Gate:** comecar quando um **segundo** jogo precisar de som. O mario-bros e o
candidato natural (um plataforma "parece o original" quer pulo/moeda/power-up;
provavelmente um degrau de audio mais a frente). Com dois consumidores reais na
mao, a forma da porta se decide sozinha.

**Evidencia 1/2 registrada:** breakout — `BreakoutForge/audio/AudioPlayer.{h,cpp}`
(backend XAudio2, ilha Win32 pura) + `brk::Events` (o dominio RELATA fatos; a
cena decide o timbre). A parte PROMOVIVEL ja esta meio isolada: os `Events` sao
o vocabulario "o que aconteceu", e o `play` e a traducao para som.

## O gate disparou (2026-07-16) - o mario e a 2a evidencia, com copia quase identica

**Evidencia 2/2 registrada:** mario-bros @ `0fab493`,
`src/platform/theforge/src/MarioForge/audio/AudioPlayer.{h,cpp}` (task 05d do
mario, validada jogando). A copia do breakout foi **deliberadamente quase
literal** para medir ESTE gate: mudaram so o enum `Sound` (vocabulario do jogo:
Jump/Coin/Stomp/Hurt/Flag/GameOver vs PaddleHit/WallBounce/BrickBreak/
LifeLost/LevelUp) e as receitas das ondas (apresentacao). A MECANICA e identica
nos dois consumidores:

- `bool init()` sintetiza os sons e abre o device; falhar = jogo mudo (no-op);
- `void play(Sound)` — enum do jogo, disparo imediato, sem volume/prioridade;
- pool de vozes round-robin, PCM 16-bit mono, buffer apontando para os samples;
- a cena traduz os `Events` do dominio em `play()` (contados tocam por unidade);
- o player vive no casco (composition root) e atravessa as trocas de cena.

E o discriminador do input (4 copias do MESMO enum `Key`): duas copias da mesma
mecanica = mecanismo, nao politica. A forma da porta que os DOIS consumidores
confirmam e a minima: **`play(id)` e mais nada** — nenhum dos dois precisou de
volume, prioridade ou stop. O que continua sendo do jogo: o enum de sons, as
receitas de sintese e o mapeamento evento->som.

**Proximo passo (decisao do dono):** implementar a porta `cengine::audio`
(0.9.0?) com os criterios de aceite abaixo; os dois jogos viram consumidores
reais nos testes (regra de proveniencia). Ate la, as duas copias seguem como
estao — congeladas em evidencia.

## Objetivo (SE o gate disparar)

Uma **porta de audio** na cengine — um vocabulario de "toque este som", sem
backend:

```cpp
namespace cengine::audio {
// O jogo pede; a plataforma toca. A cengine nao sabe o que e uma onda quadrada,
// um device ou uma voz — so o contrato. Sem device, play() e no-op (mudo).
class Player {
public:
    virtual void play(SoundId id) = 0;
};
}
```

O dominio segue RELATANDO fatos (`Events`); a cena/jogo mapeia fato -> `SoundId`;
o backend na plataforma (XAudio2 hoje) produz o som. O jogo escolhe o timbre; a
engine so carrega o pedido.

## Fora do Escopo (o corte mecanismo x politica/plataforma)

- **O backend.** Sintese (ondas, envelope), mixagem, pool de vozes, formato PCM,
  XAudio2, COM — tudo plataforma. A guerra de headers The-Forge x XAudio2 (task
  06 do breakout) e prova de que isso e Win32 puro, longe da engine.
- **O catalogo de sons e o mapeamento evento->som** — politica do jogo.
- Musica, audio 3D/espacial, streaming, DSP, mixer com buses.

## Criterios Para Comecar

Nao implementar imediatamente. Comecar apenas quando:

- um **segundo** jogo precisar de som (mario-bros e o candidato); **e**
- o vocabulario minimo (`play(id)`?) puder ser confirmado por DOIS consumidores,
  e nao adivinhado por um.

## Criterios de Aceite (quando/se subir)

- [x] Porta `cengine::audio` opt-in, so vocabulario, sem backend nem
      dependencia de plataforma. (`modules/audio`, INTERFACE library,
      `CENGINE_BUILD_AUDIO`; um header: `Player.hpp` — `SoundId` + `Player`
      com `play(id)` virtual e acucar de enum.)
- [x] Backend (XAudio2) permanece na plataforma; a engine nao inclui header de
      som nenhum. (O AudioPlayer de cada jogo passa a IMPLEMENTAR a porta;
      sintese/pool/COM continuam la.)
- [x] **Regra de proveniencia:** testes de consumidor citam a origem
      (breakout @ 31dc850, mario @ 0fab493; arquivo+linhas) e transcrevem os
      valores dos jogos (os dois catalogos de sons, os eventos contados, o
      else-if gameOver/playerHit do mario).
- [x] "Mudo" e degradacao normal: sem device, `play()` vira no-op — contrato
      documentado no header e encarnado no teste `MuteIsNormalDegradation...`.
- [x] O dominio continua sem saber o que e um alto-falante (`Events` sao
      fatos; nenhum jogo mudou o dominio para consumir a porta).

## Decisoes de implementacao (2026-07-16)

- **`SoundId` e um numero (uint32_t), nao um enum da engine** — ao contrario
  do `Key` do input, os vocabularios de som DIFEREM entre jogos (Jump/Coin vs
  PaddleHit/Brick...); a engine carrega o pedido, nao o significado. O acucar
  `play(EnumDoJogo)` (template restrito a enums) mantem as cenas identicas ao
  que ja eram (`m_audio.play(Sound::Jump)`).
- **Interface de despacho imediato, nao fila** — os dois consumidores tocam na
  hora; uma fila exigiria uma bomba (pump) no loop que nenhum consumidor pediu.
- **Header-only (INTERFACE library)** — o contrato nao tem estado nem .cpp.

## Riscos

- Adivinhar a assinatura da porta com um consumidor so (especulacao, ADR 0002).
- Deixar o backend (sintese/XAudio2) vazar para a cengine e contaminar o core
  agnostico de plataforma com Win32.
- Promover o mapeamento evento->som (politica do jogo) junto com a porta.

## Relacionado

- ADR 0002 - criterio de promocao (2 evidencias, mecanismo x politica).
- Task 20 - input como porta: o PRECEDENTE exato (porta na cengine, backend na
  plataforma). Audio segue o mesmo corte.
- breakout, task 06 (audio) - a evidencia 1/2, que ja anteviu "porta da cengine
  `play(id)` com o backend na plataforma" quando houver o 2o jogo.
- mario-bros - candidato a 2a evidencia se ganhar som.
