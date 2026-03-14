#include <greeting.hpp>

#include <fmt/format.h>

#include <userver/tracing/span.hpp>
#include <userver/utils/assert.hpp>

namespace some_service {

std::string SayHelloTo(std::string_view name, UserType type) {
    if (name.empty()) {
        name = "unknown user";
    }

    switch (type) {
        case UserType::kFirstTime:
            return fmt::format("Hello, {}!\n", name);
        case UserType::kKnown:
            return fmt::format("Hi again, {}!\n", name);
    }

    UASSERT(false);
    return "unreachable";
}

}  // namespace some_service