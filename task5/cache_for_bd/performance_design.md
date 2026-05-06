Кэширование взял из репозитория Преподавателя и адаптировал.

```c++
#pragma once

#include <Poco/Environment.h>
#include <Poco/Types.h>
#include <Poco/Exception.h>
#include <Poco/Logger.h>
#include <Poco/NumberFormatter.h>
#include <Poco/NumberParser.h>
#include <Poco/Redis/Client.h>
#include <Poco/Redis/Command.h>
#include <Poco/Redis/Exception.h>
#include <Poco/Redis/Type.h>
#include <Poco/Timespan.h>

#include <atomic>
#include <mutex>
#include <optional>
#include <string>

namespace mai {

/// Кэш JSON-представления пользователя по `user_id` в Redis (`Poco::Redis`).
/// Один `Client` + `std::mutex` — корректно при параллельных HTTP-воркерах POCO.
class UserRedisCache {
public:
    static constexpr int kTtlSeconds = 60;

    static UserRedisCache& instance() {
        static UserRedisCache inst;
        return inst;
    }

    static bool userCacheEnabled() {
        return _userCacheEnabled.load(std::memory_order_relaxed);
    }

    static void setUserCacheEnabled(bool enabled) {
        _userCacheEnabled.store(enabled, std::memory_order_relaxed);
    }

    /// Read-through: значение из Redis или nullopt при промахе / ошибке Redis.
    std::optional<std::string> tryGetJson(Poco::Int64 userId) {
        if (!userCacheEnabled()) return std::nullopt;
        if (!ensureConnected()) return std::nullopt;

        std::lock_guard<std::mutex> lock(_mutex);
        try {
            Poco::Redis::BulkString raw =
                _client.execute<Poco::Redis::BulkString>(Poco::Redis::Command::get(cacheKey(userId)));
            if (raw.isNull()) return std::nullopt;
            return raw.value();
        } catch (const Poco::Redis::RedisException& e) {
            logRedisWarning("tryGetJson", e);
            return std::nullopt;
        }
    }

    /// Read-through: значение из Redis или nullopt при промахе / ошибке Redis.
    std::optional<std::string> tryGetJson(std::string login) {
        if (!userCacheEnabled()) return std::nullopt;
        if (!ensureConnected()) return std::nullopt;

        std::lock_guard<std::mutex> lock(_mutex);
        try {
            Poco::Redis::BulkString raw =
                _client.execute<Poco::Redis::BulkString>(Poco::Redis::Command::get(
                        cacheKeyLogin(login))
                    );
            if (raw.isNull()) return std::nullopt;
            return raw.value();
        } catch (const Poco::Redis::RedisException& e) {
            logRedisWarning("tryGetJson", e);
            return std::nullopt;
        }
    }

    std::optional<std::string> tryGetJsonFullName(std::string fullName) {
        if (!userCacheEnabled()) return std::nullopt;
        if (!ensureConnected()) return std::nullopt;

        std::lock_guard<std::mutex> lock(_mutex);
        try {
            Poco::Redis::BulkString raw =
                _client.execute<Poco::Redis::BulkString>(Poco::Redis::Command::get(
                        cacheKeyFullName(fullName))
                    );
            if (raw.isNull()) return std::nullopt;
            return raw.value();
        } catch (const Poco::Redis::RedisException& e) {
            logRedisWarning("tryGetJson", e);
            return std::nullopt;
        }
    }

    /// Write-through: сохранить актуальный JSON в Redis с TTL.
    void putJson(Poco::Int64 userId, const std::string& json) {
        if (!userCacheEnabled()) return;
        if (!ensureConnected()) return;

        const Poco::Timespan ttl(kTtlSeconds, 0);

        std::lock_guard<std::mutex> lock(_mutex);
        try {
            _client.execute<std::string>(
                Poco::Redis::Command::set(cacheKey(userId), json, true, ttl, true));
        } catch (const Poco::Redis::RedisException& e) {
            logRedisWarning("putJson", e);
        }
    }

    /// Write-through: сохранить актуальный JSON в Redis с TTL.
    void putJsonLogin(std::string login, const std::string& json) {
        if (!userCacheEnabled()) return;
        if (!ensureConnected()) return;

        const Poco::Timespan ttl(kTtlSeconds, 0);

        std::lock_guard<std::mutex> lock(_mutex);
        try {
            _client.execute<std::string>(
                Poco::Redis::Command::set(cacheKeyLogin(login), json, true, ttl, true));
        } catch (const Poco::Redis::RedisException& e) {
            logRedisWarning("putJson", e);
        }
    }

    /// Write-through: сохранить актуальный JSON в Redis с TTL.
    void putJsonFullName(std::string fullName, const std::string& json) {
        if (!userCacheEnabled()) return;
        if (!ensureConnected()) return;

        const Poco::Timespan ttl(kTtlSeconds, 0);

        std::lock_guard<std::mutex> lock(_mutex);
        try {
            _client.execute<std::string>(
                Poco::Redis::Command::set(cacheKeyFullName(fullName), json, true, ttl, true));
        } catch (const Poco::Redis::RedisException& e) {
            logRedisWarning("putJson", e);
        }
    }

    void remove(Poco::Int64 userId) {
        if (!userCacheEnabled()) return;
        if (!ensureConnected()) return;

        std::lock_guard<std::mutex> lock(_mutex);
        try {
            _client.execute<Poco::Int64>(Poco::Redis::Command::del(cacheKey(userId)));
        } catch (const Poco::Redis::RedisException& e) {
            logRedisWarning("remove", e);
        }
    }

private:
    UserRedisCache() = default;

    static std::string cacheKey(Poco::Int64 userId) {
        return std::string("user:") + Poco::NumberFormatter::format(userId);
    }

    static std::string cacheKeyLogin(std::string login) {
        return std::string("login:") + login;
    }

    static std::string cacheKeyFullName(std::string fullName) {
        return std::string("fullName:") + fullName;
    }

    bool ensureConnected() {
        if (_disabled) return false;
        if (_connected && _client.isConnected()) return true;

        std::lock_guard<std::mutex> lock(_mutex);
        if (_disabled) return false;
        if (_connected && _client.isConnected()) return true;

        const std::string host = Poco::Environment::get("REDIS_HOST", "localhost");
        const std::string portStr = Poco::Environment::get("REDIS_PORT", "6379");
        int port = 6379;
        try {
            port = Poco::NumberParser::parse(portStr);
        } catch (const Poco::Exception&) {
            port = 6379;
        }

        try {
            if (_client.isConnected()) _client.disconnect();
            _client.connect(host, port);
            _client.execute<std::string>(Poco::Redis::Command::ping());
            _connected = true;
            _disabled = false;
            return true;
        } catch (const Poco::Exception& e) {
            auto& logger = Poco::Logger::get("Server");
            logger.warning("Redis unavailable at %s:%d (%s). User cache disabled for this process.",
                           host,
                           port,
                           e.displayText());
            _connected = false;
            _disabled = true;
            return false;
        }
    }

    static void logRedisWarning(const char* op, const Poco::Exception& e) {
        Poco::Logger::get("Server").warning("Redis %s: %s", std::string(op), e.displayText());
    }

    std::mutex _mutex;
    Poco::Redis::Client _client;
    bool _connected{false};
    bool _disabled{false};

    inline static std::atomic<bool> _userCacheEnabled{true};
};

} // namespace cache
```

