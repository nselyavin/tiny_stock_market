#ifndef USERHANDLER_H
#define USERHANDLER_H
#include <map>
#include <string>
#include <optional>
#include <map>

class User {
public:
    std::string user_id;
    double balance;    
};


class UserHandler {
public:
    using UserMap = std::map<std::string, User>;

    static UserHandler* get_instance();
    void add_user(User& user);
    void update_user(const User& user);
    std::optional<User> get_user(const std::string& user_id);
    std::map<std::string, User> get_users();

    bool verify_user(const std::string& user_id);

private:
    UserMap users;
};

#endif // USERHANDLER_H
