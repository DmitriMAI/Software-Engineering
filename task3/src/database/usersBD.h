#pragma once

#include <string>
#include <vector>

namespace handlers {

struct User {
    std::string login;
    std::string firstName;
    std::string lastName;
    
    User(){};
    User(const std::string& login, const std::string& firstName, const std::string& lastName) 
        : login(login), firstName(firstName), lastName(lastName){}
    
    std::string getFullName() const {
        return firstName + " " + lastName;
    }
};

extern std::vector<User> g_users;

void initDefaultUsers(std::vector<User>& users) {
    users.clear();
    users.emplace_back("admin", "Admin", "Adminov");
    users.emplace_back("default", "Default", "User");
}

} // namespace handlers
