#pragma once

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Logger.h>
#include <Poco/Timestamp.h>
#include <Poco/URI.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>

#include "../service/users_service.h"

namespace handlers {

class FindUsersByLoginHandler : public Poco::Net::HTTPRequestHandler {
private: 
    std::string user_login;

public:
    explicit FindUsersByLoginHandler(const std::string& userName) : user_login(userName) {}

    void handleRequest(Poco::Net::HTTPServerRequest& request,
                       Poco::Net::HTTPServerResponse& response) override {

        auto& logger = Poco::Logger::get("Find_user_by_login_heandler");
        logger.information("find_user_by_login_heandler: 200 POST /api/v1/users/login name is %s", user_login);

        mai::service::UsersService service = mai::service::UsersService();

        bool is_user_exists = service.existsByLogin(user_login);

        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentType("application/json");
        std::ostream& ostr = response.send();
        
        if (is_user_exists) {
            User* user = service.findByLogin(user_login);

            Poco::JSON::Object::Ptr result = new Poco::JSON::Object();
            result->set("status", "success");
            
            Poco::JSON::Object::Ptr userObj = new Poco::JSON::Object();
            userObj->set("login", user->login);
            userObj->set("firstName", user->firstName);
            userObj->set("lastName", user->lastName);
            userObj->set("fullName", user->getFullName());
            
            result->set("user", userObj);
            
            result->stringify(ostr);
        } else {
            ostr << R"({"user":"not exist"})";
        }

    }
};


} // namespace handlers
