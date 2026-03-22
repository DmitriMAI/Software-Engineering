#pragma once

#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/URI.h>

#include "../controller/auth_acquire_bearer_handler.h"
#include "../controller/not_found_handler.h"
#include "../controller/swagger_handler.h"
#include "../controller/user_find_by_login_handler.h"
#include "../controller/user_find_by_full_name_handler.h"
#include "../controller/goals_get_all_handler.h"
#include "../controller/goals_add_handler.h"
#include "../controller/users_add_handler_auth.h"

namespace handlers {

class RouterFactory : public Poco::Net::HTTPRequestHandlerFactory {
public:
    Poco::Net::HTTPRequestHandler* createRequestHandler(
        const Poco::Net::HTTPServerRequest& request) override {
        const std::string& uri = request.getURI();

        Poco::URI poco_uri(request.getURI());
        std::string path = poco_uri.getPath();
        
        const std::string& method = request.getMethod();

        if (path == "/api/v1/goals" && method == "POST") {
            return new GoalsAddHandler();
        }

        if (path == "/api/v1/users" && method == "POST") {
            return new UsersAddHandler();
        }

        if (path == "/api/v1/goals" && method == "GET") {
            return new GoalsGetAllHandler();
        }

        if (path == "/api/v1/users/login" && method == "GET") {
            Poco::URI::QueryParameters params_query = poco_uri.getQueryParameters();
            std::string name = "defaultName";
            for (const auto& param : params_query) {
                auto& logger = Poco::Logger::get("Server");
                logger.information("param is %s", param.first);
                logger.information("param second is %s", param.second);

                if (param.first == "name") {
                    name =  param.second;
                }
            }
            return new FindUsersByLoginHandler(name);
        }

        if (path == "/api/v1/users/fullname" && method == "GET") {
            Poco::URI::QueryParameters params_query = poco_uri.getQueryParameters();
            std::string name = "defaultName";
            std::string last_name = "defaultLastName";
            for (const auto& param : params_query) {
                auto& logger = Poco::Logger::get("Server");
                logger.information("param is %s", param.first);
                logger.information("param second is %s", param.second);

                if (param.first == "name") {
                    name =  param.second;
                }
                if (param.first == "lastName") {
                    last_name =  param.second;
                }
            }
            return new FindUsersFullNameHandler(name, last_name);
        }

    
        if (uri == "/api/v1/auth" && method == "POST") {
            return new AuthHandler();
        }
        if (uri == "/swagger.yaml" && method == "GET") {
            return new SwaggerHandler();
        }
        if (uri == "/swaggerUI" && method == "GET") {
            return new SwaggerUIHandler();
        }
        return new NotFoundHandler();
    }
};

} // namespace handlers
