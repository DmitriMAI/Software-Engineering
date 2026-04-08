#pragma once

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Logger.h>
#include <Poco/Timestamp.h>
#include <Poco/URI.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>

#include "../database/goalBD.h"

namespace mai::service {

class GoalsService {
public:

    void addToDB(const handlers::Goal& goal) {
        handlers::g_goals.push_back(goal);
    }

    bool addToRealDB(int owner_id, handlers::Goal& goal) {
        auto& logger = Poco::Logger::get("users_service");
        logger.information("before all save goal");
        Poco::Data::Session ses = Database::instance().createSession();
        logger.information("get sesion goal");
        Poco::Data::Statement insert(ses);
        logger.information("get sesion goal");
        try {
            int id = 0;
            logger.information("before save goal");
            ses << "INSERT INTO goals (owner_id, title)"
                "VALUES ($1,$2) RETURNING id",
                use(owner_id), use(goal.goal_name), into(id), now;

            logger.information("saved goal");
            return true;
        }
        catch (Poco::Exception& e) {
            auto& logger = Poco::Logger::get("users_service");
            logger.information("was error on saving : " + e.displayText());
            return false;
        }
    }

    bool fillGoalsArray(std::vector<handlers::Goal>& goals) {
    auto& logger = Poco::Logger::get("users_service");
    
    try {
        Poco::Data::Session ses = Database::instance().createSession();
        logger.information("Filling goals array from database");
        
        Poco::Data::Statement select(ses);
        
        std::string owner_id;
        std::string title;
        
        logger.information("was here 11212");
        select << "SELECT owner_id, title FROM goals ORDER BY id";
        
        goals.clear(); // очищаем существующий массив

        select.execute();
        Poco::Data::RecordSet rs(select);
        if (!rs.moveFirst()) {
            return false; // empty result set
        }
        do {

            handlers::Goal goal;
            goal.owner_id = owner_id;
            goal.goal_name = title;
            goal.owner_id =  rs["owner_id"].convert<std::string>();
            goal.goal_name = rs["title"].convert<std::string>();
            goals.push_back(goal);
        } while (rs.moveNext());
        
        logger.information("Filled " + std::to_string(goals.size()) + " goals");
        return true;
    }
    catch (Poco::Exception& e) {
        logger.error("Error filling goals array: " + e.displayText());
        return false;
    }
}

};

} // namespace handlers
