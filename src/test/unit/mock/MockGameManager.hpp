#pragma once

#include "gmock/gmock.h"

#include <engine/IGameManager.hpp>

class MockGameManager : public IGameManager {
public:

    MOCK_METHOD(void, onEnter, (), (override));
    MOCK_METHOD(void, render, (), (override));
    MOCK_METHOD(void, input, (), (override));
    MOCK_METHOD(void, onExit, (), (override));
    MOCK_METHOD(void, cleanup, (), (override));

    MOCK_METHOD(bool, shouldExist, (), (const, override));
};