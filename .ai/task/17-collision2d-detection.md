# 17 - Colisao 2D: deteccao AABB opt-in

- **Status:** todo
- **Prioridade:** baixa/media - so deve subir quando houver repeticao real em
  mais de um jogo consumidor.
- **Categoria:** Arquitetura / modulo opcional
- **Depende de:** 14 done (tempo no loop / fixed timestep). Consumidor
  candidato: Space Invaders, depois de fechar a PoC do jogo.
- **Breaking:** nao na primeira versao, se entrar como modulo novo opt-in
  (`cengine::collision2d`) sem alterar `core` ou `routing`.

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
