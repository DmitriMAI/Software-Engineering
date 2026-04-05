#pragma once

#include <string>
#include <vector>

namespace handlers {

struct Goal {
    std::string user_login;
    std::string goal_name;
    
    Goal(){};
    Goal(const std::string& user_login, const std::string& goal_name) 
        : user_login(user_login), goal_name(goal_name){};
    
};

extern std::vector<Goal> g_goals;

void initDefaultGoals() {
    g_goals.clear();
    g_goals.emplace_back("admin", "make money");
    g_goals.emplace_back("default", "pass tests");
}

} // namespace handlers
