#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <engine/IState.hpp>
#include <engine/IScene.hpp>

#include <test/unit/mock/MockState.hpp>
#include <test/unit/mock/MockScene.hpp>

#include "../src/main/engine/service/repository/SceneRepository.hpp"

class SceneRepositoryTest : public ::testing::Test {
protected:
    std::unique_ptr<MockState> initialState;
    std::unique_ptr<SceneRepository> sceneRepository;

    void SetUp() override {
        initialState = std::make_unique<MockState>();
        
        EXPECT_CALL(*initialState, clone())
            .WillOnce(::testing::Return(std::make_unique<MockState>()))
            .WillOnce(::testing::Return(std::make_unique<MockState>()));
        
        // Define o comportamento dos métodos getCode e getName para evitar erros de mock
        EXPECT_CALL(*initialState, getCode()).WillRepeatedly(::testing::Return("InitialStateCode"));
        EXPECT_CALL(*initialState, getName()).WillRepeatedly(::testing::Return("InitialStateName"));

        sceneRepository = std::make_unique<SceneRepository>(std::move(initialState));
    }
};

// Testa o registro e a obtenção de uma scene.
TEST_F(SceneRepositoryTest, RegisterAndGetScene) {
    // 1. Registra uma factory.
    sceneRepository->registerFactory("MainMenu", [](){
        return std::make_unique<MockScene>();
    });

    // 2. Obtém a scene, que deve ser instanciada via factory.
    // O EXPECT_NO_THROW verifica se a chamada não lança uma exceção.
    EXPECT_NO_THROW({
        IScene& scene = sceneRepository->getScene("MainMenu");
        // Verifica se a scene retornada não é nula.
        ASSERT_NE(&scene, nullptr);
    });
}

// Teste: getScene deve instanciar uma nova cena via factory se não existir.
TEST_F(SceneRepositoryTest, GetSceneInstantiatesViaFactory) {
    bool factoryCalled = false;
    sceneRepository->registerFactory("NewScene", [&](){
        factoryCalled = true;
        return std::make_unique<MockScene>();
    });

    IScene& scene = sceneRepository->getScene("NewScene");
    ASSERT_TRUE(factoryCalled);
    ASSERT_NE(&scene, nullptr);
}

// Teste: getScene deve retornar uma cena existente se já estiver instanciada.
TEST_F(SceneRepositoryTest, GetSceneReturnsExistingScene) {
    int factoryCallCount = 0;
    sceneRepository->registerFactory("ExistingScene", [&](){
        factoryCallCount++;
        return std::make_unique<MockScene>();
    });

    IScene& scene1 = sceneRepository->getScene("ExistingScene");
    ASSERT_EQ(factoryCallCount, 1);

    IScene& scene2 = sceneRepository->getScene("ExistingScene");
    ASSERT_EQ(factoryCallCount, 1);
    ASSERT_EQ(&scene1, &scene2);
}

// Teste: getScene deve lançar std::runtime_error se a cena não for encontrada.
TEST_F(SceneRepositoryTest, GetSceneThrowsIfNotFound) {
    EXPECT_THROW({
        sceneRepository->getScene("NonExistentScene");
    }, std::runtime_error);
}

// Teste: unloadScene deve remover uma cena específica, o que é verificado
// por uma segunda chamada à factory quando getScene é invocado novamente.
TEST_F(SceneRepositoryTest, UnloadSceneRemovesSpecificScene) {
    int factoryACallCount = 0;
    sceneRepository->registerFactory("SceneA", [&](){
        factoryACallCount++;
        return std::make_unique<MockScene>();
    });
    sceneRepository->registerFactory("SceneB", [](){ return std::make_unique<MockScene>(); });

    // 1. Instancia SceneA e SceneB
    sceneRepository->getScene("SceneA");
    sceneRepository->getScene("SceneB");

    // Verifica se a factory de SceneA foi chamada uma vez
    ASSERT_EQ(factoryACallCount, 1);
    
    // 2. Unload SceneA
    sceneRepository->unloadScene("SceneA");

    // 3. Tenta obter SceneA novamente. Isso deve disparar a factory novamente.
    sceneRepository->getScene("SceneA");
    
    // Agora a factory de SceneA deve ter sido chamada uma segunda vez.
    ASSERT_EQ(factoryACallCount, 2);

    // 4. Verifica se a SceneB ainda existe
    EXPECT_NO_THROW(sceneRepository->getScene("SceneB"));
}

