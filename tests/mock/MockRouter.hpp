#pragma once

#include "gmock/gmock.h"

#include <cengine/core/IScene.hpp>
#include <cengine/routing/IRouter.hpp>

class MockRouter : public cengine::routing::IRouter {
public:
    MOCK_METHOD(void, requestState, (std::unique_ptr<cengine::routing::IState>), (override));
    MOCK_METHOD(bool, hasPendingStateChange, (), (const, override));
    MOCK_METHOD(void, commitStateChange, (), (override));
    MOCK_METHOD(const cengine::routing::IState&, currentState, (), (const, override));
    MOCK_METHOD(cengine::core::IScene&, currentScene, (), (override));
};
