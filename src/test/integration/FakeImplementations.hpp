#pragma once

#include <vector>
#include <string>

#include <engine/IGameManager.hpp>
#include <engine/IWindowManager.hpp>

/**
 * @class FakeWindowManager
 * @brief Uma implementação "fake" de IWindowManager para testes de integração.
 * * Esta classe armazena um log das chamadas de método para que o teste
 * possa verificar se as interações ocorreram conforme o esperado.
 */
class FakeWindowManager : public IWindowManager {
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
class FakeGameManager : public IGameManager {
public:
    std::vector<std::string> callLog;
    int runCount = 0;
    int maxRuns = 1;

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

    void onExit() override {
        callLog.push_back("onExit");
    }

    bool shouldExist() const override {
        // Retorna true para sair do loop após o número de iterações desejado.
        return runCount >= maxRuns;
    }

    void cleanup() override {
        callLog.push_back("cleanup");
    }
};
