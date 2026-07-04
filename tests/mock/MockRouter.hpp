#pragma once

#include "gmock/gmock.h"

#include <cengine/core/IScene.hpp>
#include <cengine/routing/IRouter.hpp>

class MockRouter : public cengine::routing::IRouter {
public:
    MOCK_METHOD(void, setNextState, (std::unique_ptr<cengine::routing::IState>), (const, override));
    MOCK_METHOD(cengine::routing::IState&, getCurrentStateGame, (), (const, override));
    MOCK_METHOD(std::string, getCurrentStateGameName, (), (const, override));
    MOCK_METHOD(std::string, getCurrentStateGameCode, (), (const, override));
    MOCK_METHOD(cengine::core::IScene&, getCurrentCachedScreen, (), (const, override));
    MOCK_METHOD(cengine::routing::IState&, getNextStateGame, (), (const, override));
    MOCK_METHOD(std::string, getNextStateGameName, (), (const, override));
    MOCK_METHOD(std::string, getNextStateGameCode, (), (const, override));
    MOCK_METHOD(cengine::core::IScene&, getNextCachedScreen, (), (const, override));
    MOCK_METHOD(bool, hasNextScreen, (), (const, override));
    MOCK_METHOD(void, goToNextScreen, (), (const, override));
};
