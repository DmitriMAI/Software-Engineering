#pragma once

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Logger.h>
#include <Poco/Timestamp.h>
#include <Poco/URI.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>

#include "../database/usersBD.h"

namespace mai::service {

class UsersService {
public:

    void addToDB(const handlers::User& user) {
        handlers::g_users.push_back(user);
    }

    bool existsByLogin(const std::string& login) const {
        return std::find_if(handlers::g_users.begin(), handlers::g_users.end(),
            [&login](const handlers::User& user) {
                auto& logger = Poco::Logger::get("Server");
                logger.information("user was %s %s", user.login, user.getFullName());

                return user.login == login;
            }) != handlers::g_users.end();
    }

    handlers::User* findByLogin(const std::string& login) {
        auto it = std::find_if(handlers::g_users.begin(), handlers::g_users.end(),
            [&login](const handlers::User& user) {
                return user.login == login;
            });
        return it != handlers::g_users.end() ? &(*it) : nullptr;
    }

    bool existsByFullName(const std::string& name, const std::string& last_name) const {
        return std::find_if(handlers::g_users.begin(), handlers::g_users.end(),
            [&name, &last_name](const handlers::User& user) {
                return user.firstName == name && user.lastName == last_name ;
            }) != handlers::g_users.end();
    }

};

} // namespace handlers
