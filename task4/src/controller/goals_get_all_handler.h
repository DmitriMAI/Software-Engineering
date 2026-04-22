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
#include "../service/goals_service.h"

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
        
        std::vector<handlers::Goal> bd_goals;
        mai::service::GoalsService service = mai::service::GoalsService();
        service.fillGoalsArray(bd_goals);

        for (const auto& goal : bd_goals) {
            Poco::JSON::Object::Ptr goalObj = new Poco::JSON::Object();
            goalObj->set("id", goal.owner_id);
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
