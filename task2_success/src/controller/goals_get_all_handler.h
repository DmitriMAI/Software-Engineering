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

namespace handlers {

class GoalsGetAllHandler : public Poco::Net::HTTPRequestHandler {
public:
    void handleRequest(Poco::Net::HTTPServerRequest& request,
                       Poco::Net::HTTPServerResponse& response) override {


        auto& logger = Poco::Logger::get("Goals_get_all_handler");

        logger.information("goals_get_all_handler: 200 GET /api/v1/goals");

        Poco::JSON::Object::Ptr responseObj = new Poco::JSON::Object();
        responseObj->set("status", "success");
        
        Poco::JSON::Array::Ptr goalsArray = new Poco::JSON::Array();
        
        for (const auto& goal : g_goals) {
            Poco::JSON::Object::Ptr goalObj = new Poco::JSON::Object();
            goalObj->set("login", goal.user_login);
            goalObj->set("name", goal.goal_name);
            goalsArray->add(goalObj);
        }
        responseObj->set("goals", goalsArray);

        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentType("application/json");
        std::ostream& ostr = response.send();
        responseObj->stringify(ostr);
    }
};

} // namespace handlers
