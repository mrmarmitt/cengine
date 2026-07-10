#pragma once

#include "gmock/gmock.h"

#include <cengine/core/IWindowManager.hpp>

class MockWindowManager : public cengine::core::IWindowManager {
public:
    MOCK_METHOD(void, init, (), (override));
    MOCK_METHOD(void, update, (), (override));
    MOCK_METHOD(void, present, (), (override));
    MOCK_METHOD(void, cleanup, (), (override));
};
