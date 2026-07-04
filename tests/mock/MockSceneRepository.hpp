#pragma once

#include "gmock/gmock.h"

#include <cengine/core/IScene.hpp>
#include <cengine/routing/ISceneRepository.hpp>

class MockSceneRepository : public cengine::routing::ISceneRepository {
public:
    MOCK_METHOD(void, registerFactory, (const std::string&, std::function<std::unique_ptr<cengine::core::IScene>()>), (override));
    MOCK_METHOD(cengine::routing::IState&, getCurrentStateGame, (), (const, override));
    MOCK_METHOD(cengine::routing::IState&, getNextStateGame, (), (const, override));
    MOCK_METHOD(void, persisteCurrentState, (), (override));
    MOCK_METHOD(void, persistNextState, (std::unique_ptr<cengine::routing::IState> state), (override));
    MOCK_METHOD(cengine::core::IScene&, getScene, (const std::string& name), (override));
    MOCK_METHOD(void, unloadScene, (const std::string& name), (override));
    MOCK_METHOD(void, unloadAll, (), (override));
    MOCK_METHOD(bool, hasPendingStateChange, (), (const, override));
};
