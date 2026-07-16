# 22 - Colisao 2D: resolucao (penetracao / MTV)

- **Status:** estacionada - gate avaliado em 2026-07-15 (mario-bros) e NAO
  disparado; 1 de 2 evidencias, E com sinal de que "resolver" pode ser politica
  de jogo, nao mecanismo. Ver "Avaliacao do gate".
- **Prioridade:** baixa/media - so deve subir quando um segundo consumidor
  precisar de penetracao/MTV **e resolver do MESMO jeito** que o primeiro.
- **Categoria:** Arquitetura / extensao do modulo `collision2d`
- **Depende de:** 17 done (deteccao AABB + circulo).
- **Breaking:** nao. Entraria como funcao NOVA opt-in do `collision2d`
  (penetracao/MTV puro), sem tocar o `intersects` existente.

## Avaliacao do gate (2026-07-15) - o mario resolveu, e nao precisou disto

O `collision2d` 0.7.0 responde so `bool intersects(...)` — nenhuma penetracao,
nenhum vetor de separacao. O mario-bros (degrau 2, dominio) foi o **primeiro**
consumidor do ecossistema a precisar de colisao com **RESOLUCAO** (quanto
empurrar o corpo para fora, por qual face) e nao so deteccao. Ele NAO destravou
a task, por dois motivos:

**1. Uma evidencia nao sao duas (ADR 0002, criterio 2).** Nenhum outro jogo VIVO
precisou de resolucao. O breakout resolve a *reflexao* dele (a bola quadrada) —
mas isso e outra coisa (inverter um vetor de velocidade), e mora no jogo
(`World::reflectOff`). A contagem de evidencias de RESOLUCAO POSICIONAL e UM: o
mario.

**2. Sinal mais forte: os dois jogos que "resolvem" resolvem DIFERENTE.** E o
ponto que mais empurra esta task para longe da engine:

  - o **breakout** resolve por **menor eixo de penetracao (MTV de forma-unica)**,
    porque a bola chega em diagonal e o que importa e por qual lado ela entrou;
  - o **mario** resolve **eixo-separado** (move X e resolve, move Y e resolve) —
    e nem precisou do MTV: a direcao do movimento do quadro ja diz por qual face
    empurrar. E a tecnica padrao de plataforma de tiles.

Se cada jogo resolve com uma estrategia diferente, "resolver" cheira a
**politica de jogo**, nao a mecanismo unico promovivel. Promover o MTV agora
daria a engine uma opiniao (eixo-separado? menor penetracao? swept?) que o
proximo consumidor pode nao compartilhar — exatamente o risco de deposito que a
ADR 0002 barra.

O que a engine PODERIA oferecer sem tomar essa opiniao e a **penetracao pura** (o
vetor de separacao minimo entre dois AABB), deixando a ESTRATEGIA de resolver no
jogo. Mas mesmo isso so se paga com um segundo consumidor querendo o mesmo
calculo — hoje o mario nem usa MTV.

**Novo gate (mais afiado):** comecar quando um **segundo** consumidor precisar de
penetracao/MTV **E** a forma de consumir esse calculo for a MESMA do primeiro
(mecanismo comum), e nao duas politicas divergentes. Enquanto cada jogo resolver
o contato do seu jeito, o calculo fica no jogo.

**Evidencia 1/2 registrada:** mario-bros, `mario::World::resolveHorizontal` /
`resolveVertical` (resolucao eixo-separado; nao usa MTV). O breakout NAO conta
como evidencia desta task — ele resolve reflexao, nao penetracao posicional, e
ja o faz no jogo (`reflectOff`).

## Reavaliacao ao fechar o mario (2026-07-16) - gate MANTIDO

O mario terminou completo (degraus 1-5: goombas, moedas, vidas, bandeira,
recordes por pontos/tempo) e o dono perguntou se valia uma task para "mover a
fisica para a cengine". Veredito: **nao** — a fisica de plataforma e feel
(constantes tunadas em playtest) + politica (estrategias de resolucao), e o
unico recorte de mecanismo dela e ESTA task; registro no bloco "Consideradas e
vetadas" do README do backlog.

