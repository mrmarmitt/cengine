# 25 - Clip de animacao de sprite (frames sobre tempo)

- **Status:** done (0.10.0, 2026-07-18) — modulo `cengine::anim` extraido com o
  corte previsto: `Animator(tabela de ClipDesc {frameCount, frameTime})` +
  `update(dt, clip desejado)` — troca zera, so ciclos multiframe avancam,
  id fora da tabela e no-op (o contrato do play(id) do audio). O zelda e o
  consumidor de validacao (HeroAnimator vira casca de selecao sobre o modulo);
  o mario permanece pinado na 0.9.0 (evidencia transcrita nos testes). A
  contra-evidencia do spaceinvaders segue respeitada: quem anima por regra de
  dominio nao linka o modulo (opt-in, ADR 0001).
- **Categoria:** Arquitetura (modulo opt-in `cengine::anim`)
- **Registrada em:** 2026-07-17 (revisao pos-0.9.0, antes do 6o jogo)

## A candidata

Um "clip" de animacao dirigido pelo TEMPO: uma lista de frames + fps + loop, e
um cursor que avanca com `dt` e devolve o frame atual. Trocar de clip reseta o
cursor. E o nucleo do `PlayerAnimator` do mario-bros — a parte que NAO conhece
Idle/Walk/Jump.

## Evidencias (2/2 — gate disparado em 2026-07-18)

- **mario-bros @ 8dfbb90** (`src/mario/anim/PlayerAnimator.*`, task 03 do
  jogo): ciclo de frames sobre tempo, 1-frame para Idle/Jump, ciclo de 2 para
  Walk a 0.12s, troca de clip reseta para o frame zero. C++ puro, testado sem
  GPU (8 testes).
- **zelda @ 3a3abda** (`src/zelda/anim/HeroAnimator.*`, task 05 do jogo): a
  MESMA maquina, ate na cadencia (2 frames a 0.12s) — clips Idle/Walk/Attack.
  O que diverge e SELECAO e vocabulario (Attack por acao em vez de Jump por
  estado fisico; facing fora do animator, e fato de dominio la) — exatamente
  o que o corte manda deixar no jogo.

## Contra-evidencia (o sinal de divergencia, como na task 22)

- **spaceinvaders** anima SEM relogio: a pose do invasor deriva do passo da
  marcha (`World::animFrame()` = paridade de `m_steps`). Nao ha clip, nem fps,
  nem cursor — o "tempo" da animacao e o ritmo do proprio dominio. Se a engine
  tivesse um clip por tempo, o spaceinvaders NAO o usaria.
- breakout e asteroids nao tem animacao de frames (sprites estaticos /
  wireframe).

## O corte (se o gate um dia disparar)

- **Mecanismo (subiria):** clip = frames + fps + loop; cursor que avanca com
  `dt`; troca de clip reseta. Nenhum vocabulario de jogo.
- **Politica (fica no jogo):** QUAL clip tocar (Idle/Walk/Jump e a maquina de
  estados que escolhe), facing/espelho, o que e um "frame" na tabela de regioes
  do atlas (a tabela e do jogo, como decidido no forgesprite).

## Gate para comecar (ADR 0002)

Um 2o jogo com ciclo de frames dirigido pelo TEMPO, com o MESMO mecanismo
(discriminador do input: copias identicas, nao formas parecidas). Se o proximo
jogo animar por regra propria (como o spaceinvaders), e mais um voto de que
animar e politica — e esta task fecha como vetada.
