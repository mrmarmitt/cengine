#pragma once

#include "gmock/gmock.h"
#include <engine/IScene.hpp>

#include "../src/main/engine/service/repository/ISceneRepository.hpp"

class MockSceneRepository : public ISceneRepository {
public:
    MOCK_METHOD(void, registerFactory, (const std::string&, std::function<std::unique_ptr<IScene>()>), (override));
    MOCK_METHOD(IState&, getCurrentStateGame, (), (const, override));
    MOCK_METHOD(IState&, getNextStateGame, (), (const, override));
    MOCK_METHOD(void, persisteCurrentState, (), (override));
    MOCK_METHOD(void, persistNextState, (std::unique_ptr<IState> state), (override));
    MOCK_METHOD(IScene&, getScene, (const std::string& name), (override));
    MOCK_METHOD(void, unloadScene, (const std::string& name), (override));
    MOCK_METHOD(void, unloadAll, (), (override));
    MOCK_METHOD(bool, isNextStateEqualsToCurrentScene, (), (const, override));
};
