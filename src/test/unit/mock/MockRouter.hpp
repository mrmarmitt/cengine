#pragma once

#include "gmock/gmock.h"
#include "../src/main/engine/service/IRouter.hpp"

class MockRouter : public IRouter {
public:
    MOCK_METHOD(void, setNextState, (std::unique_ptr<IState>), (const, override));
    MOCK_METHOD(IState&, getCurrentStateGame, (), (const, override));
    MOCK_METHOD(std::string, getCurrentStateGameName, (), (const, override));
    MOCK_METHOD(std::string, getCurrentStateGameCode, (), (const, override));
    MOCK_METHOD(IScene&, getCurrentCachedScreen, (), (const, override));
    MOCK_METHOD(IState&, getNextStateGame, (), (const, override));
    MOCK_METHOD(std::string, getNextStateGameName, (), (const, override));
    MOCK_METHOD(std::string, getNextStateGameCode, (), (const, override));
    MOCK_METHOD(IScene&, getNextCachedScreen, (), (const, override));
    MOCK_METHOD(bool, hasNextScreen, (), (const, override));
    MOCK_METHOD(void, goToNextScreen, (), (const, override));
};