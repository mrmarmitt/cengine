# 23 - Camera / viewport (projecao mundo -> tela + culling)

- **Status:** done (0.10.0, 2026-07-18) — modulo `cengine::camera2d` extraido
  com o corte previsto: `Viewport {origin, size, cullMargin}` + `worldToView`
  (subtracao) + `visible` (culling contra a janela inflada). A comparacao das
  copias confirmou: o `visible()` do mario e do zelda era IDENTICO linha a
  linha (so o namespace mudava); a formula subiu como estava. O zelda e o
  consumidor de validacao (Camera dele delega visible/projecao ao modulo; o
  `follow` de 2 eixos FICA no jogo); o mario permanece pinado na 0.9.0 (jogo
  congelado nao migra — a evidencia dele esta transcrita nos testes).
- **Prioridade:** concluida.
- **Categoria:** Arquitetura / possivel modulo novo opt-in (ou fica no jogo).
- **Depende de:** nada estrutural. Depende de EVIDENCIA (consumidores reais).
- **Breaking:** nao. Nasceria como modulo opt-in (ex.: `cengine::camera2d`) ou
  simplesmente ficaria no jogo — a decisao e do gate.

## Por que esta task nasceu (historico)

O mario-bros foi escolhido, entre os plataformas, **pela fronteira da camera**:
e a unica lacuna arquitetural que nenhum jogo do ecossistema tocou. 8puzzle,
spaceinvaders, asteroids e breakout cabem TODOS numa tela — nenhum projetou
mundo -> tela nem fez culling. O mario e o primeiro cujo nivel PRECISA exceder a
janela (senao a camera nunca aparece).

Esta task nao manda construir nada: ela **registra a candidata** para o
aprendizado nao se perder entre os projetos, do mesmo jeito que a task 18 (scene
stack) ficou parada esperando o caso real.

## Avaliacao do gate (2026-07-15) - o mario ganhou camera, e ela ficou no jogo

O degrau 4 do mario-bros trouxe a **1a evidencia**: nivel 64 colunas (maior que a
tela), camera que rola suave seguindo o jogador, com culling. Ela NAO destravou a
promocao, por dois motivos:

**1. Uma evidencia nao sao duas (ADR 0002, criterio 2).** E o primeiro e unico
jogo do ecossistema com nivel maior que a tela. Contagem: UM.

**2. A camera ja NASCEU partida em duas metades, e so uma e candidata.** O
`mario::Camera` separou de proposito:
  - a **transformada mundo->janela + culling** — mecanismo puro, agnostico. E o
    que poderia subir.
  - o **seguimento** (ancora do foco, travar nos limites do nivel) — FEEL. O
    mario rola suave e horizontal; um metroidvania saltaria por sala; um jogo de
    nave centraria diferente. Promover um seguimento daria a engine uma opiniao
    sobre o genero.

Ou seja: mesmo com um 2o consumidor, so a transformada+culling subiria; o
seguimento de cada jogo fica no jogo. Enquanto houver so o mario, nem isso — a
copia (uma projecao por jogo) e o custo aceito.

**Evidencia 1/2 registrada:** mario-bros @ commit `4a8f825`,
`src/mario/camera/Camera.{h,cpp}` (a transformada + `visible()` de culling; o
`follow()` e o feel) e `ForgeGameScene::viewport/drawSprite` (a projecao subtrai
a camera). 7 testes de camera no jogo.

**Novo gate:** comecar quando um **2o** consumidor precisar de projecao/culling
com o MESMO modelo; o seguimento nunca sobe.

## Reavaliacao com o Zelda (2026-07-18) - gate DISPARADO

O Zelda, task 03, fornece a **2a evidencia real**. Sua masmorra 40x24 excede a
viewport nos dois eixos; `zelda::Camera` guarda origem e tamanho da janela,
`visible()` testa o retangulo do mundo contra a area visivel com margem, e a
cena projeta com `tela = (mundo - camera) * escala + centralizacao`. E o mesmo
mecanismo do Mario, agora exercido tambem no eixo vertical.

**Evidencia 2/2 registrada:** zelda, task 03 (done e validada jogando),
`src/zelda/camera/Camera.{h,cpp}` +
`ForgeGameScene::viewport/drawBox`, com 7 testes de camera.

