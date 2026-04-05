#include <greeting.hpp>

#include <fmt/format.h>

#include <userver/tracing/span.hpp>
#include <userver/utils/assert.hpp>
#include <jwt-cpp/jwt.h>
#include <jwt_utils.h>

namespace some_service {

std::string SayHelloTo(std::string_view name, UserType type) {
    if (name.empty()) {
        name = "unknown user";
    }

    auto token = Jwt_utils::GenerateToken("some");

    switch (type) {
        case UserType::kFirstTime:
            return fmt::format("Hello, {}! {}\n", name, token);
        case UserType::kKnown:
            return fmt::format("Hi again, {}! {}\n", name, token);
    }

    UASSERT(false);
    return "unreachable";
}

}  // namespace some_service