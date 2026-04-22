#pragma once

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Logger.h>
#include <Poco/Timestamp.h>
#include <Poco/URI.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>

#include <Poco/Data/Session.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>
#include <sstream>


#include "../database/usersBD.h"
#include "../database/Database.h"

using namespace Poco::Data::Keywords;

namespace mai::service {

class UsersService {
public:

    void addToDB(const handlers::User& user) {
        handlers::g_users.push_back(user);
    }

    bool addToRealDB(handlers::User& user) {
        auto& logger = Poco::Logger::get("users_service");
        logger.information("before all save user2");
        Poco::Data::Session ses = mai::postGre::Database::instance().createSession();
        logger.information("get sesion user");
        Poco::Data::Statement insert(ses);
        logger.information("get sesion user");
        try {
            int id = 0;
            logger.information("before save user");
            ses << "INSERT INTO users (login, first_name, last_name) "
                "VALUES ($1,$2,$3) RETURNING id",
                use(user.login), use(user.firstName), use(user.lastName), into(id), now;

            logger.information("saved user");
            return true;
        }
        catch (Poco::Exception& e) {
            auto& logger = Poco::Logger::get("users_service");
            logger.information("was error on saving : " + e.displayText());
            return false;
        }
    }

    bool existsByLogin(std::string& login) const {
        Poco::Data::Session ses = mai::postGre::Database::instance().createSession();
        Poco::Data::Statement select(ses);
        
        try {
            int id = 0;
            select << "SELECT id, login, first_name, last_name "
                "FROM users WHERE login = $1",
                use(login);

            select.execute();
            Poco::Data::RecordSet rs(select);
            if (!rs.moveFirst()) return false;
            return true;
        }
        catch (Poco::Exception& e) {
            auto& logger = Poco::Logger::get("users_service");
            logger.information("was error on find exist user : " + e.displayText());
            return false;
        }
    }

    handlers::User* findByLogin(std::string& login) {
        
        Poco::Data::Session ses = mai::postGre::Database::instance().createSession();
        Poco::Data::Statement select(ses);
        try {
            int id = 0;
            select << "SELECT id, login, first_name, last_name "
                "FROM users WHERE login = $1",
                use(login);

            select.execute();
            Poco::Data::RecordSet rs(select);
            if (!rs.moveFirst()) return nullptr;
            
            handlers::User* user = new handlers::User();
            user -> login = rs["login"].convert<std::string>();
            user -> firstName = rs["first_name"].convert<std::string>();
            user -> lastName = rs["last_name"].convert<std::string>();
            return user;
        }
        catch (Poco::Exception& e) {
            auto& logger = Poco::Logger::get("users_service");
            logger.information("was error on find by login : " + e.displayText());
            return nullptr;
        }
    }

    bool existsByFullName(const std::string& name, const std::string& last_name) const {
        return std::find_if(handlers::g_users.begin(), handlers::g_users.end(),
            [&name, &last_name](const handlers::User& user) {
                return user.firstName == name && user.lastName == last_name ;
            }) != handlers::g_users.end();
    }

    int getUserIdByLogin(std::string& login) {
    Poco::Data::Session ses = mai::postGre::Database::instance().createSession();
    Poco::Data::Statement select(ses);
    try {
        int userId = -1;
        select << "SELECT id FROM users WHERE login = $1",
            use(login), into(userId);
        
        select.execute();
        
        if (userId != -1) {
            return userId;
        }
        return -1; // пользователь не найден
    }
    catch (Poco::Exception& e) {
        auto& logger = Poco::Logger::get("users_service");
        logger.information("was error on get user id by login: " + e.displayText());
        return -1;
    }
}

};

} // namespace handlers
