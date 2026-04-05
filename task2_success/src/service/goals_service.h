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

};

} // namespace handlers
