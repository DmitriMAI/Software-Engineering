#include "jwt_utils.h"

#include <jwt-cpp/jwt.h>
#include <chrono>

std::string Jwt_utils::GenerateToken(const std::string& user) {
    auto token = jwt::create()
        .set_type("JWT")
        .set_issuer("auth0")
        .set_subject(user)
        .set_issued_at(std::chrono::system_clock::now())
        .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours(1))
        .sign(jwt::algorithm::hs256{"secret"});

    return token;
}

bool Jwt_utils::VerifyToken(const std::string& token) {
    try {
        auto decoded = jwt::decode(token);

        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{"secret"})
            .with_issuer("auth0");

        verifier.verify(decoded);

        return true;
    } catch (...) {
        return false;
    }
}