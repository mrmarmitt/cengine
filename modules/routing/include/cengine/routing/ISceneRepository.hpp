#pragma once
#include <memory>
#include <string>
#include <functional>

#include <cengine/core/IScene.hpp>

namespace cengine::routing {

/**
 * @brief Provedor de cenas: registro de factories por código de estado,
 *        instanciação *lazy*, cache e descarregamento.
 *
 * Não tem nenhuma noção de estado/navegação — a máquina de estados
 * (atual/próximo, request/commit) é do `IRouter`. Na fiação recomendada o
 * repositório pertence ao router (posse via `unique_ptr`): o jogo registra as
 * factories e transfere a posse, de modo que nenhum componente externo pode
 * descarregar cenas por fora do ciclo de navegação.
 */
class ISceneRepository {
public:
    virtual ~ISceneRepository() = default;

    /// Registra a factory que cria a cena do estado @p name (instanciação lazy).
    virtual void registerFactory(const std::string& name, std::function<std::unique_ptr<core::IScene>()> factory) = 0;

    // Contrato de tempo de vida: a referência retornada aponta para dentro do
    // mapa interno de cenas e é invalidada por unloadScene(name)/unloadAll().
    // NÃO retenha a referência através de um desses unloads. Ver
    // .ai/task/06-scene-lifetime.md.
    virtual core::IScene& getScene(const std::string& name) = 0;

    /// Descarrega a cena @p name (será recriada via factory se pedida de novo).
    /// @warning Descarregar a cena do estado ATIVO fora do commit de navegação
    ///          faz a recriação lazy devolver uma instância que não recebe
    ///          `onEnter()`. Na fiação recomendada (router dono do repositório)
    ///          esse caminho não existe; o reload deliberado da cena atual é
    ///          `IRouter::requestState()` com o mesmo código de estado.
    virtual void unloadScene(const std::string& name) = 0;

    /// Descarrega todas as cenas instanciadas.
    /// @warning Mesma restrição de `unloadScene()` quanto à cena ativa.
    virtual void unloadAll() = 0;
};

} // namespace cengine::routing
