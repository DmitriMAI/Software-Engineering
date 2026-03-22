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
#include "../errors/AuthError.h"
#include "../util/autorization_by_jwt.h"
#include "../util/goal_parcer.h"
#include "../service/goals_service.h"

namespace handlers {

class GoalsAddHandler : public Poco::Net::HTTPRequestHandler {
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

        auto& logger = Poco::Logger::get("Goals_add_handler");
        logger.information("goals_add_handler: 200 POST /api/v1/goals for add");

        mai::service::GoalsService service = mai::service::GoalsService();

        handlers::Goal goal = handlers::Goal();
        try {
            goal = mai::util::GoalParser::parseGoal(bodyStr);
        } catch(...) {
            response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
            std::ostream& ostr = response.send();
            ostr << R"({"message":"Cant parse request"})";
            return;
        }
        
        service.addToDB(
            goal
        );

        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentType("application/json");
        std::ostream& ostr = response.send();
        ostr << R"({"message":"Success on saving goal"})";

    }
};

} // namespace handlers
