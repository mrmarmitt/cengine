#pragma once

#include <string>
#include <utility>

#include <cengine/routing/IState.hpp>

// IState mínimo para testes: carrega apenas o código (nome = código).
class FakeState final : public cengine::routing::IState {
    std::string m_code;

public:
    explicit FakeState(std::string code) : m_code(std::move(code)) {}

    [[nodiscard]] std::string getCode() const override { return m_code; }
    [[nodiscard]] std::string getName() const override { return m_code; }
};
