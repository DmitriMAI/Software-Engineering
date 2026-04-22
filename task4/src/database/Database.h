#pragma once


#include <Poco/Data/PostgreSQL/Connector.h>
#include <Poco/Data/Session.h>
#include <Poco/Environment.h>

#include <Poco/Logger.h>

#include <mutex>
#include <string>


namespace mai::postGre
{

class Database {
public:
    static Database& instance() {
        static Database instance;
        return instance;
    }

    void initialize() {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_initialized) return;
        Poco::Data::PostgreSQL::Connector::registerConnector();
        _connectionString = buildConnectionString();
        _initialized = true;
    }

    Poco::Data::Session createSession() {
    auto& logger = Poco::Logger::get("users_service");
    try {
        logger.information("was here 2");
        initialize();
        logger.information("was here 3");
        return Poco::Data::Session(Poco::Data::PostgreSQL::Connector::KEY, _connectionString);
    } catch (Poco::Exception& e) {
        logger.error("Poco Exception: " + e.displayText());
        logger.error("Exception code: " + std::to_string(e.code()));
        logger.error("Exception message: " + e.message());
    } catch (const std::exception& e) {
        logger.error("Standard exception: " + std::string(e.what()));
        throw;
    } catch (...) {
        logger.error("Unknown exception occurred");
        throw;
    }
}

    void initializeSchema() {
        Poco::Data::Session session = createSession();
        session << "DROP TABLE IF EXISTS tasks", Poco::Data::Keywords::now;
        session << "DROP TABLE IF EXISTS goals", Poco::Data::Keywords::now;
        session << "DROP TABLE IF EXISTS users", Poco::Data::Keywords::now;

        session << "CREATE TABLE IF NOT EXISTS users ("
                       "id BIGSERIAL PRIMARY KEY,"
                       "first_name TEXT NOT NULL,"
                       "last_name TEXT NOT NULL,"
                       "login TEXT NOT NULL UNIQUE"
                    ")", Poco::Data::Keywords::now;
        session << "CREATE TABLE IF NOT EXISTS goals ("
                    "id SERIAL PRIMARY KEY,"
                    "owner_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,"
                    "title VARCHAR(256) NOT NULL"
                    ")", Poco::Data::Keywords::now;
        session << "CREATE TABLE IF NOT EXISTS tasks ("
            "id SERIAL PRIMARY KEY,"
            "goal_id INTEGER NOT NULL REFERENCES goals(id) ON DELETE CASCADE,"
            "assignee_id INTEGER REFERENCES users(id) ON DELETE SET NULL,"
            "title VARCHAR(256) NOT NULL,"
            "description TEXT,"
            "status VARCHAR(32) NOT NULL DEFAULT 'TODO' "
                "CHECK (status IN ('TODO', 'IN_PROGRESS', 'DONE', 'CANCELLED')),"
            "due_date DATE,"
            "created_at TIMESTAMP NOT NULL DEFAULT NOW(),"
            "updated_at TIMESTAMP NOT NULL DEFAULT NOW(),"
            "CONSTRAINT chk_task_title_nonempty CHECK (title <> '')"
            ")", Poco::Data::Keywords::now;
        session << "INSERT INTO users (first_name, last_name, login) VALUES "
                    "('Admin', 'Adminov', 'admin'),"
                    "('Default', 'User', 'default'),"
                    "('Алексей', 'Сидоров', 'alexey.sidorov'),"
                    "('Елена', 'Кузнецова', 'elena.kuznetsova'),"
                    "('Дмитрий', 'Смирнов', 'dmitry.smirnov'),"
                    "('Анна', 'Васильева', 'anna.vasilieva'),"
                    "('Сергей', 'Михайлов', 'sergey.mikhailov'),"
                    "('Ольга', 'Новикова', 'olga.novikova'),"
                    "('Павел', 'Федоров', 'pavel.fedorov'),"
                    "('Татьяна', 'Морозова', 'tatiana.morozova')", Poco::Data::Keywords::now;
        session << "INSERT INTO goals (owner_id, title) VALUES "
                    "(1, 'make money'),"
                    "(2, 'pass tests'),"
                    "(3, 'Научиться играть на гитаре'),"
                    "(4, 'Заработать первый миллион'),"
                    "(5, 'Написать книгу'),"
                    "(6, 'Открыть свой бизнес'),"
                    "(7, 'Путешествовать по Европе'),"
                    "(8, 'Сменить профессию'),"
                    "(9, 'Купить квартиру'),"
                    "(10, 'Стать экспертом в своей области')", Poco::Data::Keywords::now;
    }

private:
    Database() = default;

    std::string buildConnectionString() const {
        const std::string host = Poco::Environment::get("PGHOST", "localhost");
        const std::string port = Poco::Environment::get("PGPORT", "5432");
        const std::string dbname = Poco::Environment::get("PGDATABASE", "poco_template");
        const std::string user = Poco::Environment::get("PGUSER", "postgres");
        const std::string password = Poco::Environment::get("PGPASSWORD", "postgres");
        return "host=" + host + " port=" + port + " dbname=" + dbname + " user=" + user + " password=" + password;
    }

    std::mutex _mutex;
    bool _initialized{false};
    std::string _connectionString;
};
}