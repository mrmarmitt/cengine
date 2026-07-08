#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>
#include <string>

#include <cengine/core/IScene.hpp>
#include <cengine/routing/GameManager.hpp>
#include <cengine/routing/StateCodes.hpp>

#include <mock/MockRouter.hpp>
#include <mock/MockScene.hpp>
#include <mock/MockState.hpp>

using namespace cengine::core;
using namespace cengine::routing;

class GameManagerTest : public ::testing::Test {
protected:
    std::shared_ptr<MockRouter> m_mockRouter;
    std::unique_ptr<GameManager> m_gameManager;
    MockScene m_mockScene;
    MockState m_mockState;

    void SetUp() override {
        m_mockRouter = std::make_shared<MockRouter>();
        m_gameManager = std::make_unique<GameManager>(m_mockRouter);
    }
};

TEST_F(GameManagerTest, OnEnter_FirstActivation_CallsOnEnterOnScene) {
    EXPECT_CALL(*m_mockRouter, currentState()).WillOnce(testing::ReturnRef(m_mockState));
    EXPECT_CALL(m_mockState, getCode()).WillOnce(testing::Return("main_menu"));
    EXPECT_CALL(*m_mockRouter, currentScene()).WillOnce(testing::ReturnRef(m_mockScene));
    EXPECT_CALL(m_mockScene, onEnter()).Times(1);

    m_gameManager->onEnter();
}

TEST_F(GameManagerTest, OnEnter_SameStateOnNextIterations_DoesNotReenter) {
    EXPECT_CALL(*m_mockRouter, currentState()).WillRepeatedly(testing::ReturnRef(m_mockState));
    EXPECT_CALL(m_mockState, getCode()).WillRepeatedly(testing::Return("main_menu"));
    // A cena só é tocada na primeira ativação; nas iterações seguintes o
    // GameManager nem resolve a cena.
    EXPECT_CALL(*m_mockRouter, currentScene()).WillOnce(testing::ReturnRef(m_mockScene));
    EXPECT_CALL(m_mockScene, onEnter()).Times(1);

    m_gameManager->onEnter();
    m_gameManager->onEnter();
    m_gameManager->onEnter();
}

TEST_F(GameManagerTest, OnEnter_AfterCommittedNavigation_ReentersEvenWithSameStateCode) {
    EXPECT_CALL(*m_mockRouter, currentState()).WillRepeatedly(testing::ReturnRef(m_mockState));
    EXPECT_CALL(m_mockState, getCode()).WillRepeatedly(testing::Return("gameplay"));
    EXPECT_CALL(*m_mockRouter, currentScene()).WillRepeatedly(testing::ReturnRef(m_mockScene));

    // 1ª ativação.
    EXPECT_CALL(m_mockScene, onEnter()).Times(1);
    m_gameManager->onEnter();

    // Navegação efetivada (commit) deve limpar o registro de ativação...
    EXPECT_CALL(*m_mockRouter, hasPendingStateChange()).WillOnce(testing::Return(true));
    EXPECT_CALL(m_mockScene, onExit()).Times(1);
    EXPECT_CALL(*m_mockRouter, commitStateChange()).Times(1);
    m_gameManager->onExit();

    // ...então a próxima iteração reativa a cena, mesmo com o mesmo código
    // (proteção para futuras políticas de keep-alive — A -> B -> A).
    EXPECT_CALL(m_mockScene, onEnter()).Times(1);
    m_gameManager->onEnter();
}

TEST_F(GameManagerTest, Update_ForwardsFixedDtToCurrentScene) {
    constexpr cengine::core::Seconds dt{1.0 / 60.0};

    EXPECT_CALL(*m_mockRouter, currentScene()).WillOnce(testing::ReturnRef(m_mockScene));
    EXPECT_CALL(m_mockScene, update(dt)).Times(1);

    m_gameManager->update(dt);
}

TEST_F(GameManagerTest, Render_CallsDrawOnCurrentScene) {
    EXPECT_CALL(*m_mockRouter, currentScene()).WillOnce(testing::ReturnRef(m_mockScene));
    EXPECT_CALL(m_mockScene, draw()).Times(1);

    m_gameManager->render();
}

TEST_F(GameManagerTest, Input_CallsInputOnCurrentScene) {
    EXPECT_CALL(*m_mockRouter, currentScene()).WillOnce(testing::ReturnRef(m_mockScene));
    EXPECT_CALL(m_mockScene, input()).Times(1);

    m_gameManager->input();
}

TEST_F(GameManagerTest, OnExit_WhenPendingStateChangeExists_CallsOnExitAndCommits) {
    EXPECT_CALL(*m_mockRouter, hasPendingStateChange()).WillOnce(testing::Return(true));
    EXPECT_CALL(*m_mockRouter, currentScene()).WillOnce(testing::ReturnRef(m_mockScene));
    EXPECT_CALL(*m_mockRouter, commitStateChange()).Times(1);
    EXPECT_CALL(m_mockScene, onExit()).Times(1);

    m_gameManager->onExit();
}

TEST_F(GameManagerTest, OnExit_WhenPendingStateChangeDoesNotExist_DoesNothing) {
    EXPECT_CALL(*m_mockRouter, hasPendingStateChange()).WillOnce(testing::Return(false));
    EXPECT_CALL(*m_mockRouter, currentScene()).Times(0);
    EXPECT_CALL(*m_mockRouter, commitStateChange()).Times(0);
    EXPECT_CALL(m_mockScene, onExit()).Times(0);

    m_gameManager->onExit();
}

TEST_F(GameManagerTest, ShouldExit_WhenStateIsExit_ReturnsTrue) {
    EXPECT_CALL(*m_mockRouter, currentState()).WillOnce(testing::ReturnRef(m_mockState));
    EXPECT_CALL(m_mockState, getCode()).WillOnce(testing::Return(std::string(kExitStateCode)));

    ASSERT_TRUE(m_gameManager->shouldExit());
}

TEST_F(GameManagerTest, ShouldExit_WhenStateIsNotExit_ReturnsFalse) {
    EXPECT_CALL(*m_mockRouter, currentState()).WillOnce(testing::ReturnRef(m_mockState));
    EXPECT_CALL(m_mockState, getCode()).WillOnce(testing::Return("main_menu"));

    ASSERT_FALSE(m_gameManager->shouldExit());
}
