#ifndef SOME_SERVICE_JWT_UTILS_H
#define SOME_SERVICE_JWT_UTILS_H
#include <jwt-cpp/jwt.h>

class Jwt_utils {
public:
    static std::string GenerateToken(const std::string&);

    static bool VerifyToken(const std::string &token);
};


#endif //SOME_SERVICE_JWT_UTILS_H