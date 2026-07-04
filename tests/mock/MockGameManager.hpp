#pragma once

#include "gmock/gmock.h"

#include <cengine/core/IGameManager.hpp>

class MockGameManager : public cengine::core::IGameManager {
public:

    MOCK_METHOD(void, onEnter, (), (override));
    MOCK_METHOD(void, render, (), (override));
    MOCK_METHOD(void, input, (), (override));
    MOCK_METHOD(void, onExit, (), (override));
    MOCK_METHOD(void, cleanup, (), (override));

    MOCK_METHOD(bool, shouldExit, (), (const, override));
};
