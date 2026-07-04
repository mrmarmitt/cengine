#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>
#include <string>

#include <cengine/core/IScene.hpp>
#include <cengine/routing/IState.hpp>
#include <cengine/routing/RouterInMemory.hpp>

#include <mock/MockScene.hpp>
#include <mock/MockSceneRepository.hpp>
#include <mock/MockState.hpp>

using namespace cengine::core;
using namespace cengine::routing;

class RouterInMemoryTest : public ::testing::Test {
protected:
    std::shared_ptr<MockSceneRepository> mockSceneRepository;
    std::unique_ptr<RouterInMemory> routerService;

    MockState mockCurrentState;
    MockScene mockScene;

    void SetUp() override {
        mockSceneRepository = std::make_shared<MockSceneRepository>();
        routerService = std::make_unique<RouterInMemory>(mockSceneRepository);
    }
};

TEST_F(RouterInMemoryTest, RequestStateDelegatesToRepository) {
    auto newState = std::make_unique<MockState>();

    EXPECT_CALL(*mockSceneRepository, persistNextState(testing::A<std::unique_ptr<IState>>())).Times(1);

    routerService->requestState(std::move(newState));
}

TEST_F(RouterInMemoryTest, CurrentStateDelegatesCorrectly) {
    EXPECT_CALL(*mockSceneRepository, getCurrentStateGame()).WillOnce(testing::ReturnRef(mockCurrentState));

    const IState& returnedState = routerService->currentState();

    ASSERT_EQ(&returnedState, &mockCurrentState);
}

TEST_F(RouterInMemoryTest, CurrentSceneDelegatesCorrectly) {
    const std::string expectedCode = "MENU";

    EXPECT_CALL(*mockSceneRepository, getCurrentStateGame()).WillOnce(testing::ReturnRef(mockCurrentState));
    EXPECT_CALL(mockCurrentState, getCode()).WillOnce(testing::Return(expectedCode));
    EXPECT_CALL(*mockSceneRepository, getScene(expectedCode)).WillOnce(testing::ReturnRef(mockScene));

    IScene& returnedScene = routerService->currentScene();

    ASSERT_EQ(&returnedScene, &mockScene);
}

TEST_F(RouterInMemoryTest, CommitStateChangeCallsMethodsInCorrectOrder) {
    const std::string expectedCode = "OLD_SCENE";
    testing::InSequence sequence;

    EXPECT_CALL(*mockSceneRepository, getCurrentStateGame()).WillOnce(testing::ReturnRef(mockCurrentState));
    EXPECT_CALL(mockCurrentState, getCode()).WillOnce(testing::Return(expectedCode));
    EXPECT_CALL(*mockSceneRepository, unloadScene(expectedCode)).Times(1);
    EXPECT_CALL(*mockSceneRepository, persisteCurrentState()).Times(1);

    routerService->commitStateChange();
}

TEST_F(RouterInMemoryTest, HasPendingStateChangeDelegatesCorrectly) {
    EXPECT_CALL(*mockSceneRepository, hasPendingStateChange()).WillOnce(testing::Return(true));
    ASSERT_TRUE(routerService->hasPendingStateChange());

    EXPECT_CALL(*mockSceneRepository, hasPendingStateChange()).WillOnce(testing::Return(false));
    ASSERT_FALSE(routerService->hasPendingStateChange());
}
