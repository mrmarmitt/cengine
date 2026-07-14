# 17 - Colisao 2D: deteccao opt-in (AABB + circulo)

- **Status:** done (0.7.0)
- **Prioridade:** media - o asteroids precisa dela para existir.
- **Categoria:** Arquitetura / modulo opcional
- **Depende de:** 14 done (tempo no loop / fixed timestep).
- **Evidencias reais:** Space Invaders (AABB — tiro x invasor, bomba x jogador;
  jogo ESTACIONADO, evidencia valida pela Emenda 1 da ADR 0002) e Asteroids
  (circulo — tiro x rocha, nave x rocha; consumidor VIVO).
- **Breaking:** nao. Entra como modulo novo opt-in (`cengine::collision2d`),
  sem tocar em `core` nem em `routing`.

## Historico do gate

O gate foi disparado pelo asteroids em 2026-07-14 e **reprovado de primeira**,
por uma leitura do criterio 2 da ADR 0002 que exigia dois consumidores *vivos*
(o Space Invaders esta congelado). O dono do projeto corrigiu a regra: congelar
um jogo suspende a manutencao dele, nao o aprendizado que ele produziu — e sem
isso a ADR 0003 ("documentacao viva") vira abandono, ja que os dois jogos
completos do ecossistema estao parados.

A **Emenda 1 da ADR 0002** oficializou: evidencia congelada conta, com um
pedagio — a suite da cengine tem de **encarnar o caso de uso do jogo congelado**
(ver Criterios de Aceite). Com isso o candidato passa nos tres criterios e a
task saiu da geladeira.

## Objetivo

Um modulo opcional de **deteccao** de colisao 2D — nao um motor de fisica.

A engine responde `intersects(forma, forma)`. O jogo continua responsavel por
possuir e mover as entidades, decidir o que uma colisao significa, pontuar,
matar e encerrar a partida.

## Escopo

```cpp
namespace cengine::collision2d {

struct Vec2   { float x, y; };
struct Aabb   { float x, y, w, h; };   // canto superior esquerdo + tamanho
struct Circle { Vec2 center; float radius; };

[[nodiscard]] bool intersects(const Aabb& a, const Aabb& b);
[[nodiscard]] bool intersects(const Circle& a, const Circle& b);
[[nodiscard]] bool intersects(const Circle& c, const Aabb& b);
[[nodiscard]] bool intersects(const Aabb& b, const Circle& c);

}
```

## Fora do Escopo (o corte mecanismo x politica)

- **Wrap-around / topologia da arena.** A arena do asteroids e um toro; a do
  Space Invaders nao. Formato do mundo e **politica do jogo**: o consumidor
  corrige a posicao (menor delta no toro) e SO ENTAO pergunta a engine. Promover
  o toro daria a cengine uma opiniao sobre o formato do mundo — e ai ela viraria
  deposito (ADR 0002, limite da Emenda 1).
- `CollisionWorld` com ids/layers/masks (o "degrau 2" do desenho antigo): nenhum
  consumidor precisou. So se a necessidade aparecer.
- Resolucao de colisao, fisica, corpos rigidos, sweep/CCD, 3D.
- Ownership de entidades ou integracao obrigatoria com `core`.

## Criterios de Aceite

- [x] Modulo `cengine::collision2d` opt-in
      (`CENGINE_BUILD_COLLISION2D`), sem dependencia de `core` ou `routing`.
- [x] Testes cobrindo intersecao, nao-intersecao e as BORDAS (encostar conta?)
      das tres combinacoes de forma.
- [x] **Pedagio da Emenda 1 — a suite encarna o caso do jogo congelado:** um
      teste reproduz tiro x invasor / bomba x jogador do Space Invaders sobre o
      modulo, provando que o mecanismo expressa o caso dele sem descongela-lo.
- [x] **Consumidor vivo validado:** o asteroids reproduz tiro x rocha e nave x
      rocha, com o wrap-around ficando no jogo.
- [x] README documenta que a engine DETECTA colisao; regra e ownership seguem
      no jogo.

## Relacionado

- ADR 0002 (+ Emenda 1) - criterio de promocao; ADR 0003 - consumidores
  estacionados.
- Asteroids task 03 - o consumidor vivo que trouxe o circulo.
