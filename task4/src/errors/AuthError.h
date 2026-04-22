#pragma once
#include <stdexcept>

namespace mai::exception
{
    class AuthException : public std::runtime_error {
    public:
        explicit AuthException(const std::string &msg = "Пользователь не смог авторизоваться")
            : std::runtime_error(msg) {}
    };
}