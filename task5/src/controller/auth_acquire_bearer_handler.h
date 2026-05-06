#pragma once

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPBasicCredentials.h>
#include <Poco/Net/NetException.h>
#include <Poco/Logger.h>
#include <Poco/Timestamp.h>
#include <Poco/JWT/Token.h>
#include <Poco/JWT/Signer.h>

#include "../config/auth_config.h"

#include <sstream>

namespace handlers {

class AuthHandler : public Poco::Net::HTTPRequestHandler {
public:
    void handleRequest(Poco::Net::HTTPServerRequest& request,
                       Poco::Net::HTTPServerResponse& response) override {
        Poco::Timestamp start;

        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentType("application/json");

        try {
            Poco::Net::HTTPBasicCredentials creds(request);
            std::string username = creds.getUsername();
            std::string password = creds.getPassword();

            (void)password;

            if (username.empty()) {
                response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
                std::ostream& ostr = response.send();
                ostr << R"({"error":"username is required"})";
                return;
            }

            if (password.empty()) {
                response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
                std::ostream& ostr = response.send();
                ostr << R"({"error":"password is required"})";
                return;
            }

            if (!(password == "secured" && username == "admin")) {
                response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
                std::ostream& ostr = response.send();
                ostr << R"({"error":"I dont recognize you, your ip adress send to our security team"})";
                return;
            }


            if (g_jwtSecret.empty()) {
                response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
                std::ostream& ostr = response.send();
                ostr << R"({"error":"JWT_SECRET not configured"})";
                return;
            }

            Poco::JWT::Token token;
            token.setType("JWT");
            token.setSubject(username);
            token.setIssuedAt(Poco::Timestamp());
            Poco::Timestamp expiration;
            expiration += Poco::Timespan(24 * 60 * 60, 0);
            token.setExpiration(expiration);
            token.payload().set("username", username);

            Poco::JWT::Signer signer(g_jwtSecret);
            std::string jwt = signer.sign(token, Poco::JWT::Signer::ALGO_HS256);

            std::ostream& ostr = response.send();
            ostr << "{\"token\":\"" << jwt << "\"}";
        } catch (const Poco::Net::NotAuthenticatedException& ex) {
            (void)ex;
            response.setStatus(Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED);
            response.set("WWW-Authenticate", "Basic realm=\"api\"");
            std::ostream& ostr = response.send();
            ostr << R"({"error":"Basic authentication required"})";
            return;
        } catch (const std::exception& e) {
            response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
            std::ostream& ostr = response.send();
            ostr << R"({"error":"internal error"})";
            return;
        }
    }
};

} // namespace handlers
