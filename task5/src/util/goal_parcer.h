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

namespace mai::util {

class GoalParser  {
public:
    static handlers::Goal parseGoal(std::string& str){
        Poco::JSON::Parser parser;
        auto result = parser.parse(str);
        auto obj = result.extract<Poco::JSON::Object::Ptr>();
        
        std::string login  = obj->getValue<std::string>("login");
        std::string name  = obj->getValue<std::string>("name");
        
        return handlers::Goal(login, name);
    }
};

} // namespace handlers
