# 16 — Fim do quadro na janela: `IWindowManager::present()`

- **Status:** in-progress (implementação em 2026-07-10 —
  `feature/16-window-present-hook`; falta release 0.5.0 + migração do
  8Puzzle + validação no degrau 2)
- **Prioridade:** 🟡 Média (sobe com a fase 2)
- **Categoria:** Arquitetura / core
- **Depende de:** 15 ✅ (modo hospedado). Consumidor real: 8Puzzle task 02
  (modo biblioteca do The-Forge).
- **Breaking:** sim, pequeno — método novo em interface pura (`IWindowManager`);
  âncora do bump **0.5.0**.

## Problema

O quadro do `EngineManager` aciona a janela só no INÍCIO da iteração
(`window.update()` antes das fases). Plataformas de GPU exigem trabalho
DEPOIS do `render()`: fechar o command buffer, submeter à fila e apresentar
(`queuePresent`). Hoje não existe esse gancho:

- **Modo hospedado (fase 1)**: não dói — o host (IApp do The-Forge) faz o
  submit/present fora da cengine, em volta do `frame(dt)`.
- **Modo biblioteca (fase 2)**: dói — a cengine é dona do loop e ninguém
  fecha o quadro. Contorno possível (submeter o quadro anterior no `update()`
  seguinte) adiciona um quadro de latência e espalha estado — descartado na
  discussão de 2026-07-09.

Sinal de desenho: até o FTXUI ficaria mais correto com o gancho — hoje o
`Screen.Print()` acontece dentro do `draw()` da cena (via `present()` da
`FtxuiScene`), quando conceitualmente a apresentação é da plataforma.

## Objetivo

O quadro da janela ganha simetria início/fim:

```
window.update()   -> eventos de SO, preparo do quadro (acquire, beginCmd...)
fases do jogo     -> onEnter -> input -> update(fixedDt) 0..N -> render -> onExit
window.present()  -> fecha e apresenta o quadro (endCmd, submit, queuePresent)
```

## Esboço de desenho (validar na execução)

```cpp
class IWindowManager {
public:
    virtual void init() = 0;
    virtual void update() = 0;
    /// Fecha/apresenta o quadro desenhado pelas fases do jogo. Chamado ao
    /// FIM de cada iteração, depois de render()/onExit(). Plataformas sem
    /// conceito de present (terminal) implementam vazio.
    virtual void present() = 0;
    virtual void cleanup() = 0;
};
```

No `EngineManager`:

- `run()` (modo próprio): `update()` antes de `frame()`, `present()` depois —
  a janela continua 100% fora de `frame()` (contrato da task 15 preservado:
  modo hospedado não muda NADA).
- Ordem no quadro: `present()` roda depois de `onExit()` (a troca de cena já
  commitada não desenha — o quadro apresentado é o da cena que rendeu).
  Confirmar na execução com os testes de call-log.

## Questões fechadas na execução (2026-07-10)

1. `present()` RODA quando `shouldExit()` retornou true no mesmo quadro —
   o último quadro desenhado é apresentado antes do `cleanup()`. No run():
   `update()` → `frame()` → `present()`, sem condicional no retorno.
   Coberto por teste (call-log e InSequence). Revalidar com o caso real do
   The-Forge no degrau 2.
2. Migração dos consumidores pelo MENOR diff: terminal e FTXUI implementam
   `present()` vazio numa primeira leva. Mover o `Screen.Print()` do FTXUI
   para o window manager fica como melhoria opcional futura (o sinal de
   desenho segue registrado no Problema).
3. `present()` DEPOIS do `onExit()` (review do PR #18 questionou lifetime:
   o commit da rota destrói a cena que rendeu antes do submit). Mantido:
   cenas são lógica pura e nunca possuem recurso de GPU — tudo que o quadro
   gravado referencia pertence à plataforma (ponte de desenho/window
   manager); e o modo hospedado (fase 1, validado no 8PuzzleForge) já
   apresenta depois de `frame()`/`onExit()` sem problema. O contrato ficou
   explícito no @note do `present()`.

## Passos

1. Adicionar `present()` à interface + chamada no `run()` (fora de
   `frame()`); atualizar testes de call-log (nova expectativa de ordem).
2. Migrar mocks/fakes da suíte e as plataformas dos consumidores (8Puzzle:
   terminal e FTXUI — no-op no mínimo).
3. Doxygen + README (diagrama do quadro com início/fim).
4. Release 0.5.0 (breaking pequeno) e migração do 8Puzzle.
5. Validação real: degrau 2 da task 02 do 8Puzzle (TheForgeWindowManager
   usando update()/present() para o boilerplate de GPU).

## Critérios de aceite

- [ ] `IWindowManager::present()` chamado ao fim de cada iteração do modo
      próprio; `frame(dt)` (modo hospedado) segue sem tocar janela.
- [ ] Suíte atualizada e verde (ordem update → fases → present coberta por
      call-log).
- [ ] Consumidores migrados (terminal/FTXUI) sem regressão.
- [ ] Consumido de verdade pelo degrau 2 da fase 2 do 8Puzzle.

## Riscos

Baixo em código; o cuidado é NÃO desenhar além do caso real (mesma lição da
task 15): nada de `beginFrame()/endFrame()` genéricos, swapchain abstrato ou
interpolação — só o gancho que a fase 2 pede.

## Relacionado

- Task 15 — modo hospedado (`frame(dt)`); esta task completa o modo próprio.
- 8Puzzle task 02 — fase 2 The-Forge (consumidor que valida o desenho).