A variacao confirma o corte previsto: `follow()` e politica — Mario segue em X,
Zelda segue em X e Y — enquanto origem da viewport, subtracao mundo->janela e
culling sao iguais. Portanto o gate do ADR 0002 esta satisfeito **somente para
transformada+culling**. A extracao deve manter ancora, limites e comportamento
de seguimento nos jogos.

## Contexto

Hoje a projecao "mundo -> pixels de tela" vive em cada jogo (ex.: o `toScreen`
das cenas Forge do breakout, o `viewport` da cena do mario no degrau 2). Enquanto
o mundo cabe na tela, isso e so um enquadramento fixo (escala + centralizacao),
e nao ha nada a promover.

A camera vira mecanismo de verdade quando o nivel excede a janela: uma
transformada mundo -> tela com **deslocamento** (a rolagem), mais **culling** do
que esta fora do enquadramento. A transformada e pura e reutilizavel; o
comportamento de COMO a camera se move nao e.

## Objetivo (gate disparado; falta desenhar e executar a extracao)

Oferecer o recorte comum observado: origem+tamanho da viewport, transformada
mundo -> viewport por subtracao e culling de retangulos visiveis, sem decidir
COMO a camera segue o alvo. Escala, letterbox e centralizacao em pixels ainda
vivem nas cenas; a comparacao das copias decide se alguma parte tambem pertence
ao mecanismo:

```cpp
namespace cengine::camera2d {
// A origem e calculada pelo jogo (follow/deadzone/etc.); este tipo so projeta e
// consulta visibilidade.
struct Viewport { Vec2 origin; Vec2 size; float cullMargin; };
[[nodiscard]] Vec2 worldToView(const Viewport&, Vec2 world);
[[nodiscard]] bool visible(const Viewport&, Aabb worldRect);
}
```

O jogo continua dono de PARA ONDE a camera olha (seguir o jogador, look-ahead,
deadzone, travar nos limites do nivel) — isso e feel, e feel e politica.

## Fora do Escopo (o corte mecanismo x politica)

- **O comportamento de seguimento.** Follow suave, look-ahead na direcao do
  movimento, deadzone, travar nas bordas do nivel, "camera de sala" que salta —
  cada sub-genero tem o seu (foi por isso que o mario, e nao um metroidvania,
  foi escolhido: para ter UM modelo de camera claro). Promover um seguimento
  seria dar a engine uma opiniao sobre o genero do jogo.
- Parallax, multiplas camadas, split-screen, shake, zoom dinamico.
- Ownership de entidades ou do alvo.

## Criterios para comecar

Os criterios de evidencia estao satisfeitos:

- [x] consumidor real com nivel > tela e rolagem: mario-bros, degrau 4;
- [x] segundo consumidor com o mesmo modelo de projecao/culling: zelda, task 03;
- [ ] comparar as duas copias e definir a API minima de transformada+culling,
      deixando todo seguimento nos jogos.

## Criterios de Aceite (quando/se subir)

- [x] Transformada mundo -> viewport pura (e culling), opt-in, testavel sem GPU
      (`modules/camera2d`, 6 testes).
- [x] **Regra de proveniencia:** testes de consumidor real citam a origem
      (mario-bros @ 4a8f825 Camera.cpp:27-32; zelda @ 9658ae0 Camera.cpp:31-36)
      e transcrevem os valores dos jogos (margem 16; view 320x180 do zelda,
      masmorra 640x384; rolagem horizontal do mario).
- [x] A engine NAO decide para onde a camera olha: nenhum seguimento/deadzone
      embutido — os `follow()` (1 eixo no mario, 2 no zelda) ficaram nos jogos.
- [x] O modelo de tela unica (enquadramento fixo) continua trivial para jogos
      que nao rolam (origem zero = identidade; e o modulo e opt-in).

## Riscos

- Promover o SEGUIMENTO (feel) de um jogo como se fosse universal.
- Alargar o escopo agora que o gate disparou e incluir seguimento, shake ou
  parallax sem evidencia para esses mecanismos.

## Relacionado

- ADR 0002 - criterio de promocao (2 evidencias, mecanismo x politica).
- mario-bros, degrau 4 (done) - evidencia 1/2; a fronteira-titulo do jogo.
- zelda, task 03 (done) - evidencia 2/2; mesmo modelo em dois eixos.
- Task 18 (scene stack) - outra candidata que ficou estacionada esperando o caso
  real; mesmo padrao.