O jogo completo ate ADICIONOU evidencia contra promover a resolucao: o **pisao
no goomba** (degrau 5a) e mais um contato "de cima" resolvido com regra propria
do jogo (matar + pulinho + so na metade superior do corpo), irmao da one-way e
diferente de ambos os anteriores. Tres formas de "resolver" em dois jogos, todas
politica. O gate segue: 2o consumidor consumindo penetracao/MTV do MESMO jeito.

## Contexto

`cengine::collision2d` e um modulo de **deteccao**: `intersects(forma, forma) ->
bool` (task 17). Um plataforma precisa de mais para os corpos NAO se
atravessarem: dado que dois AABB se sobrepoem, *quanto* e *em que direcao*
separar. Isso e penetracao (um vetor), nao um booleano.

## Objetivo (SE o gate disparar)

Oferecer o **calculo de penetracao/MTV puro** — um vetor de separacao minimo
entre duas formas — sem resolver nada:

```cpp
namespace cengine::collision2d {
// Vetor de separacao minimo: quanto mover `a` para deixar de tocar `b`.
// nullopt quando nao se tocam. NAO move, NAO decide eixo, NAO decide o que a
// colisao significa — isso continua no jogo.
[[nodiscard]] std::optional<Vec2> penetration(const Aabb& a, const Aabb& b);
}
```

A engine responde "quanto se penetram?"; o jogo continua dono de mover a
entidade, escolher a estrategia (eixo-separado vs MTV), zerar velocidade, marcar
`grounded`, tocar som.

## Fora do Escopo (o corte mecanismo x politica)

- **A ESTRATEGIA de resolucao.** Eixo-separado (mario) vs menor-penetracao
  (breakout) vs swept/CCD e escolha do jogo — depende do movimento e do gosto.
  Promover uma delas seria dar a engine uma opiniao sobre COMO resolver.
- Mover entidades, zerar velocidade, `grounded`, plataforma one-way, pisar no
  inimigo — tudo politica, fica no jogo.
- Fisica, corpos rigidos, impulsos, atrito, sweep/CCD, 3D.
- Ownership de entidades.

## Criterios Para Comecar

Nao implementar imediatamente. Comecar apenas quando:

- um **segundo** consumidor precisar de penetracao/MTV; **e**
- os dois consumirem o calculo do MESMO jeito (mesmo mecanismo), e nao com
  estrategias de resolucao divergentes — se divergirem, e politica e fica no
  jogo.

## Criterios de Aceite (quando/se subir)

- [ ] `penetration()`/MTV puro no `collision2d`, opt-in, sem tocar `intersects`.
- [ ] Testes cobrindo penetracao, nao-penetracao e as BORDAS (encostar).
- [ ] **Regra de proveniencia:** os testes de consumidor real citam a origem
      (repo @ commit, arquivo, linha) e transcrevem os valores do jogo.
- [ ] A engine NAO resolve: nenhuma entidade movida, nenhuma estrategia de eixo
      embutida.
- [ ] README deixa claro: a engine DETECTA e mede penetracao; RESOLVER e do jogo.

## Riscos

- Promover a ESTRATEGIA de um jogo (eixo-separado ou MTV) como se fosse
  universal, quebrando o proximo consumidor que resolve diferente.
- Deslizar de "medir penetracao" para "resolver colisao" e virar meio motor de
  fisica — fora do escopo do modulo de deteccao (ADR 0001/0002).

## Relacionado

- Task 17 - deteccao AABB/circulo (a base deste modulo).
- ADR 0002 - criterio de promocao (2 evidencias, mecanismo x politica).
- mario-bros, degrau 2 (`.ai/task/02-dominio-andar-pular-cair.md`) - o consumidor
  que resolve eixo-separado; evidencia 1/2.
- breakout, `World::reflectOff` - resolve reflexao (nao penetracao) no jogo;
  ilustra que "resolver contato" ja apareceu em duas formas DIFERENTES.
