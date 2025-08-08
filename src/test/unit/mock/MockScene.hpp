#pragma once

#include "gmock/gmock.h"
#include <engine/IScene.hpp>

class MockScene : public IScene {
public:
    MOCK_METHOD(void, onEnter, (), (override));
    MOCK_METHOD(bool, isOnEnterExecuted, (), (const, override));
    MOCK_METHOD(void, onEnterExecuted, (), (override));
    MOCK_METHOD(void, draw, (), (override));
    MOCK_METHOD(void, input, (), (override));
    MOCK_METHOD(void, onExit, (), (override));
};
