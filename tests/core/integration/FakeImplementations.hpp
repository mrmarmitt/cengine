#pragma once

#include <vector>
#include <string>

#include <cengine/core/IGameManager.hpp>
#include <cengine/core/IWindowManager.hpp>

/**
 * @class FakeWindowManager
 * @brief Uma implementação "fake" de IWindowManager para testes de integração.
 * * Esta classe armazena um log das chamadas de método para que o teste
 * possa verificar se as interações ocorreram conforme o esperado.
 */
class FakeWindowManager : public cengine::core::IWindowManager {
public:
    std::vector<std::string> callLog;

    void init() override {
        callLog.push_back("init");
    }

    void update() override {
        callLog.push_back("update");
    }

    void cleanup() override {
        callLog.push_back("cleanup");
    }
};

/**
 * @class FakeGameManager
 * @brief Uma implementação "fake" de IGameManager para testes de integração.
 * * Além de registrar as chamadas, permite controlar o valor de retorno de
 * shouldExist() para controlar o loop de execução do EngineManager.
 */
class FakeGameManager : public cengine::core::IGameManager {
public:
    std::vector<std::string> callLog;
    int runCount = 0;
    int maxRuns = 1;
    cengine::core::Seconds lastDt{0};

    void onEnter() override {
        runCount++;
        callLog.push_back("onEnter");
    }

    void render() override {
        callLog.push_back("render");
    }

    void input() override {
        callLog.push_back("input");
    }

    void update(cengine::core::Seconds dt) override {
        callLog.push_back("update");
        lastDt = dt;
    }

    void onExit() override {
        callLog.push_back("onExit");
    }

    bool shouldExit() const override {
        // Retorna true para sair do loop após o número de iterações desejado.
        return runCount >= maxRuns;
    }

    void cleanup() override {
        callLog.push_back("cleanup");
    }
};
