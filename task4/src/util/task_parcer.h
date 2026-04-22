#pragma once

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Logger.h>
#include <Poco/Timestamp.h>
#include <Poco/URI.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>

#include "../errors/AuthError.h"
#include "../util/autorization_by_jwt.h"
#include "../database/Tasks.h"

namespace mai::util {

class TaskParser  {
public:
    static Task parseTask(std::string& str){
        Poco::JSON::Parser parser;
        auto result = parser.parse(str);
        auto obj = result.extract<Poco::JSON::Object::Ptr>();
        
        std::string goal_id  = obj->getValue<std::string>("goal_id");
        std::string assignee_id  = obj->getValue<std::string>("assignee_id");
        std::string status  = obj->getValue<std::string>("status");
        std::string text  = obj->getValue<std::string>("text");
        
        return Task{"0", "0", goal_id, assignee_id, status, text};
    }

    static Task parseUpdateTask(std::string& str){
        Poco::JSON::Parser parser;
        auto result = parser.parse(str);
        auto obj = result.extract<Poco::JSON::Object::Ptr>();
        
        std::string id  = obj->getValue<std::string>("_id");
        std::string status  = obj->getValue<std::string>("status");
        
        return Task{id, "", "", "", status, ""};
    }
};

} // namespace handlers
