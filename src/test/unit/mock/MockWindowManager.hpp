#pragma once

#include "gmock/gmock.h"

#include <engine/IWindowManager.hpp>

class MockWindowManager : public IWindowManager {
public:
    MOCK_METHOD(void, init, (), (override));
    MOCK_METHOD(void, update, (), (override));
    MOCK_METHOD(void, cleanup, (), (override));
};