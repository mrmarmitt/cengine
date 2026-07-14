# 20 - Vocabulario de input como porta da cengine

- **Status:** todo - **PRE-REQUISITO DO PROXIMO JOGO** (decisao do dono em
  2026-07-14, ao encerrar o asteroids). Sai da geladeira: o proximo jogo nasce
  depois dela, e e ele quem valida a porta como consumidor vivo.
- **Prioridade:** media (era baixa/estacionada).

## Atualizacao 2026-07-14 — o gate mudou de figura

O gate original ("falta consumidor: cenas sao por plataforma") foi escrito
quando so existiam os dois jogos hoje CONGELADOS. Desde entao:

- o `enum Key`/`KeyEvent` foi copiado uma quarta vez, agora no
  `platform-theforge-common` (`ForgeUi.h`), que virou a ponte viva de todos os
  jogos The-Forge;
- a **Emenda 1 da ADR 0002** passou a aceitar evidencia de jogo estacionado —
  entao as copias do 8puzzle e do spaceinvaders CONTAM;
- o asteroids acrescentou vocabulario novo ao contrato (`Key::Space`, o estado
  SEGURADO `isHeld`/`heldAxis`), provando que o contrato EVOLUI e que hoje ele
  evolui em copias, sem dono.

Ou seja: o criterio 2 (>= 2 evidencias) esta folgado, e o criterio 1 (mecanismo
puro) sempre esteve — `Key`/`KeyEvent` nao tem vocabulario de jogo. O que faltava
era um consumidor VIVO para validar a porta; e ele agora tem nome: o proximo
jogo. Cuidar do pedagio da Emenda 1: a suite da cengine deve encarnar o uso dos
jogos congelados (fila de edges: no maximo um evento por `input()`).
- **Categoria:** Arquitetura / porta nova
- **Depende de:** nenhuma task; depende de uma DECISAO de arquitetura
  (cenas compartilhadas entre plataformas) que ainda nao foi tomada.
- **Breaking:** nao se nascer como porta nova opt-in.

## Contexto

O vocabulario de teclado que as cenas consomem existe hoje copiado em tres
lugares:

- 8puzzle `src/platform/ftxui/Keyboard.h`
- 8puzzle `src/platform/theforge/src/8PuzzleForge/ForgeUi.h`
- spaceinvaders `src/platform/theforge/src/SpaceInvadersForge/ForgeUi.h`

O conteudo repetido e o CONTRATO, nao a captura:

- enum `Key` (Up/Down/Left/Right/Enter/Escape/Backspace/Char/...) +
  `KeyEvent` (tecla + caractere);
- a semantica da fila: no maximo um evento consumido por `input()` de cena;
- o estado continuo que o spaceinvaders adicionou no degrau 5:
  eixo de movimento segurado (`moveAxis`) + gatilho segurado (`fireHeld`).

A CAPTURA (bindings do The-Forge, `inputGetValue`, WndProc, ftxui events) e
100% da plataforma e NUNCA entra na cengine — a engine continua sem saber o
que e teclado fisico, GPU ou janela.

## Por que esta estacionada

O desenho provado nos dois jogos e: dominio compartilhado, CENAS POR
PLATAFORMA (o 8puzzle tem tres conjuntos de cenas, um por plataforma).
Enquanto as cenas forem codigo de plataforma, o tipo `Key` morar na
plataforma e coerente — promover o vocabulario economizaria so a copia do
enum, sem destravar nada.

A promocao passa a pagar quando (e se) surgir o objetivo "escrever a cena
UMA vez e rodar em N plataformas": as cenas compartilhadas precisariam
consumir input (e desenho) por portas da cengine. Essa e uma mudanca
arquitetural maior, que deve nascer de uma necessidade real, nao desta
task.

## Criterios Para Comecar

Comecar APENAS quando pelo menos um destes for verdade:

- decisao explicita de compartilhar cenas entre duas plataformas reais;
- um terceiro conjunto de cenas duplicando o mesmo enum comecar a divergir
  (bug real causado pela divergencia, nao estetica);
- a fase 2 do 8puzzle (modo biblioteca) precisar padronizar input por outro
  motivo.

## Escopo Proposto (quando destravar)

1. Porta `cengine::input` (ou no core): `Key`, `KeyEvent`, contrato da fila
   (1 evento por `input()`), eixo/gatilho segurados.
2. Plataformas preenchem a porta (ForgeUi/Keyboard viram implementacoes).
3. Se o objetivo for cena compartilhada, avaliar a porta de desenho junto —
   input sozinho nao compartilha cena.

## Fora do Escopo

- Captura de input (bindings, WndProc, gamepad) — sempre da plataforma.
- Mapeamento/rebinding de teclas.
- Porta de desenho (avaliada junto, mas com decisao propria).

## Criterios de Aceite

- [x] Gate documentado foi atingido: quarta copia do enum (no
      platform-theforge-common), Emenda 1 da ADR 0002 (evidencia congelada
      conta) e o contrato EVOLUINDO em copias (Key::Space + estado segurado
      nascidos no asteroids). Registrado na secao "Atualizacao 2026-07-14".
- [x] Vocabulario unico na cengine (`cengine::input`, 0.8.0), plataformas sem
      copia local (o `ForgeUi` do common passa a delegar — common 0.3.0).
- [x] Nenhuma API de plataforma vazando pela porta (o modulo nao inclui nada
      de janela/GPU/terminal; a captura fica 100% na plataforma).
- [ ] Cenas de pelo menos um jogo consumindo a porta: **breakout** (o proximo
      jogo) — fecha quando o casco dele subir. O asteroids ja consome
      indiretamente, via o ForgeUi que agora delega.

## Pedagio da Emenda 1 (ADR 0002)

Das quatro copias que justificaram a promocao, duas vivem em jogos
ESTACIONADOS. A suite da cengine encarna o uso deles, com a origem citada:

- **8puzzle** (`src/platform/ftxui/Keyboard.h`): navegacao de menu — tres
  teclas, TRES quadros (o contrato da fila).
- **spaceinvaders** (`@bb4e9b1`, `ForgeUi.h:62-76`): o par ja mastigado
  `pushHeldState(moveAxis, fireHeld)` do canhao, mostrado como EXPRESSAVEL pelo
  mecanismo generico por tecla — se nao fosse, a promocao estaria errada.
- **asteroids** (vivo): dois eixos + gatilho segurado + o nome do recorde
  digitado pela fila. E a prova de que o contrato evolui.

## Riscos

- Promover cedo demais e criar abstracao sem consumidor — exatamente o
  "deposito de codigo" que a cengine nao deve virar. O gate existe para
  isso.

## Relacionado

- Task 15/16 - o modo hospedado/biblioteca definem quem e dono do loop e da
  janela; a porta de input completaria o trio se cenas compartilhadas
  virarem objetivo.
- spaceinvaders degrau 5 - origem do requisito de tecla segurada
  (`moveAxis`/`fireHeld`) que qualquer porta futura precisa cobrir.
