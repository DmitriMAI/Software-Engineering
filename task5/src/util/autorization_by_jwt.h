#pragma once

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Logger.h>
#include <Poco/Timestamp.h>
#include <Poco/URI.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>

#include "../errors/AuthError.h"
#include "../config/auth_config.h"

namespace mai::utils {

class ValidatorJwt {
public:
    void authByJwt(Poco::Net::HTTPServerRequest& request,
                       Poco::Net::HTTPServerResponse& response) {

        std::string authHeader = request.get("Authorization", "");
        if (authHeader.empty() || authHeader.find("Bearer ") != 0) {
            response.setStatus(Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED);
            response.setContentType("application/json");
            std::ostream& ostr = response.send();
            ostr << R"({"error":"Bearer token required"})";
            throw mai::exception::AuthException();
        }

        std::string jwt = authHeader.substr(7);

        if (handlers::g_jwtSecret.empty()) {
            response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
            response.setContentType("application/json");
            std::ostream& ostr = response.send();
            ostr << R"({"error":"JWT_SECRET not configured"})";
            throw mai::exception::AuthException();
        }
       
        bool result_for_validation = validateToken(jwt);
        if (!result_for_validation) {
            response.setStatus(Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED);
            response.setContentType("application/json");
            std::ostream& ostr = response.send();
            ostr << R"({"error":"Your ip address send to out security team. Wrong Auth"})";
            throw mai::exception::AuthException();
        }
    }

    bool validateToken (const std::string& tokenStr) {
        try {
        // Создаем верификатор
        Poco::JWT::Signer verifier(handlers::g_jwtSecret);
        
        // Проверяем подпись и получаем payload
        Poco::JWT::Token token = verifier.verify(tokenStr);

        std::string username = token.getSubject();
        Poco::Timestamp iat = token.getIssuedAt();
        
        // Проверяем, не истек ли токен
        if (token.getExpiration() < Poco::Timestamp()) {
            std::cout << "Token expired" << std::endl;
            return false;
        }
        
        return true;
        
    } catch (const Poco::Exception& e) {
        std::cout << "Token validation failed: " << e.displayText() << std::endl;
        return false;
    }
    }
};

} // namespace handlers
