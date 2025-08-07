#pragma once

#include "gmock/gmock.h"
#include <engine/IState.hpp>

class MockState : public IState {
public:
    MOCK_METHOD(std::string, getCode, (), (const, override));
    MOCK_METHOD(std::string, getName, (), (const, override));
    MOCK_METHOD(std::unique_ptr<IState>, clone, (), (const, override));

    std::unique_ptr<IState> cloneMock() const {
        return std::make_unique<MockState>();
    }
};