# 18 - Scene stack e overlays

- **Status:** todo
- **Prioridade:** baixa/media - so deve subir quando houver consumidor real
  precisando de pause menu, modal, inventario, debug overlay ou telas
  sobrepostas.
- **Categoria:** Arquitetura / routing
- **Depende de:** 13 done (Router x Repository separados), 15 done (modo
  hospedado) e 16 done (present no fim do quadro).
- **Breaking:** provavelmente sim se substituir ou alterar o contrato atual do
  `routing`; pode nao ser breaking se nascer como API paralela opt-in.

## Contexto

O modulo `routing` atual modela uma cena ativa por vez: um estado corrente
resolve para uma factory de cena, `requestState()` agenda a proxima cena e o
commit no fim do frame substitui a cena atual. Esse desenho e suficiente para
fluxos lineares como:

```text
splash -> menu -> game -> gameOver -> records -> exit
```

Ele nao expressa bem casos comuns em jogos maiores:

- pause menu aberto sobre o jogo;
- inventario/modal por cima da gameplay;
- dialogo por cima da cena;
- console/debug overlay que nao bloqueia o jogo;
- HUD separado da cena de gameplay;
- menu que desenha o jogo ao fundo sem destruir a cena ativa.

Nesses casos a intencao nao e trocar a cena atual, mas empilhar ou compor
camadas com regras diferentes de input, update e draw.

## Objetivo

Investigar uma evolucao do `routing` para suportar **stack de cenas** e/ou
**overlays**, mantendo o modelo atual de substituicao simples para jogos que
nao precisam disso.

A engine pode oferecer infraestrutura generica:

- `replace` para troca total;
- `push` para abrir uma cena sobre outra;
- `pop` para fechar o topo;
- politica de propagacao de `input`, `update` e `draw`;
- commit seguro das operacoes no fim do frame.

O jogo continua responsavel por decidir quando abrir/fechar overlays e o que
isso significa para o dominio.

## Desenho Inicial

Um desenho possivel, ainda nao aprovado:

```cpp
struct SceneLayerPolicy {
    bool blocksInputBelow = true;
    bool updatesBelow = false;
    bool drawsBelow = true;
};

class SceneStack {
public:
    void replace(std::unique_ptr<IScene> scene);
    void push(std::unique_ptr<IScene> scene, SceneLayerPolicy policy = {});
    void pop();

    void input();
    void update(core::Seconds dt);
    void draw();
};
```

Exemplos de politica:

```text
Pause menu:
  blocksInputBelow = true
  updatesBelow = false
  drawsBelow = true

Debug overlay:
  blocksInputBelow = false
  updatesBelow = true
  drawsBelow = true

Modal bloqueante:
  blocksInputBelow = true
  updatesBelow = false
  drawsBelow = true
```

## Perguntas de Design

- O stack substitui o `RouterInMemory` ou nasce como API paralela?
- Estado (`IState`) ainda e a abstracao correta para overlays, ou overlays
  devem ser cenas diretas?
- `onEnter/onExit` bastam ou precisamos de `onCovered/onUncovered`,
  `onPause/onResume`?
- Operacoes de stack podem acontecer durante `input`, `update` e `draw`, ou
  sempre sao agendadas para commit no fim do frame?
- O topo sempre recebe input primeiro? Input pode ser consumido?
- `update` deve rodar de baixo para cima ou so nas camadas permitidas?
- `draw` deve sempre rodar de baixo para cima nas camadas visiveis?
- Como representar o estado de exit quando existe stack?
- Como testar ordem e lifetime sem acoplar a uma plataforma?

## Escopo Proposto

### Degrau 1 - documentar o caso real

Antes de implementar, validar em um consumidor real. Exemplos:

- pause menu no Space Invaders;
- debug overlay de FPS/sprites;
- modal de recorde sobre a cena de game over;
- inventario em um proximo jogo.

Se o caso for simples e exclusivo de um jogo, preferir implementar dentro do
proprio jogo primeiro.

### Degrau 2 - prototipo opt-in

Criar uma API paralela ou experimental que nao quebre o router atual. O foco e
testar:

- ordem de input/update/draw;
- push/pop/replace agendados;
- policies de propagacao;
- lifetime das cenas.

### Degrau 3 - integrar ao routing

So depois do prototipo e de um consumidor real. Decidir se o modulo passa a
expor:

- router de cena unica;
- scene stack;
- ou uma abstracao comum para ambos.

## Fora do Escopo

- ECS.
- Scene graph visual.
- Layout de UI.
- Sistema de janelas/modal completo.
- Ownership de recursos de renderizacao.
- Reescrever o `routing` atual sem consumidor real.

## Criterios Para Comecar

Nao implementar imediatamente. Comecar esta task apenas quando uma necessidade
concreta aparecer:

- um jogo precisa abrir uma tela sem destruir a cena de baixo;
- o jogo precisa desenhar gameplay atras de menu/pause;
- ha overlay que deve desenhar por cima sem bloquear update;
- dois consumidores diferentes implementarem solucao parecida localmente.

## Criterios de Aceite

- [ ] Caso real documentado antes do desenho final.
- [ ] API deixa claro quando input/update/draw propagam para camadas abaixo.
- [ ] Operacoes de stack sao commitadas em ponto previsivel do frame.
- [ ] Testes cobrem ordem de chamadas, push/pop/replace e lifetime.
- [ ] Modelo atual de cena unica continua disponivel ou tem caminho de
      migracao claro.
- [ ] Nenhuma regra de gameplay e movida para a cengine.

## Riscos

- Fazer a engine crescer para um caso que um jogo resolveria localmente com
  menos complexidade.
- Misturar navegacao de telas com estado de gameplay. Pause, inventario e
  dialogo ainda podem ter regra de dominio propria no jogo.
- Callbacks/lifetime complicarem o roteamento antes de existir necessidade
  real.
- Quebrar consumidores simples que so precisam de troca de cena unica.

## Relacionado

- Task 13 - separacao Router x Repository.
- Task 15 - modo hospedado: o stack precisa funcionar tanto em `start()` quanto
  em `frame(dt)`.
- Task 16 - present no fim do quadro: overlays devem respeitar a ordem de
  desenho antes da apresentacao.
- Task 19 - FlowRouter (fachada de navegacao): metade ORTOGONAL do routing;
  se esta task mudar a semantica (push/pop), cresce o vocabulario das
  fachadas dos jogos, nao a mecanica extraida la — sem conflito.
- Space Invaders - candidato futuro se ganhar pause menu ou debug overlay.
