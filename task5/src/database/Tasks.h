#pragma once

#include <Poco/Exception.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/MongoDB/Array.h>
#include <Poco/MongoDB/Document.h>
#include <Poco/MongoDB/Element.h>
#include <Poco/MongoDB/OpMsgCursor.h>
#include <Poco/MongoDB/OpMsgMessage.h>
#include <Poco/MongoDB/RegularExpression.h>
#include <Poco/Nullable.h>

#include "./MongoDb.h"

#include <optional>
#include <string>
#include <vector>



struct Task {
    std::string _id{"0"};
    std::string id{"0"};
    std::string goal_id;
    std::string assignee_id;
    std::string status;
    std::string text;
};

namespace {

using namespace Poco::MongoDB;

inline Task TaskFromDoc(const Document::Ptr& doc) {
    Task task;
    Poco::MongoDB::ObjectId::Ptr oid = doc->get<Poco::MongoDB::ObjectId::Ptr>("_id");
    task._id = oid->toString();

    task.assignee_id = doc->get<std::string>("assignee_id");
    task.goal_id = doc->get<std::string>("goal_id");
    task.status = doc->get<std::string>("status");
    task.text = doc->get<std::string>("text");
    return task;
}

} // namespace