Limiter
```c++
#pragma once

#include <Poco/Mutex.h>
#include <chrono>

namespace mai {

class TokenBucketRateLimiter {
public:
    TokenBucketRateLimiter(double capacity, double refillRate)
        : _capacity(capacity), _refillRate(refillRate), _tokens(capacity),
          _lastRefill(std::chrono::steady_clock::now()) {}

    bool tryConsume() {
        Poco::FastMutex::ScopedLock lock(_mutex);
        refill();

        if (_tokens >= 1.0) {
            _tokens -= 1.0;
            return true;
        }
        return false;
    }

    double tokens() {
        Poco::FastMutex::ScopedLock lock(_mutex);
        refill();
        return _tokens;
    }

    double capacity() const { return _capacity; }

    double secondsUntilNextToken() {
        Poco::FastMutex::ScopedLock lock(_mutex);
        if (_tokens >= 1.0) return 0.0;
        return (1.0 - _tokens) / _refillRate;
    }

private:
    void refill() {
        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(now - _lastRefill).count();
        _tokens = std::min(_capacity, _tokens + elapsed * _refillRate);
        _lastRefill = now;
    }

    double _capacity;
    double _refillRate;
    double _tokens;
    std::chrono::steady_clock::time_point _lastRefill;
    Poco::FastMutex _mutex;
};

} // namespace mai
```
  
   
**Кэширование write и read throught. При записи положить в кэш и при обращении к бд зайти в кэш сначала.**  
  
**rateLimiter** - ведро с токенами. Они заполняются - 1 токен в секунду, и макс лимит в один токен для тестирования.  