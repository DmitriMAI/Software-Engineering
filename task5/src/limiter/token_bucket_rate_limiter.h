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