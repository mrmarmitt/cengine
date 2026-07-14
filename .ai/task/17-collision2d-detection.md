# 17 - Colisao 2D: deteccao AABB opt-in

- **Status:** estacionada - gate avaliado e REPROVADO em 2026-07-14 (ver
  "Avaliacao do gate"); novo gate definido no fim desta task.
- **Prioridade:** baixa/media - so deve subir quando houver repeticao real em
  mais de um jogo consumidor.
- **Categoria:** Arquitetura / modulo opcional
- **Depende de:** 14 done (tempo no loop / fixed timestep). Consumidor
  candidato: Space Invaders, depois de fechar a PoC do jogo.
- **Breaking:** nao na primeira versao, se entrar como modulo novo opt-in
  (`cengine::collision2d`) sem alterar `core` ou `routing`.

## Avaliacao do gate (2026-07-14) - o asteroids disparou, e reprovou

O criterio de comeco "um segundo jogo precisar da mesma logica AABB" disparou
quando o asteroids chegou na colisao (task 03 daquele repo). Avaliado contra a
ADR 0002, o candidato **reprova no criterio 2 (>= 2 consumidores reais)**, por
dois motivos que so ficaram visiveis com o consumidor na mao:

1. **O consumidor que justificava a promocao nao existe mais como consumidor.**
   O spaceinvaders tem colisao AABB, mas esta ESTACIONADO na 0.5.0 (ADR 0003):
   ele nunca vai linkar `cengine::collision2d`. A evidencia dele e historica,
   nao viva. Promover agora produz um modulo com UM consumidor real.
2. **A forma que o asteroids precisa nao e a que o SI evidencia.** Rocha e nave
   sao redondas: o asteroids pede **circulo x circulo** (e com distancia
   TOROIDAL, porque a arena da wrap). AABB em corpos que giram erra de forma
   visivel. Ou seja: o AABB evidenciado pelo SI nao serve ao asteroids, e o
   circulo que o asteroids quer tem ZERO evidencia previa — promove-lo seria
   exatamente a especulacao que a ADR 0002 proibe.

**Decisao:** a colisao (circulo, toroidal) fica no `ast::World`, no dominio do
asteroids. Nenhuma linha sobe para a engine neste ciclo. Custo aceito: quando
a promocao acontecer, havera duplicacao entre um jogo vivo e um congelado — o
que a ADR 0002 ja declara preferivel a uma abstracao sem consumidor.

**Novo gate (substitui os "Criterios Para Comecar" abaixo):** comecar quando um
**segundo jogo VIVO** (nao estacionado) precisar de deteccao 2D. Ai havera duas
formas reais na mesa (AABB e circulo) e a pergunta certa a responder deixa de
ser "AABB ou circulo?" e passa a ser "qual a forma minima que serve aos dois?".

## Contexto

O Space Invaders mostrou a primeira necessidade concreta de colisao no
ecossistema: tiro contra invasor, bomba contra jogador, limites de arena e
invasao da horda. Hoje isso vive corretamente no dominio do jogo (`World`),
porque as consequencias sao regras de gameplay: pontuar, perder vida, trocar
onda ou encerrar a partida.

Ainda assim, existe uma parte puramente geometrica que pode se repetir em
outros jogos: representar AABBs 2D e detectar intersecoes/overlaps. Essa parte
pode virar um modulo pequeno da cengine no futuro, desde que a engine nao passe
a possuir entidades nem a decidir regras do jogo.

## Objetivo

Desenhar um modulo opcional de **deteccao** de colisao 2D, nao um motor de
fisica completo.

A engine deve responder perguntas como:

```cpp
bool intersects(const Aabb& a, const Aabb& b);
std::vector<CollisionHit> detectOverlaps(...);
```

O jogo continua responsavel por:

- possuir e atualizar as entidades;
- registrar/remover objetos colidiveis quando fizer sentido;
- decidir o que uma colisao significa;
- mover, matar, pontuar, aplicar dano ou encerrar a partida.

## Escopo Proposto

### Degrau 1 - geometria minima

Criar um modulo/header pequeno com:

```cpp
namespace cengine::collision2d {

struct Aabb {
    float x;
    float y;
    float w;
    float h;
};

bool intersects(const Aabb& a, const Aabb& b);

}
```

Esse degrau e barato, facil de testar e nao muda o desenho dos jogos.

### Degrau 2 - mundo de colisao simples

So executar se outro jogo repetir a necessidade. Um desenho possivel:

```cpp
namespace cengine::collision2d {

using ColliderId = uint32_t;

struct CollisionHit {
    ColliderId a;
    ColliderId b;
};

class CollisionWorld {
public:
    void clear();
    void add(ColliderId id, Aabb bounds, uint32_t layer, uint32_t mask);
    std::vector<CollisionHit> detectOverlaps() const;
};

}
```

Esse objeto nao possui entidades do jogo. Ele recebe ids e AABBs calculados
pelo consumidor, filtra por layer/mask e devolve pares em colisao.

## Decisoes Iniciais

- **Registro/remocao de entidades:** responsabilidade do jogo (`World` ou
  equivalente), nao da cengine.
- **Posse dos objetos:** responsabilidade do jogo.
- **Layers/masks:** categorias de colisao para evitar testar tudo contra tudo.
  Exemplo: `PlayerShot` colide com `Invader`; `EnemyBomb` colide com `Player`.
- **Resolucao:** fora do escopo. A engine so detecta; o jogo resolve.
- **Dimensao:** apenas 2D no primeiro desenho.
- **Tempo:** o modulo nao controla timestep. Deve ser chamado de dentro do
  `update(dt)` do jogo, que ja roda com fixed timestep pela task 14.
- **Entrega de eventos:** preferir modelo pull (`detectOverlaps()` retorna
  hits) em vez de callback, por ser mais simples e testavel.

## Fora do Escopo

- Physics engine completo.
- Resolucao de movimento/penetracao.
- Corpos rigidos, massa, impulso, gravidade ou atrito.
- Sweep/continuous collision detection na primeira versao.
- Scene graph, ownership de entidades ou integracao obrigatoria com
  `cengine::core`.
- 3D.

## Criterios Para Comecar

Nao implementar imediatamente. Comecar esta task apenas quando pelo menos uma
das condicoes abaixo for verdadeira:

- um segundo jogo precisar da mesma logica AABB;
- o Space Invaders comecar a duplicar consultas geometricas em mais de um
  lugar;
- houver uma necessidade clara de layers/masks que torne o `World` ruidoso.

## Criterios de Aceite

- [ ] Modulo `cengine::collision2d` opt-in, sem dependencia de `core` ou
      `routing`.
- [ ] Testes unitarios cobrindo intersecao, nao-intersecao, bordas e filtros
      layer/mask se o degrau 2 entrar.
- [ ] README documenta que a engine detecta colisao, mas regras e ownership
      continuam no jogo.
- [ ] Consumidor real validado sem mover regra de gameplay para a cengine.

## Riscos

- A engine capturar responsabilidade demais cedo: ownership, callbacks e regras
  de gameplay devem ficar fora.
- Criar uma abstracao maior que a necessidade real. O degrau 1 deve ser
  preferido enquanto so houver colisao AABB simples.
- Confundir colisao com fisica. Esta task e de deteccao 2D, nao de simulacao
  fisica.

## Relacionado

- Task 14 - fixed timestep: garante que a simulacao que chama colisao seja
  previsivel.
- Space Invaders task 01 / degrau 5: primeiro consumidor real com colisao de
  gameplay.
- ADR 0001 - core minimo + modulos opcionais: colisao deve nascer como modulo
  opt-in, fora do core.
