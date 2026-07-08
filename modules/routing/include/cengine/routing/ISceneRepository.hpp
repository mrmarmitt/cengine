#pragma once
#include <memory>
#include <string>
#include <functional>

#include <cengine/core/IScene.hpp>
#include <cengine/routing/IState.hpp>

namespace cengine::routing {

/**
 * @brief Guarda as cenas (instanciadas *lazy* via factory) e o par de estados
 *        atual/próximo que sustenta a navegação em duas fases do `IRouter`.
 *
 * Cenas são criadas sob demanda a partir de factories registradas por código de
 * estado e podem ser descarregadas para liberar memória. É a peça que o
 * `RouterInMemory` usa por baixo.
 */
class ISceneRepository {
public:
    virtual ~ISceneRepository() = default;

    /// Registra a factory que cria a cena do estado @p name (instanciação lazy).
    virtual void registerFactory(const std::string& name, std::function<std::unique_ptr<core::IScene>()> factory) = 0;

    /// @return o estado atualmente ativo.
    virtual IState& getCurrentStateGame() const = 0;

    /// @return o próximo estado agendado (igual ao atual se nada pendente).
    virtual IState& getNextStateGame() const = 0;

    /// Promove o próximo estado a atual (efetiva a troca).
    virtual void persisteCurrentState() = 0;

    /// Define o próximo estado agendado.
    virtual void persistNextState(std::unique_ptr<IState> state) = 0;
    // Contrato de tempo de vida: a referência retornada aponta para dentro do
    // mapa interno de cenas e é invalidada por unloadScene(name)/unloadAll().
    // NÃO retenha a referência através de um desses unloads. Ver
    // .ai/task/06-scene-lifetime.md.
    virtual core::IScene& getScene(const std::string& name) = 0;

    /// Descarrega a cena @p name (será recriada via factory se pedida de novo).
    /// @warning Não descarregue a cena do estado ATIVO sem uma navegação
    ///          pendente: a recriação lazy devolve uma instância nova sem que
    ///          `onEnter()` rode de novo (a contabilidade de ativação do
    ///          `GameManager` assume que a cena ativa vive até o commit da
    ///          navegação). A política de eviction será centralizada no router
    ///          na task 13.
    virtual void unloadScene(const std::string& name) = 0;

    /// Descarrega todas as cenas instanciadas.
    /// @warning Mesma restrição de `unloadScene()` quanto à cena ativa.
    virtual void unloadAll() = 0;

    /// @return true se o próximo estado difere do atual (troca pendente).
    virtual bool hasPendingStateChange() const = 0;
};

} // namespace cengine::routing
