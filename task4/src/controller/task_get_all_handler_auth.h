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
#include "../util/user_parcer.h"
#include "../service/users_service.h"
#include "../service/task_service.h"
#include "../database/Tasks.h"

namespace handlers
{

    class TasksAllHandler : public Poco::Net::HTTPRequestHandler
    {
    public:
        void handleRequest(Poco::Net::HTTPServerRequest &request,
                           Poco::Net::HTTPServerResponse &response) override
        {

            auto &logger = Poco::Logger::get("Tasks_get_all_handler");

            logger.information("tasks_get_all_handler: 200 GET /api/v1/tasks");

            Poco::JSON::Object::Ptr responseObj = new Poco::JSON::Object();
            responseObj->set("status", "success");

            Poco::JSON::Array::Ptr goalsArray = new Poco::JSON::Array();

            std::vector<Task> bd_tasks;
            mai::service::TaskService service = mai::service::TaskService();
            logger.information("before alllll");
            service.fillTasksArray(bd_tasks);
            // logger.information("res = " + res);
            try
            {

                logger.information("field");
                for (const auto &goal : bd_tasks)
                {
                    logger.information("try parce");
                    Poco::JSON::Object::Ptr goalObj = new Poco::JSON::Object();
                    // goalObj->set("id", goal.id);
                    goalObj->set("assignee_id", goal.assignee_id);
                    goalObj->set("goal_id", goal.goal_id);
                    goalObj->set("status", goal.status);
                    goalObj->set("text", goal.text);
                    goalObj->set("_id", goal._id);
                    goalsArray->add(goalObj);
                }
                responseObj->set("tasks", goalsArray);
            }
            catch (Poco::Exception &e)
            {
                logger.error("Error filling tasks array: " + e.displayText());
            }
            catch (std::exception &e)
            {
                logger.error("Standard exception: " + std::string(e.what()));
            }
            catch (...)
            {
                logger.error("Unknown exception occurred");
            }

            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
            response.setContentType("application/json");
            std::ostream &ostr = response.send();
            responseObj->stringify(ostr);
        }
    };

} // namespace handlers