// Teste: unloadAll deve remover todas as cenas instanciadas.
TEST_F(SceneRepositoryTest, UnloadAllRemovesAllScenes) {
    int factory1CallCount = 0;
    int factory2CallCount = 0;

    sceneRepository->registerFactory("Scene1", [&](){
        factory1CallCount++;
        return std::make_unique<MockScene>();
    });
    sceneRepository->registerFactory("Scene2", [&](){
        factory2CallCount++;
        return std::make_unique<MockScene>();
    });

    sceneRepository->getScene("Scene1");
    sceneRepository->getScene("Scene2");

    ASSERT_EQ(factory1CallCount, 1);
    ASSERT_EQ(factory2CallCount, 1);

    sceneRepository->unloadAll();

    // Depois de unloadAll, chamar getScene deve recriar as cenas, chamando as factories novamente.
    sceneRepository->getScene("Scene1");
    sceneRepository->getScene("Scene2");

    ASSERT_EQ(factory1CallCount, 2);
    ASSERT_EQ(factory2CallCount, 2);
}

// Teste: persisteCurrentState deve clonar m_nextState para m_currentState.
TEST_F(SceneRepositoryTest, PersistCurrentStateClonesNextState) {
    auto newStateForNext = std::make_unique<MockState>();
    
    // 1. Move o novo estado para o repositório.
    sceneRepository->persistNextState(std::move(newStateForNext));

    // 2. Obtém uma referência ao objeto que agora está em m_nextState
    IState& nextStateRef = sceneRepository->getNextStateGame();
    
    // 3. Define a única expectativa de chamada que realmente acontecerá
    EXPECT_CALL(static_cast<MockState&>(nextStateRef), clone())
        .WillOnce(::testing::Return(std::make_unique<MockState>()));

    // 4. Chama o método que irá disparar o clone()
    sceneRepository->persisteCurrentState();
}

// Teste: persistNextState deve mover a propriedade de unique_ptr.
TEST_F(SceneRepositoryTest, PersistNextStateMovesNewState) {
    auto originalNextState = &sceneRepository->getNextStateGame();
    
    auto brandNewState = std::make_unique<MockState>();
    auto brandNewStatePtr = brandNewState.get();

    // Persiste o novo estado movendo-o para a propriedade interna
    sceneRepository->persistNextState(std::move(brandNewState));

    // Verifica se a propriedade interna agora aponta para o novo objeto
    ASSERT_EQ(&sceneRepository->getNextStateGame(), brandNewStatePtr);
    // E se a propriedade anterior não é mais a mesma
    ASSERT_NE(&sceneRepository->getNextStateGame(), originalNextState);
}


// Teste: isNextStateEqualsToCurrentScene deve retornar true se os códigos de estado forem diferentes.
TEST_F(SceneRepositoryTest, IsNextStateEqualsToCurrentSceneReturnsTrueIfCodesDifferent) {
    auto nextStateMock = std::make_unique<MockState>();
    
    // Define o comportamento de getCode() para os estados
    EXPECT_CALL(static_cast<MockState&>(sceneRepository->getCurrentStateGame()), getCode())
        .WillOnce(::testing::Return("SomeCode"));
    EXPECT_CALL(*nextStateMock, getCode()).WillOnce(::testing::Return("DifferentCode"));
    
    // Persiste o novo estado
    sceneRepository->persistNextState(std::move(nextStateMock));

    // A função deve retornar true porque os códigos são diferentes
    EXPECT_TRUE(sceneRepository->isNextStateEqualsToCurrentScene());
}

// Teste: isNextStateEqualsToCurrentScene deve retornar false se os códigos de estado forem iguais.
TEST_F(SceneRepositoryTest, IsNextStateEqualsToCurrentSceneReturnsFalseIfCodesSame) {
    auto nextStateMock = std::make_unique<MockState>();
    
    // Define o comportamento de getCode() para os estados
    EXPECT_CALL(static_cast<MockState&>(sceneRepository->getCurrentStateGame()), getCode())
        .WillOnce(::testing::Return("SameCode"));
    EXPECT_CALL(*nextStateMock, getCode()).WillOnce(::testing::Return("SameCode"));
    
    // Persiste o novo estado
    sceneRepository->persistNextState(std::move(nextStateMock));

    // A função deve retornar false porque os códigos são os mesmos
    EXPECT_FALSE(sceneRepository->isNextStateEqualsToCurrentScene());
}
