# 06 — Corrigir ciclo de vida / referências de cena (risco de dangling)

- **Status:** todo
- **Prioridade:** 🟡 Média
- **Categoria:** Arquitetura / correção
- **Depende de:** 05a (estrutura modular) e 05b (desenho final do Router/Repository)
- **Decisão de referência:** [ADR 0001](../decisions/0001-modular-core-vs-modules.md) —
  esta correção acontece **dentro do módulo `cengine::routing`**, não no core.

## Problema

`getCurrentCachedScreen()` (e correlatos) retornam `IScene&` — uma referência
para dentro do `std::unordered_map` do `SceneRepository`:

```cpp
IScene& SceneRepository::getScene(const std::string& name) {
    ...
    return *(m_scenes[name]);   // referência para o unique_ptr dentro do mapa
}
```

Em `GameManager::onExit()`, a sequência é:

```cpp
IScene& screen = m_routerService->getCurrentCachedScreen();
screen.onExit();
m_routerService->goToNextScreen();   // -> unloadScene(current) -> m_scenes.erase(...)
```

Hoje **funciona** porque a referência é usada *antes* do `erase`. Mas é frágil:
qualquer refatoração que segure essa referência após uma navegação/unload causa
**use-after-free**. `unordered_map::erase` invalida referências ao elemento
removido.

## Objetivo

Eliminar a janela de dangling: nenhuma referência a cena sobrevive a uma operação
que possa descarregá-la.

## Opções de design (decidir na tarefa)

1. **Escopo estreito (mínimo):** documentar o contrato "a referência é válida
   apenas até a próxima navegação" e garantir por código que nenhum caller a
   retém. Barato, mas continua frágil.
2. **Handle/indireção:** o repositório retorna um handle estável (ex.: índice ou
   `weak`/observador) em vez de referência crua ao conteúdo do mapa.
3. **`shared_ptr<IScene>` (recomendado avaliar):** o mapa guarda
   `shared_ptr<IScene>` e os getters retornam `shared_ptr`. Um `unloadScene`
   remove do mapa, mas quem ainda segura o `shared_ptr` mantém a cena viva até
   soltar — elimina o use-after-free de forma robusta.

> Ponderar custo x benefício: (3) é o mais seguro, mas muda assinaturas públicas
> (coordenar com 05). (1) é aceitável para um projeto de estudo se bem documentado.

## Passos

1. Mapear todos os pontos que obtêm referência de cena e o que fazem depois.
2. Escolher a estratégia (recomendação: shared_ptr no repositório de cenas).
3. Implementar e ajustar `getScene`/`getCurrent...`/`GameManager`.
4. Adicionar um teste que exercite: obter cena → navegar/unload → garantir que
   não há acesso inválido (rodar sob ASan idealmente — ver tarefa 08).

## Critérios de aceite

- [ ] Nenhum caminho retém referência a cena através de um `unloadScene`.
- [ ] Teste cobrindo o cenário de navegação + unload.
- [ ] (Se ASan disponível) suíte passa limpa sob AddressSanitizer.

## Riscos

Médio. Mudança de assinatura se optar por `shared_ptr`. Fazer logo após 05 para
não retrabalhar a interface do Router duas vezes.
