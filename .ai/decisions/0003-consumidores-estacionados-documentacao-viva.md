# ADR 0003 — Consumidores estacionados como documentação viva

- **Status:** Aceito
- **Data:** 2026-07-13
- **Contexto de decisão:** abertura do ciclo 0.6.0 (tasks 21 e 19), com o
  jogo asteroids prestes a nascer como terceiro consumidor da cengine.

## Contexto

O plano original do ciclo 0.6.0 previa migrar os dois jogos consumidores
(8puzzle, com fases 1 e 2 da PoC The-Forge, e spaceinvaders) como critério
de aceite das tasks 21 (`IWindowManager` obrigatório) e 19 (`FlowRouter`).

Ao decidir o ciclo, o dono do projeto escolheu outro destino: **8puzzle e
spaceinvaders ficam onde estão**, pinados na cengine **0.5.0**, servindo de
documentação viva das etapas que validaram (fases da PoC The-Forge, modo
hospedado com `nullptr`, fachada GameRouter duplicada). O custo de manter
os jogos acompanhando cada bump da engine não paga o retorno — o valor de
PoC deles já foi extraído — e o asteroids assume o papel de consumidor de
validação do 0.6.0 em diante.

## Decisão

1. **8puzzle e spaceinvaders não migram para a cengine >= 0.6.0.** Eles
   permanecem consumindo a 0.5.0 (checkout irmão `../cengine` na tag
   `0.5.0`, ou a versão equivalente via FetchContent) e não entram mais nos
   critérios de aceite de tasks futuras da engine.
2. **O asteroids é o consumidor de validação ativo** do ciclo 0.6.0 e dos
   próximos, até que outro jogo entre no ecossistema.
3. Os critérios de aceite das tasks 21 e 19 que citavam "migrar os
   consumidores" ficam **satisfeitos pela suíte da própria cengine + o
   asteroids** como primeiro consumidor real do novo desenho.

## Consequências

- **Modo hospedado sem consumidor ativo:** o 8puzzle fase 1 era o único
  usuário real de `frame(dt)` com host externo. Estacionado, o modo
  hospedado passa a viver dos testes da cengine (6 testes da task 15 +
  os de construção da task 21). Aceito conscientemente: o mecanismo já foi
  validado por host real e a expressão `hosted()` o mantém barato de
  sustentar. Se um dia o modo hospedado evoluir, será preciso um consumidor
  novo antes (mesmo espírito do ADR 0002).
- **FlowRouter com um consumidor ativo:** a evidência de duplicação que
  aprovou a task 19 no filtro anti-depósito continua registrada nos dois
  GameRouter congelados; o asteroids nasce consumindo o helper.
- **Builds dos jogos estacionados:** quem for buildar 8puzzle/spaceinvaders
  precisa do checkout `../cengine` na tag `0.5.0` — os READMEs daqueles
  repos devem anotar isso quando forem tocados.
- Reverter é barato: migrar um jogo estacionado é seguir o "Migrating from
  0.5.0" do CHANGELOG.

## Emenda 1 — congelamento admite migração de infraestrutura (2026-07-15)

"Estacionado" suspende a **evolução do código** (features, correções, bump da
engine), não o **endereço** do repositório. Quando os repos foram transferidos
de `github.com/mrmarmitt` para a organização `github.com/cengine-dev`, os jogos
congelados receberam um commit `chore:` atualizando as URLs internas (o
`GIT_REPOSITORY` do FetchContent e os links de README). Isso **não fere** esta
ADR: é migração de infraestrutura, não retomada de manutenção — nenhuma linha de
lógica de jogo, nenhum bump de versão da cengine mudou.

Regra: um jogo estacionado pode receber commits que **não alterem seu
comportamento nem sua versão pinada da engine** — migração de host/org, correção
de link quebrado, ajuste de `.gitignore`. Qualquer mudança que toque a lógica do
jogo ou o pin da engine continua vedada e exigiria descongelar explicitamente.

## Relacionado

- Tasks 21 e 19 — o ciclo 0.6.0 que motivou a decisão.
- ADR 0002 — mesmo princípio de fundo: nada se mantém "para já deixar
  pronto"; consumidores reais dirigem o investimento.
