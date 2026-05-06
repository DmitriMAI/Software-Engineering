#pragma once

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Logger.h>
#include <Poco/Timestamp.h>
#include <Poco/URI.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>

#include "../service/users_service.h"
#include "../limiter/token_bucket_rate_limiter.h"

namespace handlers {

class FindUsersFullNameHandler : public Poco::Net::HTTPRequestHandler {
private: 
    std::string user_name;
    std::string user_last_name;

    static mai::TokenBucketRateLimiter& rateLimiter() {
        // 1 запросов макс, 1 токенов/сек для тестирования
        static mai::TokenBucketRateLimiter limiter(1.0, 1.0);
        return limiter;
    }

public:
    explicit FindUsersFullNameHandler(const std::string& userName, const std::string& userLastName) : user_name(userName), user_last_name(userLastName) {}

    void handleRequest(Poco::Net::HTTPServerRequest& request,
                       Poco::Net::HTTPServerResponse& response) override {

        auto& logger = Poco::Logger::get("Find_user_fullname");
        logger.information("find_user_fullname: 200 POST /api/v1/users/fullname name is %s", user_name);

        bool canHandleRequest = handleRateLimit(request, response);
        if (!canHandleRequest) {
            return;
        }

        mai::service::UsersService service = mai::service::UsersService();

        bool is_user_exists = service.existsByFullNameCached(user_name, user_last_name);

        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentType("application/json");
        std::ostream& ostr = response.send();
        if (is_user_exists) {
            ostr << R"({"user":"exist"})";
        } else {
            ostr << R"({"user":"not exist"})";
        }

    }

    bool handleRateLimit(
        Poco::Net::HTTPServerRequest& request,
        Poco::Net::HTTPServerResponse& response
    ) {
        auto& limiter = rateLimiter();
        response.set("X-RateLimit-Limit",     std::to_string((int)limiter.capacity()));
        response.set("X-RateLimit-Remaining", std::to_string((int)std::floor(limiter.tokens())));
        response.set("X-RateLimit-Reset",     std::to_string((int)std::ceil(limiter.secondsUntilNextToken())));

        auto& logger = Poco::Logger::get("Find_user_fullname");

        if (!limiter.tryConsume()) {
            logger.warning("Rate limit exceeded for /api/v1/users/fullname");

            response.set("Retry-After", std::to_string((int)std::ceil(limiter.secondsUntilNextToken())));
            response.setStatus(Poco::Net::HTTPResponse::HTTP_TOO_MANY_REQUESTS);
            response.setContentType("application/json");
            std::ostream& ostr = response.send();
            ostr << R"({"error":"too many requests","message":"Rate limit exceeded. Try again later."})";
            return false;
        }

        return true;
    }
};


} // namespace handlers
