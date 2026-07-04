#pragma once

#include <memory>
#include <string>

namespace cengine::routing {

class IState {
public:
    virtual ~IState() = default;

    [[nodiscard]] virtual std::string getCode() const = 0;
    [[nodiscard]] virtual std::string getName() const = 0;
    [[nodiscard]] virtual std::unique_ptr<IState> clone() const = 0;
};

} // namespace cengine::routing
