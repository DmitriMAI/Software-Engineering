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
#include "../cache/cacheForUser.h"

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

    handlers::User* getByLoginCached(std::string login) {
        auto& logger = Poco::Logger::get("users_servic_2");

        if (auto cached = mai::UserRedisCache::instance().tryGetJson(login)) {
            logger.information("User was found in cache!");

            try {
                Poco::JSON::Parser parser;
                Poco::Dynamic::Var parsed = parser.parse(*cached);
                auto obj = parsed.extract<Poco::JSON::Object::Ptr>();
                handlers::User user = userFromJson(obj);

                handlers::User* userRes = new handlers::User();
                userRes -> login = user.login;
                userRes -> firstName = user.firstName;
                userRes -> lastName = user.lastName;
                return userRes;
            } catch (const Poco::Exception&) {
                // повреждённый кэш — читаем из MongoDB
            }
        }

        handlers::User* fromMongo = findByLogin(login);
        if (fromMongo != nullptr) {
            std::ostringstream oss;
            toJson(fromMongo)->stringify(oss);
            mai::UserRedisCache::instance().putJsonLogin(login, oss.str());
        }
        return fromMongo;
    }

    static Poco::JSON::Object::Ptr toJson(const handlers::User* user) {
        Poco::JSON::Object::Ptr out = new Poco::JSON::Object();
        out->set("login", user->login);
        out->set("first_name", user->firstName);
        out->set("last_name", user->lastName);
        return out;
    }

    static handlers::User userFromJson(const Poco::JSON::Object::Ptr& obj) {
        std::string login  = obj->getValue<std::string>("login");
        std::string name  = obj->getValue<std::string>("name");
        std::string familia  = obj->getValue<std::string>("familyName");
        
        return handlers::User(login, name, familia);
    }

    bool existsByFullNameCached(const std::string& name, const std::string& last_name) const {
        auto& logger = Poco::Logger::get("users_servic_2");

        std::string fullName = name + "_" + last_name;

        if (auto cached = mai::UserRedisCache::instance().tryGetJsonFullName(fullName)) {
            logger.information("User was found in cache!");
            return (*cached == "1" || *cached == "true");
        }

        bool fromMongo = existsByFullName(name, last_name);
        mai::UserRedisCache::instance().putJsonFullName(fullName, fromMongo ? "1" : "0");
        return fromMongo;
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
