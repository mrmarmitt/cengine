# ADR 0002 — Critério de promoção de código para a engine (filtro anti-depósito)

- **Status:** Aceito
- **Data:** 2026-07-13
- **Contexto de decisão:** com dois jogos completos consumindo a cengine
  (8puzzle e spaceinvaders), o fechamento da PoC de sprites gerou uma leva
  de candidatos a promoção — e a pergunta: o que entra na engine e o que
  fica nos jogos?

## Contexto

O fim da PoC do Space Invaders (task 01 daquele repo) expôs código repetido
entre os dois jogos consumidores: recordes (Record/Service/Repository TSV
quase idênticos), a fachada de navegação (GameRouter/StateGameFlow), o
vocabulário de input (`Key`/`KeyEvent` copiado em três pontes de
plataforma), primitivas de colisão AABB, utilitários de tempo (timer de
cadência, cooldown) e RNG determinístico.

A tentação natural é promover tudo que se repete. O risco nomeado na
discussão: **transformar a cengine em um depósito de código** — uma engine
que cresce por acúmulo, não por responsabilidade. Duplicação entre jogos
não transfere responsabilidade para a engine automaticamente: engine boa
fornece **mecanismo**; **política e dados são do jogo** (coerente com o
ADR 0001: core mínimo, módulos opt-in, jogo dono das regras).

## Decisão

Um candidato só entra na cengine se passar nos **três** critérios:

1. **Mecanismo puro** — zero vocabulário de jogo no código promovido
   (nenhum `menu`, `gameOver`, `score`, `record`...). Se o nome de um
   conceito do jogo aparece, é política: fica no jogo.
2. **≥ 2 evidências reais de necessidade** — o código já existe, escrito à mão,
   em pelo menos dois jogos/plataformas. Extração, nunca especulação.

   **Um jogo estacionado (ADR 0003) continua valendo como evidência.** Congelar
   um repositório suspende a *manutenção* dele, não o *aprendizado* que ele
   produziu: o Space Invaders provou a necessidade de colisão AABB, e essa prova
   não expira porque ele parou de receber commits. Emenda de 2026-07-14, ver
   "Emenda 1" abaixo.
3. **Testável dentro da própria cengine** — a suíte da engine cobre o
   mecanismo sem precisar de um jogo ou de uma plataforma gráfica.

Tasks de promoção devem citar este ADR e demonstrar os três critérios no
contexto. Candidatos que falham em algum critério podem ser registrados
como task **estacionada com gate explícito** (critérios objetivos de
começar), nunca implementados "para já deixar pronto".

## Emenda 1 (2026-07-14) — evidência congelada conta, mas paga um pedágio

**Contexto.** Ao chegar na colisão, o asteroids disparou o gate da task 17 e o
critério 2 foi lido como "≥ 2 consumidores que vão *linkar* o módulo". Com o
Space Invaders estacionado, a leitura reprovava a promoção — e, levada a sério,
reprovaria *toda* promoção futura, já que os dois jogos completos do ecossistema
estão congelados. A ADR 0003 chama esses jogos de **documentação viva**; se o
aprendizado deles não puder alimentar a evolução estrutural, congelá-los é só
abandoná-los.

**Decisão.** O critério 2 mede **evidência de necessidade**, não contrato de
link. Um consumidor estacionado conta.

**Pedágio.** O critério dos dois consumidores fazia um segundo trabalho, calado:
dois consumidores *vivos* forçam a API a ser geral e a validam de verdade. Com
um deles congelado, nada impede a API de nascer torta para o consumidor que
ninguém compila. Para recuperar isso **sem descongelar o jogo**, toda promoção
apoiada em evidência congelada tem uma obrigação extra:

> A suíte da cengine deve **encarnar o caso de uso do consumidor congelado** —
> um teste que reproduz, com o mecanismo promovido, a situação real daquele
> jogo (ex.: tiro × invasor do Space Invaders sobre o `collision2d`). Se o
> mecanismo não consegue expressar o caso do jogo congelado, ele não está
> pronto para subir.

Assim a "documentação viva" da ADR 0003 deixa de ser metáfora e vira teste
executável: o aprendizado do jogo pronto fica reforçado *dentro* da estrutura,
que é o propósito de tê-lo congelado em vez de apagado.

**Limite que a emenda NÃO afrouxa.** Evidência congelada não dispensa o critério
1. Continua valendo o corte mecanismo × política — e ele é feito por conceito,
não por arquivo. Exemplo do próprio `collision2d`: as **formas** e o teste de
sobreposição entre elas (AABB, círculo) são mecanismo e sobem; o **wrap-around**
da arena do asteroids é topologia do mundo daquele jogo — política — e fica no
jogo, que corrige a posição antes de perguntar à engine. Promover o toro junto
daria à engine uma opinião sobre o formato do mundo: é assim que uma engine vira
depósito.

Corolário: duplicação real entre jogos que *falha* no critério 1 tem outro
destino — uma lib compartilhada de jogos (game-commons) ou a aceitação da
cópia; nunca a engine.

## Consequências

Aplicações do filtro na leva que originou esta decisão:

| Candidato | Veredito | Critério decisivo |
|---|---|---|
| Colisão AABB (task 17) | ✅ aprovado (a *detecção*; consequências ficam no jogo) | passa nos 3 |
| FlowRouter — mecânica da fachada (task 19) | ✅ aprovado (vocabulário fica nos jogos) | passa nos 3 |
| IWindowManager obrigatório (task 21) | ✅ aprovado (mecanismo do core) | passa nos 3 |
| Recordes (Record/Service/TSV) | ❌ fica nos jogos | falha no 1 (métrica/persistência são política do jogo) |
| Vocabulário de input (task 20) | ⏸ estacionada com gate | falha no 2 (cenas por plataforma: sem consumidor para a porta) |
| Pontes ForgeUi/ForgeSpriteUi | ❌ destino seria platform-common | falha no 1 e no 3 (conhecem GPU/The-Forge) |

Custo aceito: alguma duplicação convive nos jogos até o filtro liberar a
promoção — é preferível a abstração sem consumidor.
