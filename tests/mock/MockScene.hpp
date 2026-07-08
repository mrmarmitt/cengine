#pragma once

#include "gmock/gmock.h"
#include <cengine/core/IScene.hpp>

class MockScene : public cengine::core::IScene {
public:
    MOCK_METHOD(void, onEnter, (), (override));
    MOCK_METHOD(void, update, (cengine::core::Seconds), (override));
    MOCK_METHOD(void, draw, (), (override));
    MOCK_METHOD(void, input, (), (override));
    MOCK_METHOD(void, onExit, (), (override));
};
