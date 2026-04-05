#pragma once

#include <userver/server/handlers/http_handler_base.hpp>

namespace tasksMai {
    class TasksControllerHandler final : public userver::server::handlers::HttpHandlerBase {
    public:
        static constexpr std::string_view kName = "task-hello";

        using HttpHandlerBase::HttpHandlerBase;

        std::string HandleRequestThrow(const userver::server::http::HttpRequest&, userver::server::request::RequestContext&)
            const override;

    };
}


