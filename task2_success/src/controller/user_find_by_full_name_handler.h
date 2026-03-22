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

class FindUsersFullNameHandler : public Poco::Net::HTTPRequestHandler {
private: 
    std::string user_name;
    std::string user_last_name;

public:
    explicit FindUsersFullNameHandler(const std::string& userName, const std::string& userLastName) : user_name(userName), user_last_name(userLastName) {}

    void handleRequest(Poco::Net::HTTPServerRequest& request,
                       Poco::Net::HTTPServerResponse& response) override {

        auto& logger = Poco::Logger::get("Find_user_fullname");
        logger.information("find_user_fullname: 200 POST /api/v1/users/fullname name is %s", user_name);

        mai::service::UsersService service = mai::service::UsersService();

        bool is_user_exists = service.existsByFullName(user_name, user_last_name);

        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentType("application/json");
        std::ostream& ostr = response.send();
        if (is_user_exists) {
            ostr << R"({"user":"exist"})";
        } else {
            ostr << R"({"user":"not exist"})";
        }

    }
};


} // namespace handlers
