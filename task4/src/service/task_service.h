#pragma once

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Logger.h>
#include <Poco/Timestamp.h>
#include <Poco/URI.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>
#include <Poco/MongoDB/Array.h>
#include <Poco/MongoDB/Document.h>
#include <Poco/MongoDB/Element.h>
#include <Poco/MongoDB/OpMsgCursor.h>
#include <Poco/MongoDB/OpMsgMessage.h>
#include <Poco/MongoDB/RegularExpression.h>
#include "Poco/MongoDB/ObjectId.h"

#include "../database/Tasks.h"

namespace mai::service
{

    class TaskService
    {
    public:
        bool updateStatus(const std::string &taskId, const std::string &newStatus)
        {
            auto &logger = Poco::Logger::get("tasks_service");
            logger.information("Updating status for task: " + taskId + " new status " + newStatus + " < ");

            try
            {
                mongo::Database &d = mongo::Database::instance();
                Poco::MongoDB::Database &mdb = d.mongoDb();
                const std::string dbName = mdb.name();

                Poco::SharedPtr<OpMsgMessage> req = mdb.createOpMsgMessage("tasks");
                req->setCommandName(OpMsgMessage::CMD_UPDATE);

                // Фильтр: найти задачу по _id
                Document::Ptr filterDoc = new Document();
                Poco::MongoDB::ObjectId::Ptr oid = new Poco::MongoDB::ObjectId(taskId);
                filterDoc->add("_id", oid);

                // Обновление: установить новый статус
                Document::Ptr setDoc = new Document();
                setDoc->add("status", newStatus);

                Document::Ptr updateDoc = new Document();
                updateDoc->add("$set", setDoc);

                // Собираем update-документ
                Document::Ptr updateEntry = new Document();
                updateEntry->add("q", filterDoc);
                updateEntry->add("u", updateDoc);
                updateEntry->add("multi", false);
                updateEntry->add("upsert", false);

                req->documents().push_back(updateEntry);

                OpMsgMessage resp;
                d.send(*req, resp);

                if (!resp.responseOk())
                {
                    throw Poco::RuntimeException("task status update failed: " + resp.body().toString());
                }

                Poco::Int32 nModified = 0;
                try
                {
                    nModified = resp.body().getInteger("nModified");
                }
                catch (const Poco::Exception &)
                {
                }

                if (nModified == 0)
                {
                    logger.warning("No task found with id: " + taskId);
                    return false;
                }

                logger.information("Status updated successfully for task: " + taskId);
            }
            catch (Poco::Exception &e)
            {
                logger.error("Error updating task status: " + e.displayText());
                return false;
            }

            return true;
        }

