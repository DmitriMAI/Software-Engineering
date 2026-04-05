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
#include "../util/autorization_by_jwt.h"
#include "../util/user_parcer.h"
#include "../service/users_service.h"

namespace handlers {

class UsersAddHandler : public Poco::Net::HTTPRequestHandler {
public:
    void handleRequest(Poco::Net::HTTPServerRequest& request,
                       Poco::Net::HTTPServerResponse& response) override {
        
        try {
            mai::utils::ValidatorJwt validator = mai::utils::ValidatorJwt();
            validator.authByJwt(request, response);
        } catch (const mai::exception::AuthException &){
            return; 
        }

        std::istream& body = request.stream();
        std::string bodyStr(std::istreambuf_iterator<char>(body), {});

        auto& logger = Poco::Logger::get("Add_handler_logger");
        logger.information("Add_user_handler_logger: 200 POST /api/v1/users body whit is is %s", bodyStr);

        mai::service::UsersService userService = mai::service::UsersService();

        handlers::User user;
        try {
            user = mai::util::UserParser::parseUser(bodyStr);
        } catch(...) {
            response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
            std::ostream& ostr = response.send();
            ostr << R"({"message":"Cant parse request"})";
            return;
        }

        userService.addToDB(
            user
        );

        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentType("application/json");
        std::ostream& ostr = response.send();
        ostr << R"({"message":"Success on saving user"})";
    }
};

} // namespace handlers
