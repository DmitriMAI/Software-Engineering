#pragma once

#include <string>
#include <string_view>

namespace some_service {

enum class UserType { kFirstTime, kKnown };

std::string SayHelloTo(std::string_view name, UserType type);

}  // namespace some_service