        bool addToRealDB(Task &task)
        {
            auto &logger = Poco::Logger::get("users_service");
            logger.information("before all save user2");

            try
            {
                Poco::JSON::Array::Ptr result = new Poco::JSON::Array();
                mongo::Database &d = mongo::Database::instance();
                Poco::MongoDB::Database &mdb = d.mongoDb();
                const std::string dbName = mdb.name();

                Poco::SharedPtr<OpMsgMessage> req = mdb.createOpMsgMessage("tasks");
                req->setCommandName(OpMsgMessage::CMD_INSERT);

                Document::Ptr doc = new Document();
                doc->add("assignee_id", task.assignee_id);
                doc->add("goal_id", task.goal_id);
                doc->add("status", task.status);
                doc->add("text", task.text);

                req->documents().push_back(doc);

                OpMsgMessage resp;
                d.send(*req, resp);
                if (!resp.responseOk()){
                    Poco::Int64 code = 0;
                    try{
                        code = resp.body().getInteger("code");
                    }
                    catch (const Poco::Exception &){}
                    if (code == 11000)
                    {
                        throw Poco::InvalidArgumentException("duplicate key (email or login must be unique)");
                    }
                    if (code == 121) {
                        std::string details = "";
                        try {
                            Poco::MongoDB::Document::Ptr errInfo = 
                                resp.body().get<Poco::MongoDB::Document::Ptr>("errInfo");
                            details = errInfo->toString();
                        } catch (const Poco::Exception &) {}
                        throw Poco::InvalidArgumentException("schema validation failed: " + details);
                    }
                    throw Poco::RuntimeException("user insert failed: " + resp.body().toString());
                    
                } else {
                    bool hasWriteErrors = resp.body().exists("writeErrors");
                    if (hasWriteErrors) {
                        bool hasWriteError = false;
                        try {
                            Poco::MongoDB::Array::Ptr writeErrors =
                                resp.body().get<Poco::MongoDB::Array::Ptr>("writeErrors");

                            if (writeErrors && writeErrors->size() > 0) {
                                hasWriteError = true;
                                Poco::MongoDB::Document::Ptr firstError =
                                    writeErrors->get<Poco::MongoDB::Document::Ptr>(0);

                                Poco::Int32 errCode = firstError->getInteger("code");
                                std::string errmsg  = firstError->get<std::string>("errmsg");

                                logger.error("writeError code=" + std::to_string(errCode) + " msg=" + errmsg);

                                if (errCode == 121)
                                    throw Poco::InvalidArgumentException("schema validation failed: " + errmsg);
                                if (errCode == 11000)
                                    throw Poco::InvalidArgumentException("duplicate key: " + errmsg);

                                throw Poco::RuntimeException("write error: " + errmsg);
                            }
                        } catch (const Poco::InvalidArgumentException &) { throw; }
                        catch (const Poco::RuntimeException &)         { throw; }
                        catch (const Poco::Exception &)                { /* writeErrors отсутствует — ок */ }

                        if (!hasWriteError) {
                            Poco::Int32 n = 0;
                            try { n = resp.body().getInteger("n"); } catch (const Poco::Exception &) {}
                            if (n == 0) {
                                logger.warning("n=0: document silently rejected");
                                return false;
                            }
                            logger.information("inserted successfully, n=" + std::to_string(n));
                        }
                    } 
                }
            }
            catch (Poco::Exception &e)
            {
                logger.error("Error on saving: " + e.displayText());
                return false;
            }
            return true;
        }

        bool fillTasksArray(std::vector<Task> &tasks)
        {
            auto &logger = Poco::Logger::get("tasks_service");
            try
            {
                Poco::JSON::Array::Ptr result = new Poco::JSON::Array();
                mongo::Database &d = mongo::Database::instance();
                Poco::MongoDB::Database &mdb = d.mongoDb();
                const std::string dbName = mdb.name();

                d.withLockedConnection([&](Poco::MongoDB::Connection &conn){
                    logger.information("was here name");
                    Poco::MongoDB::OpMsgCursor cursor(dbName, "tasks");
                    cursor.setBatchSize(1000);
                    cursor.query().setCommandName(OpMsgMessage::CMD_FIND);
                    Poco::MongoDB::Document& body = cursor.query().body();
                    body.addNewDocument("sort").add("id", 1);
                    body.addNewDocument("filter");

                    Poco::MongoDB::OpMsgMessage& cresp = cursor.next(conn);
                    while (true) {
                        mongo::mongoEnsureOk(cresp, "user list");
                        for (const auto& doc : cresp.documents()) {
                            Task t = TaskFromDoc(doc);                   
                            tasks.push_back(t);
                        }
                        if (cursor.cursorID() == 0) break;
                        cresp = cursor.next(conn);
                    } 
                });
                for (const auto& goal : tasks) {
                    logger.information("task assignee_id: " + goal.assignee_id);
                    logger.information("task goal_id: " + goal.goal_id);
                    logger.information("task status: " + goal.status);
                    logger.information("task text: " + goal.text);
                }
            } catch (Poco::Exception &e){
                logger.error("Error filling tasks array: " + e.displayText());
                return false;
            }
            return true;
        };
    };

};