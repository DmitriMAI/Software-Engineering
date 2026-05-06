#include <iostream>
#include <string>

#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Environment.h>
#include <Poco/Logger.h>
#include <Poco/NumberParser.h>
#include <Poco/Prometheus/Counter.h>
#include <Poco/Prometheus/Histogram.h>
#include <Poco/Prometheus/Registry.h>
#include <Poco/Util/ServerApplication.h>

#include "config/auth_config.h"
#include "handler/router_factory.h"
#include "database/usersBD.h"
#include "database/goalBD.h"
#include "database/Database.h"

using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;

namespace {

void configureLogging() {
    std::string level = Environment::get("LOG_LEVEL", "information");

    Poco::Message::Priority prio = Poco::Message::PRIO_INFORMATION;
    if (level == "trace") prio = Poco::Message::PRIO_TRACE;
    else if (level == "debug") prio = Poco::Message::PRIO_DEBUG;
    else if (level == "information" || level == "info") prio = Poco::Message::PRIO_INFORMATION;
    else if (level == "notice") prio = Poco::Message::PRIO_NOTICE;
    else if (level == "warning" || level == "warn") prio = Poco::Message::PRIO_WARNING;
    else if (level == "error") prio = Poco::Message::PRIO_ERROR;
    else if (level == "critical") prio = Poco::Message::PRIO_CRITICAL;
    else if (level == "fatal") prio = Poco::Message::PRIO_FATAL;
    else if (level == "none") prio = static_cast<Poco::Message::Priority>(Poco::Message::PRIO_FATAL + 1);

    Logger::root().setLevel(prio);
}

} // namespace

namespace handlers {

// Poco::Prometheus::Counter* g_httpRequests = nullptr;
// Poco::Prometheus::Counter* g_httpErrors = nullptr;
// Poco::Prometheus::Histogram* g_httpDuration = nullptr;
std::string g_jwtSecret;
std::vector<User> g_users;
std::vector<Goal> g_goals;
void initDefaultUsers(std::vector<User>& users);

} // namespace handlers

class ServerApp : public ServerApplication {
protected:
    int main(const std::vector<std::string>&) override {
        configureLogging();
        auto& logger = Logger::get("Server");

        unsigned short port = 8080;
        if (Environment::has("PORT")) {
            try {
                port = static_cast<unsigned short>(NumberParser::parse(Environment::get("PORT")));
            } catch (const Exception& e) {
                logger.warning("Invalid PORT, using default 8080: %s", e.displayText());
            }
        }

        logger.information("Starting server on port %hu", port);

        handlers::g_jwtSecret = Environment::get("JWT_SECRET", "");

        handlers::initDefaultUsers(handlers::g_users);
        handlers::initDefaultGoals();

        mai::postGre::Database::instance().initializeSchema();

        ServerSocket svs(port);
        HTTPServer srv(new handlers::RouterFactory(), svs, new HTTPServerParams);
        srv.start();

        logger.information("Server started. Endpoints: GET /api/v1/helloworld, GET /api/v1/helloworld_jwt, POST /api/v1/auth, GET /metrics, GET /swagger.yaml");

        waitForTerminationRequest();
        srv.stop();

        return Application::EXIT_OK;
    }
};

POCO_SERVER_MAIN(ServerApp)
