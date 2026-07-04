#pragma once

#include "gmock/gmock.h"
#include <cengine/routing/IState.hpp>

class MockState : public cengine::routing::IState {
public:
    MOCK_METHOD(std::string, getCode, (), (const, override));
    MOCK_METHOD(std::string, getName, (), (const, override));
    MOCK_METHOD(std::unique_ptr<cengine::routing::IState>, clone, (), (const, override));

    std::unique_ptr<cengine::routing::IState> cloneMock() const {
        return std::make_unique<MockState>();
    }
};
